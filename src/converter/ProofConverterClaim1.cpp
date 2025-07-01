#include "ProofConverter.h"

using namespace VeriPB;

namespace converter {
    std::vector<VeriPB::Lit> ProofConverter::get_total_vars(
        const std::vector<int32_t>& literals_1,
        const std::vector<int32_t>& literals_2
    ) {
        std::vector<VeriPB::Lit> total_vars;

        for (const auto& lit : literals_1) {
            total_vars.push_back(this->vars[std::abs(lit)]);
        }
        for (const auto& lit : literals_2) {
            total_vars.push_back(this->vars[std::abs(lit)]);
        }

        return total_vars;
    }

    VeriPB::constraintid ProofConverter::weaken_all_except(VeriPB::constraintid id, std::vector<VeriPB::Lit> &literals, uint32_t except) {
        return weaken_all_except(id, literals, except, except);
    }

    VeriPB::constraintid ProofConverter::weaken_all_except(
        VeriPB::constraintid id,
        std::vector<VeriPB::Lit> &literals,
        uint32_t begin,
        uint32_t end
    ) {
        CuttingPlanesDerivation cpder(this->pl, false);
        cpder.start_from_constraint(id);
        for (int i = 0; i < literals.size(); i++) {
            if (i < begin || i > end) {
                cpder.weaken(variable(literals[i]));
            }
        }

        cpder.saturate();
        return cpder.end();
    }

    VeriPB::constraintid ProofConverter::add_all(std::vector<VeriPB::constraintid> constraints) {
        CuttingPlanesDerivation cpder(this->pl, false);
        cpder.start_from_constraint(constraints[0]);
        for (size_t i = 1; i < constraints.size(); i++) {
            cpder.add_constraint(constraints[i]);
        }
        cpder.saturate();
        return cpder.end();
    }

    // TODO: Ckeck if the indexing is correct (Check reference)
    VeriPB::constraintid ProofConverter::add_all_prev(int32_t range) {
        CuttingPlanesDerivation cpder(this->pl, false);
        cpder.start_from_constraint(-1);
        for (int32_t i = 1; i < range; i++) {
            cpder.add_constraint(-i - 1);
        }
        cpder.saturate();
        return cpder.end();
    }

    VeriPB::constraintid ProofConverter::add_all_prev_from_literal(int32_t range, VeriPB::Lit var) {
        CuttingPlanesDerivation cpder(this->pl, false);
        cpder.start_from_literal_axiom(var);
        for (int32_t i = 0; i < range; i++) {
            cpder.add_constraint(-i - 1);
        }
        cpder.saturate();
        return cpder.end();
    }

    VeriPB::constraintid ProofConverter::add_all_from_literal(std::vector<VeriPB::constraintid> constraints, VeriPB::Lit var)
    {
        CuttingPlanesDerivation cpder(this->pl, false);
        cpder.start_from_literal_axiom(var);
        for (size_t i = 0; i < constraints.size(); i++) {
            cpder.add_constraint(constraints[i]);
        }
        cpder.saturate();
        return cpder.end();
    }

    VeriPB::constraintid ProofConverter::build_proof_by_contradiction(
        VeriPB::Constraint<VeriPB::Lit, uint32_t, uint32_t> &C,
        VeriPB::constraintid claim_1,
        VeriPB::constraintid claim_2)
    {
        pl->start_proof_by_contradiction(C);

        CuttingPlanesDerivation cpder(pl, false);
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

        return pl->end_proof_by_contradiction();
    }




    VeriPB::constraintid ProofConverter::claim_1(
        const uint32_t clause_id_1,
        const uint32_t clause_id_2,
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

        // Excecute the proof steps
        std::vector<VeriPB::constraintid> subclaims = claim_1_step_2(
            cpder,
            x,
            s2,
            s3,
            literals_clause_1,
            literals_clause_2
        );
        VeriPB::constraintid cxn_4 = claim_1_contradiction(
            cpder,
            x,
            s2,
            s3,
            literals_clause_1,
            literals_clause_2,
            subclaims
        );
        
        // Build the final claim
        cpder.start_from_literal_axiom(neg(x));
        cpder.multiply(rhs - 1);
        cpder.add_constraint(-1);
        VeriPB::constraintid subclaim = cpder.end();
        
        int32_t counter = 0;
        int32_t offset = blocking_vars.size() - literals_clause_1.size();
        for (int i = 0; i < literals_clause_2.size(); i++) {
            VeriPB::Lit sn = this->blocking_vars[offset - i];
        
            // Take all the unused constraints and weaken them
            cpder.start_from_constraint(this->pl->get_reified_constraint_left_implication(variable(sn)));
            for (int j = 0; j < literals_clause_1.size(); j++) {
                cpder.weaken(variable(this->vars[std::abs(literals_clause_1[j])]));
            }
            for (int j = 0; j < literals_clause_2.size() - i; j++) {
                cpder.weaken(variable(this->vars[std::abs(literals_clause_2[j])]));
            }
            cpder.saturate();
            cpder.end();
            counter++;
        }

        // Add all the previous constraints
        if (counter != 0) {
            cpder.start_from_constraint(-counter);
            for (int i = counter - 1; i > 0; i--) {
                cpder.add_constraint(-i);
            }
            cpder.add_constraint(subclaim);
        } else {
            cpder.start_from_constraint(subclaim);
        }
        cpder.add_literal_axiom(neg(s1));
        VeriPB::constraintid result = cpder.end();
        pl->write_comment("Claim 1");
        pl->write_comment("");
        
        this->pl->flush_proof();
        
        return result;
    }

