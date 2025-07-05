#include <algorithm>
#include "ClaimType2.h"

namespace converter {

    VeriPB::constraintid ClaimType2::write(Prooflogger &pl) {
        const int32_t RHS = get_blocking_vars().size() - 3;
        constraintid cn_1;
        constraintid cn_2;
        constraintid cnneg;
        uint32_t offset = is_negated_pivot() ? get_unactive_blocking_vars().size() : 0;

        CuttingPlanesDerivation cpder(&pl, false);

        std::vector<Lit> vars_without_pivot = get_vars();
        vars_without_pivot.pop_back(); // Remove the pivot variable

        std::vector<constraintid> active_constraints;
        for (const Lit &sn : get_active_blocking_vars()) {
            active_constraints.push_back(pl.get_reified_constraint_right_implication(variable(sn)));
        }
        set_active_constraints(active_constraints);

        cn_1 = add_all_and_saturate(pl, get_active_constraints());

        cpder.start_from_constraint(pl.get_reified_constraint_left_implication(variable(get_active_original_blocking_var())));
        cpder.weaken(variable(get_pivot_literal()));
        cpder.saturate();
        cpder.end();

        cpder.start_from_literal_axiom(get_pivot_literal());
        for (Lit sn : get_active_blocking_vars()) {
            cpder.add_literal_axiom(sn); 
        }
        cpder.multiply((int32_t)get_unactive_blocking_vars().size()); // TODO: This should be changed
        cpder.add_constraint(-1);
        cn_2 = cpder.end();
        pl.write_comment("Step 2");
        pl.write_comment("");

        // Build contradicting constraint
        Constraint<Lit, uint32_t, uint32_t> C;
        C.add_literal(get_pivot_literal(), 1);
        C.add_literal(get_active_original_blocking_var(), 1);
        for (Lit sn : get_active_blocking_vars()) {
            C.add_literal(sn, 1);
        }
        C.add_RHS(1);

        // TODO: Put this in separate function
        pl.start_proof_by_contradiction(C);

        cpder.start_from_constraint(-1);
        cpder.multiply((int32_t) get_unactive_blocking_vars().size());
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

        constraintid id = pl.get_reified_constraint_left_implication(variable(get_unactive_original_blocking_var()));
        weaken(pl, id, vars_without_pivot, offset, offset + get_active_blocking_vars().size() - 1);

        std::vector<Lit> temp = get_unactive_blocking_vars();
        std::reverse(temp.begin(), temp.end());

        cpder.start_from_constraint(-2);
        cpder.add_constraint(-1);
        for (Lit sn : temp) {
            cpder.add_literal_axiom(sn);
        }
        return cpder.end();
    }

}