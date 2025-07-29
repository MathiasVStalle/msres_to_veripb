#include "ResClaim.h"

namespace converter {

    ResClaim::ResClaim(
        const cnf::ResRule &rule, 
        const std::vector<std::pair<VeriPB::Lit, cnf::Clause>> &clauses, 
        const std::function<VeriPB::Lit(int32_t)> &variable_supplier,
        const std::function<bool(VeriPB::Lit)> &tautology_predicate,
        const std::function<bool(VeriPB::Lit)> &hard_clause_predicate,
        bool negated_pivot
    ) : Claim(negated_pivot, tautology_predicate, hard_clause_predicate) {
        if (clauses.size() < 3) {
            throw std::runtime_error("Claim must have at least two clauses.");
        }

        int32_t pivot = rule.get_pivot();

        // Remove the pivot literal from the clauses
        std::vector<int32_t> literals_1 = clauses[0].second.get_literals();
        std::vector<int32_t> literals_2 = clauses[1].second.get_literals();
        auto it = std::find(literals_1.begin(), literals_1.end(), pivot);
        if (it != literals_1.end()) {
            literals_1.erase(it);
        }
        it = std::find(literals_2.begin(), literals_2.end(), -pivot);
        if (it != literals_2.end()) {
            literals_2.erase(it);
        }

        // Sort the literals to ensure consistent ordering (a_1 ... a_n, b_1 ... b_m, pivot)
        initialize_vars(pivot, literals_1, literals_2, variable_supplier);
        vars.push_back(variable_supplier(pivot));


        std::vector<Lit> blocking_vars;
        for (const auto &clause : clauses) {
            blocking_vars.push_back(clause.first);
        }
        std::vector<Lit> new_blocking_vars = std::vector<Lit>(blocking_vars.begin() + 2, blocking_vars.end());

        uint32_t num_clauses_1 = literals_1.size();
        uint32_t num_clauses_2 = literals_2.size();

        this->active_blocking_vars = { new_blocking_vars[0] };

        if (negated_pivot) {
           this->pivot_literal = pivot < 0 ? neg(variable_supplier(pivot)) : variable_supplier(pivot);
           this->active_original_blocking_var = blocking_vars[0];
           this->inactive_original_blocking_var = blocking_vars[1];
    
           this->active_blocking_vars.insert(this->active_blocking_vars.end(), new_blocking_vars.begin() + 1, new_blocking_vars.end() - num_clauses_1);
           this->inactive_blocking_vars = std::vector<Lit>(new_blocking_vars.begin() + num_clauses_2 + 1, new_blocking_vars.end());
        } else {
           this->pivot_literal = pivot < 0 ? variable_supplier(pivot) : neg(variable_supplier(pivot));
           this->active_original_blocking_var = blocking_vars[1];
           this->inactive_original_blocking_var = blocking_vars[0];
    
           this->active_blocking_vars.insert(this->active_blocking_vars.end(), new_blocking_vars.begin() + num_clauses_2 + 1, new_blocking_vars.end());
           this->inactive_blocking_vars = std::vector<Lit>(new_blocking_vars.begin() + 1, new_blocking_vars.end() - num_clauses_1);
        }
    }

    const std::vector<Lit> &ResClaim::get_vars() const {
        return vars;
    }

    const std::vector<Lit> &ResClaim::get_literals() const {
        return literals;
    }

    const Lit &ResClaim::get_pivot_literal() const {
        return pivot_literal;
    }

    const Lit &ResClaim::get_active_original_blocking_var() const {
        return active_original_blocking_var;
    }

    const Lit &ResClaim::get_inactive_original_blocking_var() const {
        return inactive_original_blocking_var;
    }

    const std::vector<Lit> &ResClaim::get_active_blocking_vars() const {
        return active_blocking_vars;
    }

    const std::vector<Lit> &ResClaim::get_inactive_blocking_vars() const {
        return inactive_blocking_vars;
    }

    constraintid ResClaim::weaken(Prooflogger &pl, constraintid id, const std::vector<Lit> &variables, uint32_t begin, uint32_t end) {
        CuttingPlanesDerivation cpder(&pl, false);
        cpder.start_from_constraint(id);
        for (int i = begin; i < end; i++) {
            cpder.weaken(variable(variables[i]));
        }
        cpder.saturate();
        return cpder.end();
    }

    constraintid ResClaim::weaken_all_except(Prooflogger &pl, constraintid id, const std::vector<Lit> &variables, uint32_t except) {
        return weaken_all_except(pl, id, variables, except, except);
    }

    constraintid ResClaim::weaken_all_except(Prooflogger &pl, constraintid id, const std::vector<Lit> &variables, uint32_t begin, uint32_t end) {
        CuttingPlanesDerivation cpder(&pl, false);
        cpder.start_from_constraint(id);

        // TODO: This can be optimized
        std::unordered_set<Lit, LitHash, LitEqual> vars_set(variables.begin() + begin, variables.begin() + end + 1);
        for (int i = 0; i < variables.size(); i++) {
            if ((i < begin || i > end) && !(get_vars()[begin].v.v == variables[i].v.v)) {
                cpder.weaken(variable(variables[i]));
            }
        }
        cpder.saturate();
        return cpder.end();
    }

    constraintid ResClaim::add_all(Prooflogger &pl, const std::vector<constraintid> &constraints) {
        CuttingPlanesDerivation cpder(&pl, false);
        cpder.start_from_constraint(constraints[0]);
        for (size_t i = 1; i < constraints.size(); i++) {
            cpder.add_constraint(constraints[i]);
        }
        cpder.saturate();
        return cpder.end();
    }

    constraintid ResClaim::add_all_from_literal(Prooflogger &pl, const std::vector<constraintid> &constraints, const Lit lit) {
        CuttingPlanesDerivation cpder(&pl, false);
        cpder.start_from_literal_axiom(lit);
        for (size_t i = 0; i < constraints.size(); i++) {
            cpder.add_constraint(constraints[i]);
        }
        cpder.saturate();
        return cpder.end();
    }

    constraintid ResClaim::add_all_prev(Prooflogger &pl, uint32_t range) {
        CuttingPlanesDerivation cpder(&pl, false);
        cpder.start_from_constraint(-1);
        for (int32_t i = 1; i < range; i++) {
            cpder.add_constraint(-i - 1);
        }
        cpder.saturate();
        return cpder.end();
    }

    constraintid ResClaim::add_all_prev_from_literal(Prooflogger &pl, uint32_t range, const Lit lit) {
        CuttingPlanesDerivation cpder(&pl, false);
        cpder.start_from_literal_axiom(lit);
        for (int32_t i = 0; i < range; i++) {
            cpder.add_constraint(-i - 1);
        }
        cpder.saturate();
        return cpder.end();
    }

    constraintid ResClaim::build_proof_by_contradiction(Prooflogger &pl, Constraint<Lit, uint32_t, uint32_t> &C, constraintid claim_1, constraintid claim_2) {
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

    constraintid ResClaim::build_proof_by_contradiction(Prooflogger &pl, Constraint<Lit, uint32_t, uint32_t> &C, std::vector<VeriPB::constraintid> &claims) {
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

    void ResClaim::initialize_vars(int32_t pivot, const std::vector<int32_t>& literals_1, const std::vector<int32_t>& literals_2, std::function<VeriPB::Lit(int32_t)> variable_supplier) {
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