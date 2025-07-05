#include <algorithm>
#include "ClaimTypeA.h"

namespace converter {
    VeriPB::constraintid ClaimTypeA::write(Prooflogger &pl) {
        const int32_t RHS = get_blocking_vars().size() - 4;

        std::vector<Lit> vars_without_pivot = get_vars();
        vars_without_pivot.pop_back(); // Remove the pivot variable

        CuttingPlanesDerivation cpder(&pl, false);
        constraintid completed;
        std::vector<constraintid> subclaims;

        std::vector<constraintid> active_constraints;
        for (const Lit &sn : get_active_blocking_vars()) {
            active_constraints.push_back(pl.get_reified_constraint_left_implication(variable(sn)));
        }
        set_active_constraints(active_constraints);


        subclaims = build_iterative_subclaims(pl);
        iterative_proofs_by_contradiction(pl, subclaims);

        // Complete the formula (1 ~x 1 b1 1 b2 ... 1 bn 1 s2 1 ~s3 1 ~s(n+1) ... 1 ~s(n+m) >= m)
        cpder.start_from_constraint(pl.get_reified_constraint_right_implication(variable(get_active_original_blocking_var())));
        cpder.add_constraint(-1);
        completed = cpder.end();

        subclaims = build_conjunctive_subclaims(pl);
        subclaims.push_back(completed);

        // Make contradicting constraint
        Constraint<Lit, uint32_t, uint32_t> C;
        C.add_literal(get_pivot_literal(), 1);
        C.add_literal(neg(get_active_original_blocking_var()), 1);
        for (auto &sn : get_active_blocking_vars()) {
            C.add_literal(neg(sn), 1);
        }
        C.add_RHS(get_active_blocking_vars().size());

        build_proof_by_contradiction(pl, C, subclaims);

        // Complete the final claim
        cpder.start_from_literal_axiom(get_pivot_literal());
        cpder.multiply(RHS - 1);
        cpder.add_constraint(-1);
        cpder.end();

        // TODO: This should be put in a separete function
        for (Lit sn : get_unactive_blocking_vars()) {
            constraintid constraint = pl.get_reified_constraint_left_implication(variable(sn));

            // TODO: Add weakening restriction
            weaken_all_except(pl, constraint, get_vars(), get_vars().size() - 1);
        }

        constraintid result = add_all_prev_from_literal(pl, get_unactive_blocking_vars().size() + 1, neg(get_unactive_original_blocking_var())); // Also add the constraint that adds the pivot variable

        pl.write_comment("Claim 1");
        pl.write_comment("");
        return result;
    }

    std::vector<constraintid> ClaimTypeA::build_iterative_subclaims(Prooflogger &pl) {
        std::vector<Lit> vars_without_pivot = get_vars();
        vars_without_pivot.pop_back(); // Remove the pivot variable

        std::vector<constraintid> result;

        uint32_t offset = is_negated_pivot() ? get_unactive_blocking_vars().size() : 0;
        uint32_t num_active_vars = get_active_constraints().size() - 1;

        // Initial constraint
        for (uint32_t i = 0; i < get_active_blocking_vars().size() - 1; i++) {
            weaken_all_except(pl, get_active_constraints()[i + 1], get_vars(), i + offset, (num_active_vars - 1) + offset);
        }
        constraintid initial = add_all_prev_from_literal(pl, num_active_vars, neg(get_active_blocking_vars()[0]));

        pl.write_comment("Initial constraint");
        pl.write_comment("");

        // Reapeat for the reamining constraints
        for (uint32_t i = 0; i < num_active_vars; i++) {
            weaken_all_except(pl, get_active_constraints()[0], vars_without_pivot, i + offset);

            for (uint32_t j = 0; j < num_active_vars; j++) {
                if (i == j) continue;

                uint32_t n = (j <= i) ? j : i;
                weaken_all_except(pl, get_active_constraints()[j + 1], get_vars(), n + offset);
            }

            Lit sn = get_active_blocking_vars()[i + 1];
            constraintid curr_constraint = add_all_prev_from_literal(pl, num_active_vars, neg(sn));
            result.push_back(curr_constraint);

            pl.write_comment("Subclaim " + std::to_string(i));
            pl.write_comment("");
        }

        result.push_back(initial);
        return result;
    }

    VeriPB::constraintid ClaimTypeA::iterative_proofs_by_contradiction(Prooflogger &pl, std::vector<constraintid> &subclaims) {
        CuttingPlanesDerivation cpder(&pl, false);

        constraintid subclaim_1 = subclaims.back();
        constraintid subclaim_2;
        for (int i = subclaims.size() - 2; i >= 0; i--) {
            uint32_t offset = is_negated_pivot() ? get_unactive_blocking_vars().size() : 0;
            subclaim_2 = subclaims[i];

            // Build the constraint
            Constraint<Lit, uint32_t, uint32_t> C;
            for (Lit sn : get_active_blocking_vars()) {
                C.add_literal(neg(sn), 1);
            }
            for (int j = 0; j < i; j++) {
                C.add_literal(get_vars()[offset + j], 1);
            }
            C.add_RHS(subclaims.size() - 1);

            // Start the proof
            pl.write_comment("Proof by contradiction" + std::to_string(i));
            pl.write_comment("");
            build_proof_by_contradiction(pl, C, subclaim_1, subclaim_2);

            // Add the missing literal to the result for the next iteration
            if (i != 0) {
                cpder.start_from_constraint(-1);
                cpder.add_literal_axiom(get_vars()[offset + i - 1]);
                cpder.end();
            }

            subclaim_1 = pl.get_constraint_counter();
        }

        return subclaim_1;
    }

    std::vector<VeriPB::constraintid> ClaimTypeA::build_conjunctive_subclaims(Prooflogger &pl) {
        CuttingPlanesDerivation cpder(&pl, false);

        uint32_t num_active_vars = get_active_blocking_vars().size() - 1;
        uint32_t num_unactive_vars = get_unactive_blocking_vars().size();
        uint32_t offset = is_negated_pivot() ? 0 : num_active_vars;

        std::vector<Lit> vars_without_pivot = get_vars();
        vars_without_pivot.pop_back(); // Remove the pivot variable

        std::vector<VeriPB::constraintid> subclaims;

        for (uint32_t i = 0; i < num_unactive_vars; i++) {
            weaken_all_except(pl, get_active_constraints()[0], vars_without_pivot, i + offset);

            for (uint32_t j = 1; j < get_active_constraints().size(); j++) {
                // TODO: Not all the a's are used, so this can be optimized
                weaken_all_except(pl, get_active_constraints()[j], get_vars(), i + offset);
            }

            add_all_prev(pl, get_active_constraints().size());

            // Add the missing literals
            cpder.start_from_constraint(-1);
            cpder.add_literal_axiom(neg(get_active_original_blocking_var()));
            cpder.add_literal_axiom(get_pivot_literal());
            subclaims.push_back(cpder.end());
        }

        return subclaims;
    }
}