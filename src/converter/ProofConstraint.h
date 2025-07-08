#ifndef PROOF_CONSTRAINT_H
#define PROOF_CONSTRAINT_H

#include <span>
#include <unordered_set>

#include "Hash.h"

#include "VeriPbSolverTypes.h"
#include "MaxSATProoflogger.h"

namespace converter {
    /**
     * @brief Represents a proof constraint in the converter.
     */
    class ProofConstraint {
        private:
            VeriPB::Var blocking_var;
            bool tautology = false;
            std::unordered_set<VeriPB::Lit, LitHash, LitEqual> duplicate_literals;

            std::span<const VeriPB::Var> active_vars;
            std::span<const VeriPB::Var> inactive_vars;

        public:
            /**
             * @brief Default constructor for ProofConstraint.
             */
            ProofConstraint(
                const VeriPB::Var &blocking_variable, 
                const bool tautology,
                const std::unordered_set<VeriPB::Lit, LitHash, LitEqual> &duplicate_literals,
                const std::span<const VeriPB::Var> &active_vars,
                const std::span<const VeriPB::Var> &inactive_vars
            );
            
            /**
             * @brief Get the blocking variable of the constraint.
             * 
             * @return The blocking variable of the constraint.
             */
            VeriPB::Var get_blocking_variable();

            /**
             * @brief Check if the constraint is a tautology.
             * 
             * @return True if the constraint is a tautology, false otherwise.
             */
            bool is_tautology();

            /**
             * @brief Get the set of duplicate literals in the constraint.
             * 
             * @return The unordered_set of duplicate literals in the constraint.
             */
            const std::unordered_set<VeriPB::Lit, LitHash, LitEqual>& get_duplicate_literals() const;

            /**
             * @brief Get the active variables of the constraint.
             * 
             * @return A span of active variables in the constraint.
             */
            std::span<const VeriPB::Var> get_active_vars();

            /**
             * @brief Get the inactive variables of the constraint.
             * 
             * @return A span of inactive variables in the constraint.
             */
            std::span<const VeriPB::Var> get_inactive_vars();      
    };

} // namespace converter


#endif // PROOF_CONSTRAINT_H