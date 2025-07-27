#ifndef SPLIT_CLAIM_H
#define SPLIT_CLAIM_H

#include <vector>
#include <functional>

#include "Claim.h"
#include "../cnf/SplitRule.h"

#include "VeriPbSolverTypes.h"
#include "MaxSATProoflogger.h"

namespace converter {
    class SplitClaim : public Claim {
        private:
            
            VeriPB::Lit original_blocking_lit;
            VeriPB::Lit new_blocking_lit_1;
            VeriPB::Lit new_blocking_lit_2;

        public:
            
            // TODO: Simplify the constructor
            // TODO: Negated pivot shouln't be named negated_pivot
            SplitClaim(
                const std::vector<std::pair<VeriPB::Lit, cnf::Clause>> &clauses,
                const std::function<bool(VeriPB::Lit)> &tautology_predicate,
                const std::function<bool(VeriPB::Lit)> &hard_clause_predicate,
                bool negated_pivot
            );

            VeriPB::constraintid write(VeriPB::Prooflogger &pl) override;
    };
}

#endif