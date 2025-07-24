#ifndef RES_CLAIM_TYPE_C_H
#define RES_CLAIM_TYPE_C_H

#include <vector>

#include "ResClaim.h"
#include "../cnf/ResRule.h"

#include "VeriPbSolverTypes.h"
#include "MaxSATProoflogger.h"

namespace converter {
    class ResClaimTypeC final : public ResClaim {
        
        public:

            ResClaimTypeC(
                const cnf::ResRule &rule, 
                const std::vector<std::pair<VeriPB::Lit, cnf::Clause>> &clauses, 
                const std::function<VeriPB::Lit(int32_t)> &variable_supplier, 
                const std::function<bool(VeriPB::Lit)> &tautology_predicate,
                const std::function<VeriPB::constraintid(VeriPB::Lit)> &tautology_supplier,
                const std::function<bool(VeriPB::Lit)> &hard_clause_predicate,
                bool negated_pivot
            ) : ResClaim(rule, clauses, variable_supplier, tautology_predicate, tautology_supplier, hard_clause_predicate, negated_pivot) {}

            constraintid write(Prooflogger &pl) override;
    };
}

#endif // RES_CLAIM_TYPE_C_H