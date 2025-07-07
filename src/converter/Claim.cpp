#include "Claim.h"

namespace converter {

    Claim::Claim(
        const cnf::ResRule &rule, 
        const std::vector<Lit> &vars, 
        const std::vector<Lit> &blocking_vars,
        const bool negated_pivot
    ) : negated_pivot(negated_pivot), vars(vars), blocking_vars(blocking_vars) 
    {
        if (blocking_vars.size() < 3) {
            throw std::runtime_error("Blocking variables must have at least 3 elements.");
        }

        const uint32_t num_clauses_1 = rule.get_clause_1().get_literals().size() - 1; // -1 for the pivot variable
        const uint32_t num_clauses_2 = rule.get_clause_2().get_literals().size() - 1; // -1 for the pivot variable
        const std::vector<Lit> new_blocking_vars = std::vector<Lit>(blocking_vars.begin() + 2, blocking_vars.end());

        this->active_blocking_vars = {new_blocking_vars[0]}; // Add the first new blocking variables

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

        for (const Lit &var : this->active_vars) {
            std::cout << "Active var: " << var.v.v << (var.negated ? " (negated)" : "") << std::endl;
        }

        this->initialize_duplicate_vars(rule);
    }


    bool Claim::is_negated_pivot() const {
        return negated_pivot;
    }

    const std::vector<Lit> &Claim::get_vars() const {
        return vars;
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
        for (int i = 0; i < variables.size(); i++) {
            if ((i < begin || i > end)) {

                // TODO: This does not check if the duplicate is in the same range
                if (
                    duplicate_vars.find(variable(variables[i])) != duplicate_vars.end()
                    || possible_pivots.find(variable(variables[i])) != possible_pivots.end()
                ) {
                    continue;
                }

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
        if (claims.size() < 2) {
            throw std::runtime_error("At least two claims are required to build a proof by contradiction.");
        }
        
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


    void Claim::initialize_duplicate_vars(const cnf::ResRule &rule) {
        duplicate_vars.clear();
        possible_pivots.clear();

        const auto& clause_1 = rule.get_clause_1().get_literals();
        const auto& clause_2 = rule.get_clause_2().get_literals();

        const int32_t pivot = rule.get_pivot();

        std::vector<int32_t> active_variables = negated_pivot ? 
            std::vector<int32_t>(clause_1.begin(), clause_1.end()) :
            std::vector<int32_t>(clause_2.begin(), clause_2.end());

        std::unordered_set<int32_t> unactive_variables = negated_pivot ? 
            std::unordered_set<int32_t>(clause_2.begin(), clause_2.end()) :
            std::unordered_set<int32_t>(clause_1.begin(), clause_1.end());
    
        uint32_t index = negated_pivot ? 0 : clause_1.size() - 1;

        for (int32_t l : active_variables) {
            if (l == pivot || l == -pivot) continue;

            Var var = variable(vars[index]);

            if (unactive_variables.find(l) != unactive_variables.end()) {
                duplicate_vars[var] = index;
            } else 
            if (unactive_variables.find(-l) != unactive_variables.end()) {
                possible_pivots[var] = index;
            }

            index++;
        }
    }

    bool Claim::is_possible_pivot(const Lit &lit) const {
        return possible_pivots.find(variable(lit)) != possible_pivots.end();
    }

    bool Claim::is_duplicate(const Lit &lit) const {
        return duplicate_vars.find(variable(lit)) != duplicate_vars.end();
    }
}