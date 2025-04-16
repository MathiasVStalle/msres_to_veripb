#ifndef RULE_H
#define RULE_H

#include <vector>
#include "Clause.h"

namespace cnf
{
    class Rule
    {
        public:
            virtual ~Rule() = default;
            virtual std::vector<Clause> apply() const = 0;
            virtual void print() const = 0;

            virtual const Clause& operator[](const std::size_t index) const = 0;
    };
}

#endif