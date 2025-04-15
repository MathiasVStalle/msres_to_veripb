#include <set>
#include <cstdint>
#include <iostream>
#include <stdexcept>

#include "SplitRule.h"
#include "Clause.h"

namespace cnf
{
    SplitRule::SplitRule(const Clause& clause) : clause(clause) {}

    SplitRule::~SplitRule() {}

    std::set<Clause> SplitRule::apply() const {
        throw std::runtime_error("SplitRule is not yet defined.");
    }

    const Clause& SplitRule::getClause() const {
        return clause;
    }

    void SplitRule::print() const {
        std::cout << "SplitRule: ";
        clause.print();
    }
}