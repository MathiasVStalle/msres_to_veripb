#include <vector>
#include <cstdint>
#include <iostream>
#include <stdexcept>

#include "SplitRule.h"
#include "Clause.h"

namespace cnf
{
    SplitRule::SplitRule(const Clause& clause) : clause(clause), constraint_id(0) {}

    SplitRule::SplitRule(const Clause& clause, const uint32_t constraint_id) 
        : clause(clause), constraint_id(constraint_id) {}

    SplitRule::~SplitRule() {}

    std::vector<Clause> SplitRule::apply() const {
        throw std::runtime_error("SplitRule is not yet defined.");
    }

    const Clause& SplitRule::getClause() const {
        return clause;
    }

    void SplitRule::print() const {
        std::cout << "SplitRule: ";
        clause.print();
    }
    
    const Clause &SplitRule::operator[](const std::size_t index) const {
        if (index != 0) {
            throw std::out_of_range("Index out of range");
        }
        return clause;
    }
}