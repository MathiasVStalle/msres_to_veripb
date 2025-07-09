#ifndef CLAIM_H
#define CLAIM_H

#include <vector>
#include <tuple>
#include <unordered_set>
#include <functional>
#include <memory>

#include "Hash.h"
#include "../cnf/ResRule.h"

#include "VeriPbSolverTypes.h"
#include "MaxSATProoflogger.h"

using namespace VeriPB;

namespace converter {
    class Claim {
        private:

            // TODO: Name vars --> lit or change type Lit --> Var
            const bool negated_pivot;
            std::vector<Lit> vars;
            std::vector<Lit> blocking_vars;

            Lit pivot_literal;

            Lit active_original_blocking_var;
            Lit unactive_original_blocking_var;

            std::vector<Lit> active_blocking_vars;
            std::vector<Lit> unactive_blocking_vars;

            std::vector<Lit> active_vars;
            std::vector<constraintid> active_constraints;

            std::unordered_set<Lit, LitHash, LitEqual> tautologies;
            std::unordered_map<Var, uint32_t, VarHash, VarEqual> duplicate_vars;
            std::unordered_map<Var, uint32_t, VarHash, VarEqual> possible_pivots;

        public:
            Claim(const cnf::ResRule &rule, const std::vector<std::pair<VeriPB::Lit, cnf::Clause>> &clauses, const std::function<VeriPB::Lit(int32_t)> &variable_supplier, bool negated_pivot);

            virtual constraintid write(Prooflogger &pl) = 0;

        protected:

            /** 
             * Check if the pivot literal is negated.
             * 
             * @return True if the pivot literal is negated, false otherwise.
             */
            bool is_negated_pivot() const;

            const std::vector<Lit>& get_vars() const;
            const std::vector<Lit>& get_blocking_vars() const;
            const Lit& get_pivot_literal() const;
            const Lit& get_active_original_blocking_var() const;
            const Lit& get_unactive_original_blocking_var() const;
            const std::vector<Lit>& get_active_blocking_vars() const;
            const std::vector<Lit>& get_unactive_blocking_vars() const;
            const std::vector<constraintid>& get_active_constraints() const;
            const std::vector<Lit>& get_active_vars() const;

            void set_active_constraints(const std::vector<constraintid> &active_constraints);

            // TODO: variables should not be given directly
            constraintid weaken(Prooflogger &pl, constraintid id, const std::vector<Lit> &variables, uint32_t begin, uint32_t end);
            constraintid weaken_all_except(Prooflogger &pl, constraintid id, const std::vector<Lit> &variables, uint32_t except);
            constraintid weaken_all_except(Prooflogger &pl, constraintid id, const std::vector<Lit> &variables, uint32_t begin, uint32_t end);

            constraintid add_all(Prooflogger &pl, const std::vector<constraintid> &constraints);
            constraintid add_all_and_saturate(Prooflogger &pl, const std::vector<constraintid> &constraints);
            constraintid add_all_from_literal(Prooflogger &pl, const std::vector<constraintid> &constraints, const Lit lit);
            constraintid add_all_prev(Prooflogger &pl, uint32_t range);
            constraintid add_all_prev_from_literal(Prooflogger &pl, uint32_t range, const Lit lit);

            constraintid build_proof_by_contradiction(Prooflogger &pl, Constraint<Lit, uint32_t, uint32_t> &C, constraintid claim_1, constraintid claim_2);
            constraintid build_proof_by_contradiction(Prooflogger &pl, Constraint<Lit, uint32_t, uint32_t> &C, std::vector<VeriPB::constraintid> &claims);

            bool is_duplicate(const Lit &lit) const;
            bool is_possible_pivot(const Lit &lit) const;
            bool is_tautology(const Lit &lit) const;

        private:

            void initialize_duplicate_vars(const cnf::ResRule &rule);
            
            std::vector<VeriPB::Lit> get_total_vars(const std::vector<int32_t>& literals_1, const std::vector<int32_t>& literals_2, std::function<VeriPB::Lit(int32_t)> variable_supplier);
    };
}

#endif // CLAIM_H