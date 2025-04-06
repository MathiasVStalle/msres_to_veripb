#include <set>
#include <cstdint>
#include <iostream>
#include <stdexcept>

#include "SplitRule.h"
#include "../../cnf/Clause.h"

namespace parser
{
    namespace msres
    {
        SplitRule::SplitRule(const cnf::Clause& clause) : clause(clause) {}

        SplitRule::~SplitRule() {}

        std::set<cnf::Clause> SplitRule::apply() const {
            throw std::runtime_error("SplitRule is not yet defined.");
        }

        const cnf::Clause& SplitRule::getClause() const {
            return clause;
        }

        void SplitRule::print() const {
            std::cout << "SplitRule: ";
            clause.print();
        }
    }
}