    // TODO: If the total amount of subclaims is large, dynamic allocation should be used
    std::vector<VeriPB::constraintid> ProofConverter::build_iterative_subclaims(
        VeriPB::Lit x,
        std::vector<VeriPB::Lit> &total_vars,
        std::vector<VeriPB::Lit> &active_blocking_vars,
        std::vector<VeriPB::constraintid> &active_constraints
    ) {
        uint32_t active_vars_amount = active_blocking_vars.size() - 1;
        std::vector<VeriPB::Lit> total_vars_with_x = total_vars;
        total_vars_with_x.push_back(x);

        std::vector<VeriPB::constraintid> subclaims;

        // Initial constraint
        for (int j = 0; j < active_constraints.size() - 1; j++)
        {
            weaken_all_except(active_constraints[j + 1], total_vars_with_x, j, active_vars_amount - 1);
        }
        constraintid intitial = add_all_prev_from_literal(active_constraints.size() - 1, neg(active_blocking_vars[0]));

        pl->write_comment("Initial constraint");
        pl->write_comment("");

        // Repeat for the remaining constraints
        for (int i = 0; i < active_vars_amount; i++) {
            weaken_all_except(active_constraints[0], total_vars, i);

            for (int j = 0; j < active_constraints.size() - 1; j++) {
                if (i == j) continue;

                uint32_t n = (j <= i) ? j : i;
                weaken_all_except(active_constraints[j + 1], total_vars_with_x, n);
            }

            Lit sn = active_blocking_vars[i + 1];
            constraintid curr_constraint = add_all_prev_from_literal(active_constraints.size() - 1, neg(sn));
            subclaims.push_back(curr_constraint);

            pl->write_comment("Subclaim " + std::to_string(i));
            pl->write_comment("");
        }

        subclaims.push_back(intitial);

        return subclaims;
    }

    VeriPB::constraintid ProofConverter::iterative_proofs_by_contradiction(
        std::vector<int32_t>& active_literals,
        std::vector<VeriPB::Lit>& active_blocking_vars,
        std::vector<VeriPB::constraintid>& subclaims
    ) {
        CuttingPlanesDerivation cpder(pl, false);

        constraintid subclaim_1 = subclaims.back();
        constraintid subclaim_2;
        for (int i = subclaims.size() - 2; i >= 0; i--) {
            subclaim_2 = subclaims[i];

            // Build the constraint
            Constraint<VeriPB::Lit, uint32_t, uint32_t> C;
            for (Lit sn : active_blocking_vars) {
                C.add_literal(neg(sn), 1);
            }
            for (int j = 0; j < i; j++) {
                C.add_literal(vars[std::abs(active_literals[j])], 1);
            }
            C.add_RHS(subclaims.size() - 1);

            // Start the proof
            pl->write_comment("Proof by contradiction" + std::to_string(i));
            pl->write_comment("");
            build_proof_by_contradiction(C, subclaim_1, subclaim_2);

            // Add the missing literal to the result for the next iteration
            if (i != 0) {
                cpder.start_from_constraint(-1);
                cpder.add_literal_axiom(vars[std::abs(active_literals[i - 1])]);
                cpder.end();
            }

            subclaim_1 = pl->get_constraint_counter();
        }

        return subclaim_1;
    }

