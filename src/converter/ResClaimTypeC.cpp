
#include "ResClaimTypeC.h"

namespace converter {

    VeriPB::constraintid ResClaimTypeC::write(Prooflogger &pl) {
        constraintid cn_1;

        CuttingPlanesDerivation cpder(&pl, false);

        std::unordered_set<Lit, LitHash, LitEqual> literals_to_add;
        cn_1 = add_all_and_saturate(pl, get_active_blocking_vars(), literals_to_add);

        cpder.start_from_constraint(pl.get_reified_constraint_left_implication(variable(get_active_original_blocking_var())));
        cpder.add_constraint(cn_1);
        cpder.saturate();

        // Add the missing literals
        for (const Lit &sn : get_inactive_blocking_vars()) {
            cpder.add_literal_axiom(neg(sn));
        }

        return cpder.end();
    }
}