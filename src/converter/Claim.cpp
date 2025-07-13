#include "Claim.h"

namespace converter {

    Claim::Claim(
        const cnf::ResRule &rule, 
        const std::vector<std::pair<VeriPB::Lit, cnf::Clause>> &clauses, 
        const std::function<VeriPB::Lit(int32_t)> &variable_supplier,
        const std::function<bool(VeriPB::Lit)> &tautology_predicate,
        const std::function<VeriPB::constraintid(VeriPB::Lit)> &tautology_supplier,
        bool negated_pivot
    ) : negated_pivot(negated_pivot), tautology_predicate(tautology_predicate), tautology_supplier(tautology_supplier) {
        if (clauses.size() < 3) {
            throw std::runtime_error("Claim must have at least two clauses.");
        }

        int32_t pivot = rule.get_pivot();

        // Remove the pivot literal from the clauses
        std::unordered_multiset<int32_t> literals_1 = clauses[0].second.get_literals();
        std::unordered_multiset<int32_t> literals_2 = clauses[1].second.get_literals();
        if (literals_1.erase(pivot) == 0 && literals_1.erase(-pivot) == 0)
            throw std::runtime_error("Pivot literal not found in the first clause.");
        if (literals_2.erase(pivot) == 0 && literals_2.erase(-pivot) == 0)
            throw std::runtime_error("Pivot literal not found in the second clause.");
        std::vector<int32_t> literals_1_vec(literals_1.begin(), literals_1.end());
        std::vector<int32_t> literals_2_vec(literals_2.begin(), literals_2.end());

        // Sort the literals to ensure consistent ordering (a_1 ... a_n, b_1 ... b_m, pivot)
        initialize_vars(pivot, literals_1_vec, literals_2_vec, variable_supplier);
        vars.push_back(variable_supplier(pivot));


        for (const auto &clause : clauses) {
            blocking_vars.push_back(clause.first);
        }
        std::vector<Lit> new_blocking_vars = std::vector<Lit>(blocking_vars.begin() + 2, blocking_vars.end());

        uint32_t num_clauses_1 = literals_1.size();
        uint32_t num_clauses_2 = literals_2.size();

        this->active_blocking_vars = { new_blocking_vars[0] };

        if (negated_pivot) {
           this->pivot_literal = vars.back();
           this->active_original_blocking_var = blocking_vars[0];
           this->unactive_original_blocking_var = blocking_vars[1];
    
           this->active_blocking_vars.insert(this->active_blocking_vars.end(), new_blocking_vars.begin() + 1, new_blocking_vars.end() - num_clauses_1);
           this->unactive_blocking_vars = std::vector<Lit>(new_blocking_vars.begin() + num_clauses_2 + 1, new_blocking_vars.end());
           this->active_vars = std::vector<Lit>(vars.begin(), vars.begin() + num_clauses_1);
        } else {
           this->pivot_literal = neg(vars.back());
           this->active_original_blocking_var = blocking_vars[1];
           this->unactive_original_blocking_var = blocking_vars[0];
    
           this->active_blocking_vars.insert(this->active_blocking_vars.end(), new_blocking_vars.begin() + num_clauses_2 + 1, new_blocking_vars.end());
           this->unactive_blocking_vars = std::vector<Lit>(new_blocking_vars.begin() + 1, new_blocking_vars.end() - num_clauses_1);
           this->active_vars = std::vector<Lit>(vars.begin() + num_clauses_1, vars.end() - 1);
        }

        // TODO: This can be optimized
        // Find every variable that is in both clauses
        for (const auto &lit : literals_1) {
            if (literals_2.contains(lit)) {
                common_vars.insert(variable(variable_supplier(lit)));
            }
        }
    }


    bool Claim::is_negated_pivot() const {
        return negated_pivot;
    }

    const std::vector<Lit> &Claim::get_vars() const {
        return vars;
    }

    const std::vector<Lit> &Claim::get_literals() const {
        return literals;
    }

    const std::vector<Lit> &Claim::get_blocking_vars() const {
        return blocking_vars;
    }

    const Lit &Claim::get_pivot_literal() const {
        return pivot_literal;
    }

    const Lit &Claim::get_active_original_blocking_var() const {
        return active_original_blocking_var;
    }

    const Lit &Claim::get_unactive_original_blocking_var() const {
        return unactive_original_blocking_var;
    }

    const std::vector<Lit> &Claim::get_active_blocking_vars() const {
        return active_blocking_vars;
    }

    const std::vector<Lit> &Claim::get_unactive_blocking_vars() const {
        return unactive_blocking_vars;
    }

    const std::vector<constraintid> &Claim::get_active_constraints() const {
        return active_constraints;
    }

    void Claim::set_active_constraints(const std::vector<constraintid> &active_constraints) {
        this->active_constraints = active_constraints;
    }


    constraintid Claim::weaken(Prooflogger &pl, constraintid id, const std::vector<Lit> &variables, uint32_t begin, uint32_t end) {
        CuttingPlanesDerivation cpder(&pl, false);
        cpder.start_from_constraint(id);
        for (int i = begin; i < end; i++) {
            cpder.weaken(variable(variables[i]));
        }
        cpder.saturate();
        return cpder.end();
    }

