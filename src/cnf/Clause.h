#ifndef CNF_H
#define CNF_H

#include <cstdint>
#include <set>

namespace cnf
{
    class Clause {
        private:
            int32_t weight;
            std::set<int32_t> literals;

        public:
            /**
             * Constructor for a hard clause.
             * The weight will be set to 0 by default.
             * 
             * @param literals The set of literals in the clause.
             */
            Clause(const std::set<int32_t>& literals);

            /**
             * Constructor for a soft clause.
             * 
             * @param weight The weight of the clause.
             * @param literals The set of literals in the clause.
             */
            Clause(int32_t weight, const std::set<int32_t>& literals);

            /**
             * Destructor for the Clause class.
             */
            ~Clause();

            /**
             * Copy constructor for the Clause class.
             * 
             * @param other The clause to copy from.
             */
            Clause(const Clause& other);

            /**
             * Get the weight of the clause.
             * 
             * @return The weight of the clause.
             */
            int32_t getWeight() const;

            /**
             * Get the set of literals in the clause.
             * 
             * @return The set of literals in the clause.
             */
            const std::set<int32_t>& getLiterals() const;

            /**
             * Print the clause to the standard output.
             */
            void print() const;

            /**
             * Check if two clauses are equal.
             * 
             * @param other The clause to compare with.
             * @return True if the clauses are equal, false otherwise.
             */
            bool operator==(const Clause& other);

            // TODO: Requered for the set, might be unnecessary when using unordered_set
            /**
             * Check if the current clause is less than another clause based on weight.
             * 
             * @param other The clause to compare with.
             * @return True if the current clause is less than the other clause, false otherwise.
             */
            bool operator<(const Clause& other) const {
                return weight < other.weight;
            }
    };
}



#endif

