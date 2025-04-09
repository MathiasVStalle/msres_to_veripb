#include <set>
#include <iostream>
#include <cstdint>
#include <stdexcept>
#include <algorithm>

#include "ResRule.h"
#include "../../cnf/Clause.h"

namespace parser
{
    namespace msres
    {
        ResRule::ResRule(const cnf::Clause& clause_1, const cnf::Clause& clause_2) 
            : clause_1(clause_1), clause_2(clause_2) {}

        ResRule::~ResRule() {}

        // TODO: What to do when there are more than two common literals?
        // TODO: Weighted clauses.
        // TODO: Should this return a pointer?
        std::set<cnf::Clause> ResRule::apply() const {
            // find the two literals that are the same in both but with different signs
            std::set<int32_t> literals_1 = clause_1.getLiterals();
            std::set<int32_t> literals_2 = clause_2.getLiterals();

            int32_t common_literal = 0;
            for (const auto& lit : literals_1) {
                if (literals_2.find(-lit) != literals_2.end()) {
                    common_literal = lit;
                    break;
                }
            }

            // If no common literal is found, throw an error
            if (common_literal == 0) {
                throw std::runtime_error("No common literal found between the two clauses.");
            }

            // Remove the common literal from both clauses
            std::set<int32_t> mod_literals_1 = literals_1;
            std::set<int32_t> mod_literals_2 = literals_2;
            mod_literals_1.erase(common_literal);
            mod_literals_2.erase(-common_literal);
            

            std::set<cnf::Clause> new_clauses;

            // Build the first new clause
            std::set<int32_t> new_literals = mod_literals_1;
            new_literals.insert(mod_literals_2.begin(), mod_literals_2.end());
            cnf::Clause new_clause(std::min(clause_1.getWeight(), clause_2.getWeight()), new_literals);
            new_clauses.insert(new_clause);

            // Build the first half of the resolution clauses
            std::set<int32_t> extention;
            for (const auto& lit : mod_literals_2) {
                new_literals = literals_1;
                new_literals.insert(extention.begin(), extention.end());
                new_literals.insert(-lit);

                int weight = std::min(clause_1.getWeight(), clause_2.getWeight());

                cnf::Clause new_clause(weight, new_literals);
                new_clauses.insert(new_clause);

                extention.insert(lit);
            }

            // Build the second half of the resolution clauses
            extention.clear();
            for (const auto& lit : mod_literals_1) {
                new_literals = literals_2;
                new_literals.insert(extention.begin(), extention.end());
                new_literals.insert(-lit);

                int weight = std::min(clause_1.getWeight(), clause_2.getWeight());

                cnf::Clause new_clause(weight, new_literals);
                new_clauses.insert(new_clause);

                extention.insert(lit);
            }

            return new_clauses;
        }

        const cnf::Clause& ResRule::getClause1() const {
            return clause_1;
        }

        const cnf::Clause& ResRule::getClause2() const {
            return clause_2;
        }

        void ResRule::print() const {
            std::cout << "ResRule: " << std::endl;
            std::cout << "Clause 1: ";
            clause_1.print();
            std::cout << "Clause 2: ";
            clause_2.print();
        }
    }
}

