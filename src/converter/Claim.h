#ifndef CLAIM_H
#define CLAIM_H

#include "VeriPbSolverTypes.h"
#include "MaxSATProoflogger.h"

namespace converter {
    class Claim {
        private:
            const bool negated_pivot;

            const std::function<bool(VeriPB::Lit)> &tautology_predicate;
            const std::function<bool(VeriPB::Lit)> &hard_clause_predicate;

            const std::function<VeriPB::constraintid(VeriPB::Lit)> &tautology_supplier; // TODO: Remove if possible

        public:
            Claim (
                bool negated_pivot, 
                   const std::function<bool(VeriPB::Lit)> &tautology_predicate,
                   const std::function<bool(VeriPB::Lit)> &hard_clause_predicate,
                   const std::function<VeriPB::constraintid(VeriPB::Lit)> &tautology_supplier
                ) : negated_pivot(negated_pivot), tautology_predicate(tautology_predicate), hard_clause_predicate(hard_clause_predicate), tautology_supplier(tautology_supplier) {}

            virtual VeriPB::constraintid write(VeriPB::Prooflogger &pl) = 0;

        protected:
            virtual ~Claim() = default;

            /**
             * Check if the pivot literal is negated.
             *
             * @return True if the pivot literal is negated, false otherwise.
             */
            bool is_negated_pivot() const {
                return negated_pivot;
            }

            /**
             * Check if the clause of the given blocking literal is a tautology.
             * 
             * @param lit The blocking literal to check.
             * @return True if the clause is a tautology, false otherwise.
             */
            bool is_tautology(const VeriPB::Lit &lit) const {
                return tautology_predicate(lit);
            }

            /**
             * Check if the clauses of the given blocking literal are hard clauses.
             *
             * @param lit The blocking literal to check.
             * @return True if the clause is a hard clause, false otherwise.
             */
            bool is_hard_clause(const VeriPB::Lit &lit) const {
                return hard_clause_predicate(lit);
            }

            /**
             * Get the constraint ID for the tautology of the given blocking literal.
             *
             * @param lit The blocking literal to get the tautology constraint ID for.
             * @return The constraint ID for the tautology.
             */
            VeriPB::constraintid get_tautology(const VeriPB::Lit &lit) const {
                return tautology_supplier(lit);
            }
    };
}

#endif // CLAIM_H