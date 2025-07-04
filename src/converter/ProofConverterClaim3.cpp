#include "ProofConverter.h"

using namespace VeriPB;

namespace converter {
    VeriPB::constraintid ProofConverter::claim_3(
        uint32_t clause_id_1,
        uint32_t clause_id_2,
        const cnf::ResRule& rule,
        const std::vector<cnf::Clause>& new_clauses
    ) {
        VeriPB::CuttingPlanesDerivation cpder(this->pl, false);

        std::unordered_set<int32_t> literals_set_clause_1 = rule.getClause1().getLiterals();
        std::unordered_set<int32_t> literals_set_clause_2 = rule.getClause2().getLiterals();

        int32_t rhs = new_clauses.size() - 2;

        // Remove pivot literal from both clauses
        uint32_t pivot = rule.get_pivot();
        literals_set_clause_1.erase(pivot);
        literals_set_clause_2.erase(-pivot);

        // Ordered set for iteration
        std::vector<int32_t> literals_clause_1(literals_set_clause_1.begin(), literals_set_clause_1.end());
        std::vector<int32_t> literals_clause_2(literals_set_clause_2.begin(), literals_set_clause_2.end());

        // Frequently used variables
        Lit x = this->vars[pivot];
        Lit s1 = this->blocking_vars[clause_id_1];
        Lit s2 = this->blocking_vars[clause_id_2];
        Lit s3 = this->blocking_vars[blocking_vars.size() - (new_clauses.size() - 1)];


        // Step 1
        cpder.start_from_constraint(pl->get_reified_constraint_right_implication(variable(s3)));
        for (int i = 0; i < literals_clause_1.size(); i++) {
            Lit sn = blocking_vars[blocking_vars.size() - i];
            cpder.add_constraint(pl->get_reified_constraint_right_implication(variable(sn)));
            cpder.saturate();
        }
        constraintid cxn_1 = cpder.end();

        // Step 2
        cpder.start_from_constraint(pl->get_reified_constraint_left_implication(variable(s2)));
        cpder.weaken(variable(x));
        cpder.saturate();
        cpder.end();

        cpder.start_from_literal_axiom(neg(x));
        cpder.add_literal_axiom(s3);
        for (int i = 0; i < literals_clause_1.size(); i++) {
            Lit sn = blocking_vars[blocking_vars.size() - i];
            cpder.add_literal_axiom(sn);
        }
        cpder.multiply((int32_t) literals_clause_2.size());
        cpder.add_constraint(-1);
        constraintid cxn_2 = cpder.end();
        pl->write_comment("Step 2");
        pl->write_comment("");


        // Case Splitting
        Constraint<VeriPB::Lit, uint32_t, uint32_t> C;
        C.add_literal(neg(x), 1);
        C.add_literal(s2, 1);
        C.add_literal(s3, 1);
        for (int i = 0; i < literals_clause_1.size(); i++) {
            C.add_literal(blocking_vars[blocking_vars.size() - i], 1);
        }
        C.add_RHS(1);

        pl->start_proof_by_contradiction(C);
        
        cpder.start_from_constraint(-1);
        cpder.multiply((int32_t)literals_clause_2.size());
        cpder.add_constraint(cxn_2);
        cpder.saturate();
        cpder.end();

        cpder.start_from_constraint(-2);
        cpder.add_constraint(cxn_1);
        cpder.saturate();
        cpder.end();

        cpder.start_from_constraint(-2);
        cpder.add_constraint(-1);
        cpder.end();

        constraintid cxnneg = pl->end_proof_by_contradiction();

        cpder.start_from_constraint(pl->get_reified_constraint_left_implication(variable(s1)));
        for (int i = 0; i < literals_clause_1.size(); i++) {
            cpder.weaken(variable(vars[std::abs(literals_clause_1[i])]));
        }
        cpder.saturate();
        cpder.end();

        cpder.start_from_constraint(-2);
        cpder.add_constraint(-1);
        for (int i = 0; i < literals_clause_2.size(); i++) {
            cpder.add_literal_axiom(blocking_vars[blocking_vars.size() - literals_clause_1.size() - i]);
        }
        pl->write_comment("Step 3");
        pl->write_comment("");
        return cpder.end();
    }

