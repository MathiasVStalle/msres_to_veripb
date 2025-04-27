#include <vector>
#include <cstdint>
#include <iostream>
#include <stdexcept>

#include "SplitRule.h"
#include "Clause.h"

namespace cnf
{
    SplitRule::SplitRule(const uint32_t pivot, const Clause& clause) : Rule(pivot), clause(clause), constraint_id(0) {}

    SplitRule::SplitRule(const uint32_t pivot, const Clause& clause, const uint32_t constraint_id) 
        : Rule(pivot), clause(clause), constraint_id(constraint_id) {}

    SplitRule::~SplitRule() {}

    std::vector<Clause> SplitRule::apply() const {
        std::unordered_set<int32_t> literals_1 = clause.getLiterals();
        std::unordered_set<int32_t> literals_2 = clause.getLiterals();

        literals_1.insert(get_pivot());
        literals_2.insert(-get_pivot());

        return {Clause(clause.getWeight(), literals_1), Clause(clause.getWeight(), literals_2)};
    }

    const Clause& SplitRule::getClause() const {
        return clause;
    }

    void SplitRule::print() const {
        std::cout << "SplitRule: ";
        std::cout << "Pivot: " << get_pivot() << ", ";
        clause.print();
        clause.print();
    }
    
    const Clause &SplitRule::operator[](const std::size_t index) const {
        if (index != 0) {
            throw std::out_of_range("Index out of range");
        }
        return clause;
    }
}