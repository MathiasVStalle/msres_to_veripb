#include "ClaimType1.h"

namespace converter {
    VeriPB::constraintid ClaimType1::write(Prooflogger &pl) {
        const int32_t RHS = get_blocking_vars().size() - 3;

        std::vector<Lit> vars_without_pivot = get_vars();
        vars_without_pivot.erase(vars_without_pivot.begin()); // Remove the pivot variable

        CuttingPlanesDerivation cpder(&pl, false);
        constraintid completed;
        std::vector<constraintid> subclaims;

        subclaims = build_iterative_subclaims(pl);
        iterative_proofs_by_contradiction(pl, subclaims);

        // Complete the formula (1 ~x 1 b1 1 b2 ... 1 bn 1 s2 1 ~s3 1 ~s(n+1) ... 1 ~s(n+m) >= m)
        cpder.start_from_constraint(pl.get_reified_constraint_left_implication(variable(get_active_original_blocking_var())));
        cpder.add_constraint(-1);
        completed = cpder.end();

        subclaims = build_conjunctive_subclaims(pl);
        subclaims.push_back(completed);

        // Make contradicting constraint
        Constraint<VeriPB::Lit, uint32_t, uint32_t> C;
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
            weaken_all_except(pl, constraint, 0);
        }

        constraintid result = add_all_prev_from_literal(pl, get_unactive_blocking_vars().size() + 1, neg(get_unactive_original_blocking_var())); // Also add the constraint that adds the pivot variable

        pl.write_comment("Claim 1");
        pl.write_comment("");
        return result;
    }

    std::vector<constraintid> ClaimType1::build_iterative_subclaims(Prooflogger &pl) {
        std::vector<Lit> vars_without_pivot = get_vars();
        vars_without_pivot.erase(vars_without_pivot.begin()); // Remove the pivot variable

        
    }
}