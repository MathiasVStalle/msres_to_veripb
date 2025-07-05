#ifndef CLAIM_TYPE_2_H
#define CLAIM_TYPE_2_H

#include <vector>

#include "Claim.h"
#include "../cnf/ResRule.h"

#include "VeriPbSolverTypes.h"
#include "MaxSATProoflogger.h"

namespace converter {
    class ClaimType2 final : public Claim {
        
        public:

            ClaimType2(
                const cnf::ResRule &rule, 
                const std::vector<Lit> &vars, 
                const std::vector<Lit> &blocking_vars,
                const bool negated_pivot
            ) : Claim(rule, vars, blocking_vars, negated_pivot) {}

            constraintid write(Prooflogger &pl) override;
    };
}

#endif // CLAIM_TYPE_2_H