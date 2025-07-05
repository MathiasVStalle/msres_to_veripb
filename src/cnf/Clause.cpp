#include <cstdint>
#include <unordered_set>
#include <iostream>

#include "Clause.h"

namespace cnf
{
    Clause::Clause(const std::unordered_set<int32_t>& literals) : weight(0), literals(literals) {}

    Clause::Clause(int32_t weight, const std::unordered_set<int32_t>& literals) : weight(weight), literals(literals) {}
   
    Clause::~Clause() {}

    Clause::Clause(const Clause& other) : weight(other.weight), literals(other.literals) {}

    int32_t Clause::get_weight() const {
        return weight;
    }

    const std::unordered_set<int32_t>& Clause::get_literals() const {
        return literals;
    }

    bool Clause::is_unit_clause() const {
        return literals.size() == 1;
    }

    void Clause::print() const {
        std::cout << "Clause weight: " << weight << ", literals: ";
        for (const auto& lit : literals) {
            std::cout << lit << " ";
        }
        std::cout << std::endl;
    }

    bool Clause::operator==(const Clause& other) const {
        return weight == other.weight && literals == other.literals;
    }   
}