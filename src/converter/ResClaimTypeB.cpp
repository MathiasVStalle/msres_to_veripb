#include <algorithm>
#include "ResClaimTypeB.h"

namespace converter {

    VeriPB::constraintid ResClaimTypeB::write(Prooflogger &pl) {
        const int32_t RHS = get_blocking_vars().size() - 3;
        constraintid cn_1;
        constraintid cn_2;
        constraintid cnneg;
        uint32_t offset = is_negated_pivot() ? get_inactive_blocking_vars().size() : 0;

        CuttingPlanesDerivation cpder(&pl, false);

        std::vector<Lit> vars_without_pivot = get_vars();
        vars_without_pivot.pop_back(); // Remove the pivot variable

        std::unordered_set<Lit, LitHash, LitEqual> literals_to_add;
        cn_1 = add_all_and_saturate(pl, get_active_blocking_vars(), literals_to_add);

        // TODO: Hard clause
        cpder.start_from_constraint(pl.get_reified_constraint_left_implication(variable(get_active_original_blocking_var())));
        cpder.weaken(variable(get_pivot_literal()));
        cpder.saturate();
        cpder.end();

        cpder.start_from_literal_axiom(get_pivot_literal());
        for (Lit sn : get_active_blocking_vars()) {
            if (is_tautology(sn)) continue;
            cpder.add_literal_axiom(neg(sn)); 
        }
        cpder.multiply((int32_t) get_inactive_blocking_vars().size());
        cpder.add_constraint(-1);
        cn_2 = cpder.end();
        pl.write_comment("Step 2");
        pl.write_comment("");

        // Build contradicting constraint
        Constraint<Lit, uint32_t, uint32_t> C;
        C.add_literal(get_pivot_literal(), 1);
        C.add_literal(get_active_original_blocking_var(), 1);
        for (Lit sn : get_active_blocking_vars()) {
            C.add_literal(neg(sn), 1);
        }
        C.add_RHS(1);

        // TODO: Put this in separate function
        pl.start_proof_by_contradiction(C);

        cpder.start_from_constraint(-1);
        cpder.multiply((int32_t) get_inactive_blocking_vars().size());
        cpder.add_constraint(cn_2);
        cpder.saturate();
        cpder.end();

        cpder.start_from_constraint(-2);
        cpder.add_constraint(cn_1);
        cpder.saturate();
        cpder.end();

        cpder.start_from_constraint(-2);
        cpder.add_constraint(-1);
        cpder.end();

        cnneg = pl.end_proof_by_contradiction();


        constraintid id = pl.get_reified_constraint_left_implication(variable(get_inactive_original_blocking_var()));
        weaken(pl, id, vars_without_pivot, offset, offset + get_active_blocking_vars().size() - 1);

        cpder.start_from_constraint(-2);
        cpder.add_constraint(-1);
        for (Lit sn : get_inactive_blocking_vars()) {
            cpder.add_literal_axiom(neg(sn));
        }
        return cpder.end();
    }

}