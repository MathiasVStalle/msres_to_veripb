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

        // find the two literals that are the same in both but with different signs
        std::vector<int32_t> literals_1 = clause_1.get_literals();
        std::vector<int32_t> literals_2 = clause_2.get_literals();
        int32_t common_literal = this->get_pivot();

        // Remove the common literal from both clauses
        std::vector<int32_t> mod_literals_1 = literals_1;
        std::vector<int32_t> mod_literals_2 = literals_2;

        auto it = std::find(mod_literals_1.begin(), mod_literals_1.end(), common_literal);
        if (it != mod_literals_1.end()) {
            mod_literals_1.erase(it);  // Only erases the *first* match
        }

        it = std::find(mod_literals_2.begin(), mod_literals_2.end(), -common_literal);
        if (it != mod_literals_2.end()) {
            mod_literals_2.erase(it);  // Only erases the *first* match
        }

        std::vector<Clause> new_clauses;

        // Build the first new clause
        std::vector<int32_t> new_literals = mod_literals_1;
        new_literals.insert(new_literals.end(), mod_literals_2.begin(), mod_literals_2.end());
        Clause new_clause(std::min(clause_1.get_weight(), clause_2.get_weight()), new_literals);
        new_clauses.push_back(new_clause);

        // Build the first half of the resolution clauses
        std::vector<int32_t> extention;
        for (const auto& lit : mod_literals_2) {
            new_literals = literals_1;
            new_literals.insert(new_literals.end(), extention.begin(), extention.end());
            new_literals.insert(new_literals.end(), -lit);

            int weight = std::min(clause_1.get_weight(), clause_2.get_weight());

            Clause new_clause(weight, new_literals);
            new_clauses.push_back(new_clause);

            extention.push_back(lit);
        }

        // Build the second half of the resolution clauses
        extention.clear();
        for (const auto& lit : mod_literals_1) {
            new_literals = literals_2;
            new_literals.insert(new_literals.end(), extention.begin(), extention.end());
            new_literals.insert(new_literals.end(), -lit);

            int weight = std::min(clause_1.get_weight(), clause_2.get_weight());

            Clause new_clause(weight, new_literals);
            new_clauses.push_back(new_clause);

            extention.push_back(lit);
        }

        return new_clauses;
    }

    const Clause& ResRule::get_clause_1() const {
        return clause_1;
    }

    const Clause& ResRule::get_clause_2() const {
        return clause_2;
    }

    void ResRule::print() const {
        std::cout << "ResRule: " << get_pivot() << std::endl;
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

