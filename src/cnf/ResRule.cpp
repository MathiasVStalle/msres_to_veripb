#include <vector>
#include <iostream>
#include <cstdint>
#include <stdexcept>
#include <algorithm>

#include "ResRule.h"
#include "Clause.h"

namespace cnf
{
    int32_t min (int32_t a, int32_t b) {
        if (a == 0) return b;
        if (b == 0) return a;
        return (a < b) ? a : b;
    }

    ResRule::ResRule(const uint32_t pivot, const Clause& clause_1, const Clause& clause_2) 
        : Rule(pivot), clause_1(clause_1), clause_2(clause_2), constraint_id_1(0), constraint_id_2(0) {}

    ResRule::ResRule(const uint32_t pivot, const Clause& clause_1, const Clause& clause_2, const uint32_t constraint_id_1, const uint32_t constraint_id_2) 
        : Rule(pivot), clause_1(clause_1), clause_2(clause_2), constraint_id_1(constraint_id_1), constraint_id_2(constraint_id_2) {}

    ResRule::~ResRule() {}

    // TODO: Weighted clauses.
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
        if (!clause_1.is_hard_clause() && !clause_2.is_hard_clause()) {
            new_clauses.reserve(1 + mod_literals_1.size() + mod_literals_2.size());
        }

        // Build the first new clause (normal resolution)
        std::vector<int32_t> new_literals = mod_literals_1;
        new_literals.insert(new_literals.end(), mod_literals_2.begin(), mod_literals_2.end());
        Clause new_clause(min(clause_1.get_weight(), clause_2.get_weight()), new_literals);
        new_clauses.push_back(new_clause);

        if (clause_1.is_hard_clause() && clause_2.is_hard_clause()) {
            return new_clauses;
        }

        // Build the first half of the resolution clauses
        std::vector<int32_t> extention;
        for (const auto& lit : mod_literals_2) {
            new_literals = { common_literal };
            new_literals.insert(new_literals.end(), mod_literals_1.begin(), mod_literals_1.end());
            new_literals.insert(new_literals.end(), extention.begin(), extention.end());
            new_literals.insert(new_literals.end(), -lit);

            int weight = min(clause_1.get_weight(), clause_2.get_weight());

            Clause new_clause(weight, new_literals);
            new_clauses.push_back(new_clause);

            extention.push_back(lit);
        }

        // Build the second half of the resolution clauses
        extention.clear();
        for (const auto& lit : mod_literals_1) {
            new_literals = { -common_literal };
            new_literals.insert(new_literals.end(), mod_literals_2.begin(), mod_literals_2.end());
            new_literals.insert(new_literals.end(), extention.begin(), extention.end());
            new_literals.insert(new_literals.end(), -lit);

            int weight = min(clause_1.get_weight(), clause_2.get_weight());

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

