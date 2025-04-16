#include <vector>
#include <set>
#include <cstdint>
#include <string>

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "WCNFParser.h"
#include "../cnf/Clause.h"

namespace parser 
{
    // TODO: Add functionality to handle hard clauses
    // TODO: Error handling for invalid line types
    std::vector<cnf::Clause> WCNFParser::parseWCNF(const std::string& filename) 
    {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + filename);
        }

        std::vector<cnf::Clause> clauses;
        std::string line;

        while (std::getline(file, line)) {
            if (line.empty() || line[0] == 'c') {
                continue; // Skip comments
            }
            if (line[0] == 'p') {
                continue; // Skip problem line
            }

            // Parse the clause
            std::istringstream iss(line);
            int weight;
            iss >> weight;

            std::unordered_set<int32_t> literals;
            int literal;
            while (iss >> literal && literal != 0) {
                literals.insert(literal);
            }

            clauses.emplace_back(weight, literals);
        }

        file.close();
        return clauses;
    }
}