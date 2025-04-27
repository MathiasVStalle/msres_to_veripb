#include <set>
#include <vector>
#include <iostream>
#include <cstdint>
#include <stdexcept>
#include <algorithm>

#include "ResRule.h"
#include "Clause.h"

namespace cnf
{
    ResRule::ResRule(const uint32_t pivot, const Clause& clause_1, const Clause& clause_2) 
        : Rule(pivot), clause_1(clause_1), clause_2(clause_2), constraint_id_1(0), constraint_id_2(0) {}

    ResRule::ResRule(const uint32_t pivot, const Clause& clause_1, const Clause& clause_2, const uint32_t constraint_id_1, const uint32_t constraint_id_2) 
        : Rule(pivot), clause_1(clause_1), clause_2(clause_2), constraint_id_1(constraint_id_1), constraint_id_2(constraint_id_2) {}

    ResRule::~ResRule() {}

    // TODO: What to do when there are more than two common literals?
    // TODO: Weighted clauses.
    // TODO: Should this return a pointer?
    std::vector<Clause> ResRule::apply() const {
        std::cout << "Applying resolution rule with pivot: " << this->get_pivot() << std::endl;

        // find the two literals that are the same in both but with different signs
        std::unordered_set<int32_t> literals_1 = clause_1.getLiterals();
        std::unordered_set<int32_t> literals_2 = clause_2.getLiterals();
        int32_t common_literal = this->get_pivot();

        // Remove the common literal from both clauses
        std::unordered_set<int32_t> mod_literals_1 = literals_1;
        std::unordered_set<int32_t> mod_literals_2 = literals_2;
        mod_literals_1.erase(common_literal);
        mod_literals_2.erase(-common_literal);

        std::vector<Clause> new_clauses;

        // Build the first new clause
        std::unordered_set<int32_t> new_literals = mod_literals_1;
        new_literals.insert(mod_literals_2.begin(), mod_literals_2.end());
        Clause new_clause(std::min(clause_1.getWeight(), clause_2.getWeight()), new_literals);
        new_clauses.push_back(new_clause);

        // Build the first half of the resolution clauses
        std::set<int32_t> extention;
        for (const auto& lit : mod_literals_2) {
            new_literals = literals_1;
            new_literals.insert(extention.begin(), extention.end());
            new_literals.insert(-lit);

            int weight = std::min(clause_1.getWeight(), clause_2.getWeight());

            Clause new_clause(weight, new_literals);
            new_clauses.push_back(new_clause);

            extention.insert(lit);
        }

        // Build the second half of the resolution clauses
        extention.clear();
        for (const auto& lit : mod_literals_1) {
            new_literals = literals_2;
            new_literals.insert(extention.begin(), extention.end());
            new_literals.insert(-lit);

            int weight = std::min(clause_1.getWeight(), clause_2.getWeight());

            Clause new_clause(weight, new_literals);
            new_clauses.push_back(new_clause);

            extention.insert(lit);
        }

        return new_clauses;
    }

    const Clause& ResRule::getClause1() const {
        return clause_1;
    }

    const Clause& ResRule::getClause2() const {
        return clause_2;
    }

    void ResRule::print() const {
        std::cout << "ResRule: " << std::endl;
        std::cout << "Clause 1: ";
        clause_1.print();
        std::cout << "Clause 2: ";
        clause_2.print();
    }

    const Clause& ResRule::operator[] (const size_t index) const {
        if (index == 0) {
            return clause_1;
        } else if (index == 1) {
            return clause_2;
        } else {
            throw std::out_of_range("Index out of range");
        }
    }
}

