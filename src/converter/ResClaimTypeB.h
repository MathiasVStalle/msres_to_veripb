#ifndef RES_CLAIM_TYPE_B_H
#define RES_CLAIM_TYPE_B_H

#include <vector>

#include "ResClaim.h"
#include "../cnf/ResRule.h"

#include "VeriPbSolverTypes.h"
#include "MaxSATProoflogger.h"

namespace converter {
    class ResClaimTypeB final : public ResClaim {
        
        public:

            ResClaimTypeB(
                const cnf::ResRule &rule, 
                const std::vector<std::pair<VeriPB::Lit, cnf::Clause>> &clauses, 
                const std::function<VeriPB::Lit(int32_t)> &variable_supplier, 
                const std::function<bool(VeriPB::Lit)> &tautology_predicate,
                const std::function<VeriPB::constraintid(VeriPB::Lit)> &tautology_supplier,
                const std::function<bool(VeriPB::Lit)> &hard_clause_predicate,
                bool negated_pivot
            ) : ResClaim(rule, clauses, variable_supplier, tautology_predicate, tautology_supplier, hard_clause_predicate, negated_pivot) {}

            constraintid write(Prooflogger &pl) override;

        private:
            constraintid add_all_and_saturate(Prooflogger &pl, const std::vector<Lit> &blocking_literals);
    };
}

#endif // CLAIM_TYPE_2_H