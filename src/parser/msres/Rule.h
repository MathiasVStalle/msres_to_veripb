#ifndef RULE_H
#define RULE_H

#include <set>
#include "../../cnf/Clause.h"

namespace parser
{
    namespace msres
    {
        class Rule
        {
            public:
                virtual ~Rule() = default;
                virtual std::set<cnf::Clause> apply() const = 0;
                virtual void print() const = 0;
        };
    }
}

#endif