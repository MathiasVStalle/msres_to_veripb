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
        std::unordered_multiset<int32_t> literals_1 = clause.get_literals();
        std::unordered_multiset<int32_t> literals_2 = clause.get_literals();

        literals_1.insert(get_pivot());
        literals_2.insert(-get_pivot());

        return {Clause(clause.get_weight(), literals_1), Clause(clause.get_weight(), literals_2)};
    }

    const Clause& SplitRule::get_clause() const {
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