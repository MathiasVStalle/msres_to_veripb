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

            ClaimTypeB(const cnf::ResRule &rule, const std::vector<std::pair<VeriPB::Lit, cnf::Clause>> &clauses, const std::function<VeriPB::Lit(int32_t)> &variable_supplier, bool negated_pivot)
            : Claim(rule, clauses, variable_supplier, negated_pivot) {}

            constraintid write(Prooflogger &pl) override;
    };
}

#endif // CLAIM_TYPE_2_H