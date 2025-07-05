#ifndef CLAIM_TYPE_1_H
#define CLAIM_TYPE_1_H

#include <vector>

#include "Claim.h"
#include "../cnf/ResRule.h"

#include "VeriPbSolverTypes.h"
#include "MaxSATProoflogger.h"

using namespace VeriPB;

namespace converter {
    class ClaimType1 final : public Claim {
        

        public:

            ClaimType1(
                const cnf::ResRule &rule, 
                const std::vector<Lit> &vars, 
                const std::vector<Lit> &blocking_vars,
                const bool negated_pivot
            ) : Claim(rule, vars, blocking_vars, negated_pivot) {}

            constraintid write(Prooflogger &pl) override;

        private:
        
            // TODO: These functions should return a pointer to a dynamically allocated vector when working with large proofs
            std::vector<constraintid> build_iterative_subclaims(Prooflogger &pl);
            constraintid iterative_proofs_by_contradiction(Prooflogger &pl, std::vector<constraintid> &subclaims);
            std::vector<VeriPB::constraintid> build_conjunctive_subclaims(Prooflogger &pl);
    };
}

#endif // CLAIM_TYPE_1_H