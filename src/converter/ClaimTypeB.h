#ifndef CLAIM_TYPE_B_H
#define CLAIM_TYPE_B_H

#include <vector>

#include "Claim.h"
#include "../cnf/ResRule.h"

#include "VeriPbSolverTypes.h"
#include "MaxSATProoflogger.h"

namespace converter {
    class ClaimTypeB final : public Claim {
        
        public:

            ClaimTypeB(
                const cnf::ResRule &rule, 
                const std::vector<Lit> &vars, 
                const std::vector<Lit> &blocking_vars,
                const bool negated_pivot
            ) : Claim(rule, vars, blocking_vars, negated_pivot) {}

            constraintid write(Prooflogger &pl) override;
    };
}

#endif // CLAIM_TYPE_2_H