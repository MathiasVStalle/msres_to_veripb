#ifndef RULE_H
#define RULE_H

#include <vector>
#include "Clause.h"

namespace cnf
{
    class Rule
    {
        private:
            int32_t pivot;

        public:
            Rule(uint32_t pivot) : pivot(pivot) {}

            virtual ~Rule() = default;
            virtual std::vector<Clause> apply() const = 0;
            virtual void print() const = 0;

            virtual const Clause& operator[](const std::size_t index) const = 0;

            const int32_t get_pivot() const {
                return pivot;
            }

            void set_pivot(int32_t new_pivot) {
                pivot = new_pivot;
            }
    };
}

#endif