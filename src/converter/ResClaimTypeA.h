#ifndef RES_CLAIM_TYPE_A_H
#define RES_CLAIM_TYPE_A_H

#include <vector>

#include "ResClaim.h"
#include "../cnf/ResRule.h"

#include "VeriPbSolverTypes.h"
#include "MaxSATProoflogger.h"

using namespace VeriPB;

namespace converter {
    class ResClaimTypeA final : public ResClaim {
        private:
            std::vector<constraintid> active_constraints; // TODO: Remove

        public:
            ResClaimTypeA(
                const cnf::ResRule &rule, 
                const std::vector<std::pair<VeriPB::Lit, cnf::Clause>> &clauses, 
                const std::function<VeriPB::Lit(int32_t)> &variable_supplier, 
                const std::function<bool(VeriPB::Lit)> &tautology_predicate,
                const std::function<bool(VeriPB::Lit)> &hard_clause_predicate,
                bool negated_pivot
            ) : ResClaim(rule, clauses, variable_supplier, tautology_predicate, hard_clause_predicate, negated_pivot) {}

            constraintid write(Prooflogger &pl) override;

        private:
        
            // TODO: These functions should return a pointer to a dynamically allocated vector when working with large proofs
            std::vector<constraintid> build_iterative_subclaims(Prooflogger &pl);
            constraintid iterative_proofs_by_contradiction(Prooflogger &pl, std::vector<constraintid> &subclaims);
            std::vector<VeriPB::constraintid> build_conjunctive_subclaims(Prooflogger &pl);
    };
}

#endif // CLAIM_TYPE_1_H