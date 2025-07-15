#include <cstdint>
#include <unordered_set>
#include <iostream>

#include "Clause.h"

namespace cnf
{
    Clause::Clause(const std::vector<int32_t>& literals) : Clause(0, literals) {}

    Clause::Clause(int32_t weight, const std::vector<int32_t>& literals) : weight(weight), literals(literals) {
        for (const auto& lit : literals) {
            if (std::find(literals.begin(), literals.end(), -lit) != literals.end()) {
                tautology = true;
            }
            if (std::find(literals.begin(), literals.end(), lit) != literals.end()) {
                has_double = true;
                duplicate_literals.insert(lit);
            }
        }
    }
   
    Clause::~Clause() {}

    Clause::Clause(const Clause& other) : 
    weight(other.weight), literals(other.literals), duplicate_literals(other.duplicate_literals), tautology(other.tautology), has_double(other.has_double) {}

    int32_t Clause::get_weight() const {
        return weight;
    }

    const std::vector<int32_t>& Clause::get_literals() const {
        return literals;
    }

    const std::unordered_set<int32_t>& Clause::get_duplicate_literals() const {
        return duplicate_literals;
    }

    bool Clause::is_unit_clause() const {
        return literals.size() == 1;
    }

    bool Clause::is_tautology() const {
        return tautology;
    }

    bool Clause::is_hard_clause() const {
        return weight == 0;
    }

    void Clause::print() const {
        std::cout << "Clause weight: " << weight << ", literals: ";
        for (const auto& lit : literals) {
            std::cout << lit << " ";
        }
        std::cout << std::endl;
    }

    // TODO: Every time a clause is compaired, it makes a new unordered_set. Make this more efficient.
    bool Clause::operator==(const Clause& other) const {
        return weight == other.weight && get_literals_set() == other.get_literals_set();
    }   
}