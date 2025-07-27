#ifndef RES_CLAIM_H
#define RES_CLAIM_H

#include <vector>
#include <tuple>
#include <unordered_set>
#include <functional>
#include <memory>

#include "Claim.h"
#include "Hash.h"
#include "../cnf/ResRule.h"

#include "VeriPbSolverTypes.h"
#include "MaxSATProoflogger.h"

using namespace VeriPB;

namespace converter {
    class ResClaim : public Claim {
        private:

            // TODO: Name vars --> lit or change type Lit --> Var
            Lit pivot_literal;

            std::vector<Lit> vars;                          // TODO: Remove and use literals if possible
            std::vector<Lit> literals;

            Lit active_original_blocking_var;
            Lit inactive_original_blocking_var;
            std::vector<Lit> active_blocking_vars;
            std::vector<Lit> inactive_blocking_vars;

        public:
            ResClaim(
                const cnf::ResRule &rule,
                const std::vector<std::pair<VeriPB::Lit, cnf::Clause>> &clauses,
                const std::function<VeriPB::Lit(int32_t)> &variable_supplier,
                const std::function<bool(VeriPB::Lit)> &tautology_predicate,
                const std::function<bool(VeriPB::Lit)> &hard_clause_predicate,
                bool negated_pivot);

        protected:

            const std::vector<Lit>& get_vars() const;
            const std::vector<Lit>& get_literals() const;
            const Lit& get_pivot_literal() const;
            const Lit& get_active_original_blocking_var() const;
            const Lit& get_inactive_original_blocking_var() const;
            const std::vector<Lit>& get_active_blocking_vars() const;
            const std::vector<Lit>& get_inactive_blocking_vars() const;

            // TODO: variables should not be given directly
            constraintid weaken(Prooflogger &pl, constraintid id, const std::vector<Lit> &variables, uint32_t begin, uint32_t end);
            constraintid weaken_all_except(Prooflogger &pl, constraintid id, const std::vector<Lit> &variables, uint32_t except);
            constraintid weaken_all_except(Prooflogger &pl, constraintid id, const std::vector<Lit> &variables, uint32_t begin, uint32_t end);

            constraintid add_all(Prooflogger &pl, const std::vector<constraintid> &constraints);
            constraintid add_all_from_literal(Prooflogger &pl, const std::vector<constraintid> &constraints, const Lit lit);
            constraintid add_all_prev(Prooflogger &pl, uint32_t range);
            constraintid add_all_prev_from_literal(Prooflogger &pl, uint32_t range, const Lit lit);

            constraintid build_proof_by_contradiction(Prooflogger &pl, Constraint<Lit, uint32_t, uint32_t> &C, constraintid claim_1, constraintid claim_2);
            constraintid build_proof_by_contradiction(Prooflogger &pl, Constraint<Lit, uint32_t, uint32_t> &C, std::vector<VeriPB::constraintid> &claims);

        private:
            
            void initialize_vars(int32_t pivot, const std::vector<int32_t>& literals_1, const std::vector<int32_t>& literals_2, std::function<VeriPB::Lit(int32_t)> variable_supplier);
    };
}

#endif // CLAIM_H