    VeriPB::constraintid ProofConverter::claim_type_2(const cnf::ResRule &rule, const bool negated_pivot)
    {
        const uint32_t clause_id_1 = constraint_ids[rule.getClause1()];
        const uint32_t clause_id_2 = constraint_ids[rule.getClause2()];
        const uint32_t num_new_clauses = rule.getClause1().getLiterals().size() + rule.getClause2().getLiterals().size() - 1; // TODO: This should be changed to the actual number of new clauses
        const int32_t rhs = num_new_clauses - 2;
        const uint32_t pivot = rule.get_pivot();

        Lit pivot_literal;
        Lit active_original_blocking_var;
        Lit unactive_original_blocking_var;
        uint32_t active_blocking_var_offset;
        uint32_t active_constraints_amount;
        uint32_t unactive_constraints_amount;
        uint32_t active_constraints_offset;

        VeriPB::CuttingPlanesDerivation cpder(this->pl, false);

        // Obtain list of the literals in the clauses without the pivot variable
        std::unordered_set<int32_t> literals_set_clause_1 = rule.getClause1().getLiterals();
        std::unordered_set<int32_t> literals_set_clause_2 = rule.getClause2().getLiterals();
        literals_set_clause_1.erase(pivot);
        literals_set_clause_2.erase(-pivot);
        std::vector<int32_t> literals_clause_1(literals_set_clause_1.begin(), literals_set_clause_1.end());
        std::vector<int32_t> literals_clause_2(literals_set_clause_2.begin(), literals_set_clause_2.end());

        // Frequently used variables
        const Lit x = this->vars[pivot];
        const Lit s1 = this->blocking_vars[clause_id_1];
        const Lit s2 = this->blocking_vars[clause_id_2];
        const Lit s3 = this->blocking_vars[blocking_vars.size() - (num_new_clauses - 1)];

        if (negated_pivot)
        {
            pivot_literal = x;
            active_original_blocking_var = s1;
            unactive_original_blocking_var = s2;
            active_blocking_var_offset = num_new_clauses - 2; // TODO: Check why not num_new_clauses - 1
            active_constraints_amount = literals_clause_2.size();
            unactive_constraints_amount = literals_clause_1.size();
            active_constraints_offset = literals_clause_1.size();
        }
        else
        { // Default case
            pivot_literal = neg(x);
            active_original_blocking_var = s2;
            unactive_original_blocking_var = s1;
            active_blocking_var_offset = literals_clause_1.size() - 1;
            active_constraints_amount = literals_clause_1.size();
            unactive_constraints_amount = literals_clause_2.size();
            active_constraints_offset = 0;
        }

        std::vector<VeriPB::Lit> total_vars = get_total_vars(literals_clause_1, literals_clause_2);

        // List all the blocking variables of the active constraints
        std::vector<VeriPB::Lit> active_blocking_vars = {s3};
        for (int i = 0; i < active_constraints_amount; i++) {
            Lit sn = blocking_vars[blocking_vars.size() - active_blocking_var_offset + i];
            active_blocking_vars.push_back(sn);
        }

        // List of the relavant constraint ids sorted by the way they are added to the proof
        std::vector<VeriPB::constraintid> active_constraints;
        for (Lit sn : active_blocking_vars) {
            active_constraints.push_back(pl->get_reified_constraint_right_implication(variable(sn)));
        }

        constraintid cxn_1 = add_all_and_saturate(active_constraints);

        cpder.start_from_constraint(pl->get_reified_constraint_left_implication(variable(active_original_blocking_var)));
        cpder.weaken(variable(x));
        cpder.saturate();
        cpder.end();

        cpder.start_from_literal_axiom(pivot_literal);
        for (Lit sn : active_blocking_vars) {
            cpder.add_literal_axiom(sn);
        }
        cpder.multiply((int32_t)literals_clause_2.size()); // TODO: This should be changed
        cpder.add_constraint(-1);
        constraintid cxn_2 = cpder.end();
        pl->write_comment("Step 2");
        pl->write_comment("");

        // Build contradicting constraint
        Constraint<VeriPB::Lit, uint32_t, uint32_t> C;
        C.add_literal(pivot_literal, 1);
        C.add_literal(active_original_blocking_var, 1);
        for (Lit sn : active_blocking_vars) {
            C.add_literal(sn, 1);
        }
        C.add_RHS(1);

        // TODO: Put this in separate function
        pl->start_proof_by_contradiction(C);

        cpder.start_from_constraint(-1);
        cpder.multiply((int32_t)unactive_constraints_amount);
        cpder.add_constraint(cxn_2);
        cpder.saturate();
        cpder.end();

        cpder.start_from_constraint(-2);
        cpder.add_constraint(cxn_1);
        cpder.saturate();
        cpder.end();

        cpder.start_from_constraint(-2);
        cpder.add_constraint(-1);
        cpder.end();

        constraintid cxnneg = pl->end_proof_by_contradiction();

        // TODO: unactive variables list
        constraintid id = pl->get_reified_constraint_left_implication(variable(unactive_original_blocking_var));
        weaken(id, total_vars, active_constraints_offset, active_constraints_offset + active_constraints_amount);

        cpder.start_from_constraint(-2);
        cpder.add_constraint(-1);
        int32_t offset = negated_pivot ? blocking_vars.size() : (blocking_vars.size() - literals_clause_1.size());
        for (int i = 0; i < unactive_constraints_amount; i++) {
            cpder.add_literal_axiom(blocking_vars[offset - i]);
        }
        return cpder.end();
    }
}