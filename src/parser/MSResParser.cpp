#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <stdexcept>

#include "MSResParser.h"

#include "../cnf/Rule.h"
#include "../cnf/ResRule.h"
#include "../cnf/SplitRule.h"
#include "../cnf/Clause.h"

namespace parser
{
    // TODO: Hard clauses
    cnf::Clause parseClause(const std::string& line) {
        std::istringstream iss(line);
        int weight;
        iss >> weight;

        std::set<int32_t> literals;
        int literal;
        while (iss >> literal && literal != 0) {
            literals.insert(literal);
        }

        return cnf::Clause(weight, literals);
    }

    // TODO: Error handling for invalid line types
    // TODO: o and v lines
    std::vector<cnf::Rule*> MSResParser::parseMSRes(const std::string& filename) 
    {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + filename);
        }

        std::vector<cnf::Rule *> rules;
        std::string line;
        
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == 'c') {
                continue; // Skip comments
            }

            if (line[0] == 't') {
                std::string type;
                std::istringstream iss(line);
                iss >> type;

                
                if (type == "mres") {
                    std::string rest = iss.str().substr(iss.tellg());
                    std::regex pattern("\\s*<\\s*(.*?)\\s*\\|\\s*(.*?)\\s*>\\s*");

                    std::string clause_1_str;
                    std::string clause_2_str;

                    std::smatch matches;
                    if (std::regex_match(rest, matches, pattern))
                    {
                        clause_1_str = matches[1].str();
                        clause_2_str = matches[2].str();
                    }
                    else
                    {
                        std::cout << "Invalid format\n";
                    }

                    cnf::Clause clause_1 = parseClause(clause_1_str);
                    cnf::Clause clause_2 = parseClause(clause_2_str);

                    cnf::ResRule *rule = new cnf::ResRule(clause_1, clause_2);
                    rules.push_back(rule);
                }

                if (type == "split") {
                    std::string rest = iss.str().substr(iss.tellg());
                    std::regex pattern("\\s*<\\s*(.*?)\\s*>\\s*");

                    std::string clause_str;

                    std::smatch matches;
                    if (std::regex_match(rest, matches, pattern))
                    {
                        clause_str = matches[1].str();
                    }
                    else
                    {
                        std::cout << "Invalid format\n";
                    }

                    cnf::Clause clause = parseClause(clause_str);

                    cnf::SplitRule *rule = new cnf::SplitRule(clause);
                    rules.push_back(rule);
                }
            }
        }

        file.close();
        return rules;
    }
}