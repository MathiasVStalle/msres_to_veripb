#ifndef CLAIM_H
#define CLAIM_H

#include "VeriPbSolverTypes.h"
#include "MaxSATProoflogger.h"

namespace converter {
    class Claim {
        private:
            const bool negated_pivot;

        public:
            Claim (bool negated_pivot) : negated_pivot(negated_pivot) {}

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
    };
}

#endif // CLAIM_H