    constraintid Claim::weaken_all_except(Prooflogger &pl, constraintid id, const std::vector<Lit> &variables, uint32_t except) {
        return weaken_all_except(pl, id, variables, except, except);
    }

    constraintid Claim::weaken_all_except(Prooflogger &pl, constraintid id, const std::vector<Lit> &variables, uint32_t begin, uint32_t end) {
        CuttingPlanesDerivation cpder(&pl, false);
        cpder.start_from_constraint(id);

        // TODO: This can be optimized
        std::unordered_set <Lit, LitHash, LitEqual> vars_set(variables.begin() + begin, variables.begin() + end + 1);
        for (int i = 0; i < variables.size(); i++) {
            if ((i < begin || i > end) && !(get_vars()[begin].v.v == variables[i].v.v)) {
                cpder.weaken(variable(variables[i]));
            }
        }
        cpder.saturate();
        return cpder.end();
    }

    constraintid Claim::add_all(Prooflogger &pl, const std::vector<constraintid> &constraints) {
        CuttingPlanesDerivation cpder(&pl, false);
        cpder.start_from_constraint(constraints[0]);
        for (size_t i = 1; i < constraints.size(); i++) {
            cpder.add_constraint(constraints[i]);
        }
        cpder.saturate();
        return cpder.end();
    }

    constraintid Claim::add_all_and_saturate(Prooflogger &pl, const std::vector<constraintid> &constraints) {
        CuttingPlanesDerivation cpder(&pl, false);
        cpder.start_from_constraint(constraints[0]);
        for (size_t i = constraints.size() - 1; i > 0; i--) {
            cpder.add_constraint(constraints[i]);
            cpder.saturate();
        }
        return cpder.end();
    }

    constraintid Claim::add_all_from_literal(Prooflogger &pl, const std::vector<constraintid> &constraints, const Lit lit) {
        CuttingPlanesDerivation cpder(&pl, false);
        cpder.start_from_literal_axiom(lit);
        for (size_t i = 0; i < constraints.size(); i++) {
            cpder.add_constraint(constraints[i]);
        }
        cpder.saturate();
        return cpder.end();
    }

    constraintid Claim::add_all_prev(Prooflogger &pl, uint32_t range) {
        CuttingPlanesDerivation cpder(&pl, false);
        cpder.start_from_constraint(-1);
        for (int32_t i = 1; i < range; i++) {
            cpder.add_constraint(-i - 1);
        }
        cpder.saturate();
        return cpder.end();
    }

    constraintid Claim::add_all_prev_from_literal(Prooflogger &pl, uint32_t range, const Lit lit) {
        CuttingPlanesDerivation cpder(&pl, false);
        cpder.start_from_literal_axiom(lit);
        for (int32_t i = 0; i < range; i++) {
            cpder.add_constraint(-i - 1);
        }
        cpder.saturate();
        return cpder.end();
    }

    constraintid Claim::build_proof_by_contradiction(Prooflogger &pl, Constraint<Lit, uint32_t, uint32_t> &C, constraintid claim_1, constraintid claim_2) {
        pl.start_proof_by_contradiction(C);

        CuttingPlanesDerivation cpder(&pl, false);
        cpder.start_from_constraint(claim_1);
        cpder.add_constraint(-1);
        cpder.saturate();
        cpder.end();

        cpder.start_from_constraint(claim_2);
        cpder.add_constraint(-2);
        cpder.saturate();
        cpder.end();

        // Add the previous constraints
        cpder.start_from_constraint(-1);
        cpder.add_constraint(-2);
        cpder.saturate();
        cpder.end();

        return pl.end_proof_by_contradiction();
    }

    constraintid Claim::build_proof_by_contradiction(Prooflogger &pl, Constraint<Lit, uint32_t, uint32_t> &C, std::vector<VeriPB::constraintid> &claims) {
        // if (claims.size() < 2) {
        //     throw std::runtime_error("At least two claims are required to build a proof by contradiction.");
        // }
        
        constraintid constraint = pl.start_proof_by_contradiction(C);

        CuttingPlanesDerivation cpder(&pl, false);
        
        for (auto& claim : claims) {
            cpder.start_from_constraint(constraint);
            cpder.add_constraint(claim);
            cpder.saturate();
            cpder.end();
        }

        add_all_prev(pl, claims.size());

        return pl.end_proof_by_contradiction();
    }

    void Claim::initialize_vars(int32_t pivot, const std::vector<int32_t>& literals_1, const std::vector<int32_t>& literals_2, std::function<VeriPB::Lit(int32_t)> variable_supplier) {
        vars.reserve(literals_1.size() + literals_2.size());
        literals.reserve(literals_1.size() + literals_2.size() - 1);

        for (const auto& lit : literals_1) {
            vars.push_back(variable_supplier(lit));

            if (lit != pivot) {
                VeriPB::Lit negated_lit = (lit < 0) ? neg(variable_supplier(lit)) : variable_supplier(lit);
                literals.push_back(negated_lit);
            }
        }
        for (const auto& lit : literals_2) {
            vars.push_back(variable_supplier(lit));

            if (lit != pivot) {
                VeriPB::Lit negated_lit = (lit < 0) ? neg(variable_supplier(lit)) : variable_supplier(lit);
                literals.push_back(negated_lit);
            }
        }
    }
}