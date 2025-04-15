#ifndef RULE_H
#define RULE_H

#include <set>
#include "Clause.h"

namespace cnf
{
    class Rule
    {
        public:
            virtual ~Rule() = default;
            virtual std::set<Clause> apply() const = 0;
            virtual void print() const = 0;
    };
}

#endif