#ifndef CNF_H
#define CNF_H

#include <cstdint>
#include <unordered_set>

namespace cnf
{
    class Clause {
        private:
            int32_t weight;
            std::unordered_set<int32_t> literals;

        public:
            /**
             * Constructor for a hard clause.
             * The weight will be set to 0 by default.
             *
             * @param literals The unordered_set of literals in the clause.
             */
            Clause(const std::unordered_set<int32_t> &literals);

            /**
             * Constructor for a soft clause.
             *
             * @param weight The weight of the clause.
             * @param literals The unordered_set of literals in the clause.
             */
            Clause(int32_t weight, const std::unordered_set<int32_t> &literals);

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
             * @return The unordered_set of literals in the clause.
             */
            const std::unordered_set<int32_t> &getLiterals() const;

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
            bool operator==(const Clause& other) const;
    };
}


// Hash function for the Clause class.
namespace std {
    template <>
    struct hash<cnf::Clause> {
        size_t operator()(const cnf::Clause& clause) const {
            size_t hash_value = 0;
            for (const auto& literal : clause.getLiterals()) {
                hash_value ^= std::hash<int32_t>()(literal);
            }
            return hash_value;
        }
    };
}



#endif