    std::vector<VeriPB::constraintid> ProofConverter::build_conjunctive_subclaims(
        VeriPB::Lit x,
        VeriPB::Lit s2,
        std::vector<VeriPB::Lit>& total_vars,
        std::vector<VeriPB::constraintid>& active_constraints
    ) {
        CuttingPlanesDerivation cpder(pl, false);
        uint32_t active_vars_amount = active_constraints.size() - 1;
        uint32_t unactive_vars_amount = total_vars.size() - active_vars_amount;
        std::vector<VeriPB::constraintid> subclaims;

        std::vector<VeriPB::Lit> total_vars_with_x = total_vars;
        total_vars_with_x.push_back(x);

        // Repeat for every b_i
        for (int i = 0; i < unactive_vars_amount; i++) {
            weaken_all_except(active_constraints[0], total_vars, active_vars_amount + i);

            // Weaken on everything except b_i
            for (int j = 1; j < active_constraints.size(); j++) {
                // TODO: Not all the a's are used, so this can be optimized
                weaken_all_except(active_constraints[j], total_vars_with_x, active_vars_amount + i);
            }

            // Add all the previous constraints
            add_all_prev(active_constraints.size());

            // Add the missing literals
            cpder.start_from_constraint(-1);
            cpder.add_literal_axiom(neg(s2));
            cpder.add_literal_axiom(neg(x));

            subclaims.push_back(cpder.end());   
        }

        return subclaims;
    }

    std::vector<constraintid> ProofConverter::claim_1_step_2(
        CuttingPlanesDerivation& cpder,
        Lit x,
        Lit s2,
        Lit s3,
        std::vector<int32_t>& literals_1, 
        std::vector<int32_t>& literals_2
    ) {
        // TODO: This should be optimized
        std::vector<VeriPB::Lit> total_vars = get_total_vars(literals_1, literals_2);

        // List all the blocking variables of the active constraints
        std::vector<VeriPB::Lit> active_blocking_vars = {s3};
        for (int i = 0; i < literals_1.size(); i++)
        {
            Lit sn = blocking_vars[blocking_vars.size() - (literals_1.size() - 1) + i];
            active_blocking_vars.push_back(sn);
        }

        // List of the relavant constraint ids sorted by the way they are added to the proof
        std::vector<VeriPB::constraintid> active_constraints;
        for (Lit sn : active_blocking_vars)
        {
            active_constraints.push_back(pl->get_reified_constraint_left_implication(variable(sn)));
        }

        std::vector<VeriPB::constraintid> subclaims = build_iterative_subclaims(x, total_vars, active_blocking_vars, active_constraints);
        iterative_proofs_by_contradiction(literals_1, active_blocking_vars, subclaims);

        // Complete the formula (1 ~x 1 b1 1 b2 ... 1 bn 1 s2 1 ~s3 1 ~s(n+1) ... 1 ~s(n+m) >= m)
        cpder.start_from_constraint(pl->get_reified_constraint_right_implication(variable(s2)));
        cpder.add_constraint(-1);
        constraintid completed = cpder.end();

        subclaims = build_conjunctive_subclaims(x, s2, total_vars, active_constraints);
        subclaims.push_back(completed);
        return subclaims;
    }

    VeriPB::constraintid ProofConverter::claim_1_contradiction(
        CuttingPlanesDerivation& cpder,
        Lit x,
        Lit s2,
        Lit s3,
        std::vector<int32_t>& literals_1, 
        std::vector<int32_t>& literals_2,
        std::vector<VeriPB::constraintid>& subclaims
    ) {
        uint32_t num_new_clauses = literals_1.size() + literals_2.size() + 1;

        // make contradicting constraint
        Constraint<VeriPB::Lit, uint32_t, uint32_t> C;
        C.add_literal(neg(x), 1);
        C.add_literal(neg(s2), 1);
        C.add_literal(neg(s3), 1);
        for (int i = literals_1.size() - 1; i >= 0; i--) {
            VeriPB::Lit sn = blocking_vars[blocking_vars.size() - i];
            C.add_literal(neg(sn), 1);
        }
        C.add_RHS(num_new_clauses - literals_2.size());

        // Proof by constradiction (1 ~x1 1 s2 1 ~s3 1 ~s6 ... 1 ~sn >= m)
        int32_t counter = 0;
        VeriPB::constraintid cxnneg = pl->start_proof_by_contradiction(C);
        for (auto& subclaim : subclaims) {
            cpder.start_from_constraint(cxnneg);
            cpder.add_constraint(subclaim);
            cpder.saturate();
            cpder.end();
            counter++;
        }

        // Add all the previous constraints
        cpder.start_from_constraint(-counter);
        for (int i = counter - 1; i > 0; i--) {
            cpder.add_constraint(-i);
        }
        cpder.saturate();
        cpder.end();

        cxnneg = pl->end_proof_by_contradiction();

        return cxnneg;
    }


}
