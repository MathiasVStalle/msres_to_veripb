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

using namespace parser;

namespace parser
{
    MSResParser::MSResParser(const std::string filename) 
        : filename(filename), line_number(0), rule_number(0) {
        this->file_stream = new std::ifstream(filename);
        if (!this->file_stream->is_open())
        {
            throw std::runtime_error("Could not open file: " + filename);
        }
    }

    MSResParser::~MSResParser() {
        if (this->file_stream != nullptr)
        {
            this->file_stream->close();
            delete this->file_stream;
            this->file_stream = nullptr;
        }
    }

    // TODO: Hard clauses
    cnf::Clause MSResParser::parseClause(const std::string &line)
    {
        std::istringstream iss(line);
        double weight;
        iss >> weight;

        std::unordered_set<int32_t> literals;
        int literal;
        while (iss >> literal && literal != 0)
        {
            literals.insert(literal);
        }

        return cnf::Clause(weight, literals);
    }

    // TODO: Be able to parse when a number is given as a float
    cnf::Rule* MSResParser::next_rule()
    {
        std::string line;
        if (!std::getline(*this->file_stream, line)) {
            return nullptr; // End of file
        }
        this->line_number++;

        if (line[0] == 'o', line[0] == 'v') {
            std::cerr << "Parsing finished at line: " << this->line_number << std::endl;
            return nullptr; // End of parsing
        }
        if (line.empty() || line[0] == 'c') {
            return next_rule(); // Skip comments
        }
        
        if (line[0] == 't')
        {
            std::string dummy, type;
            std::istringstream iss(line);
            iss >> dummy >> type;

            if (type == "msres")
            {
                std::string rest = iss.str().substr(iss.tellg());

                // The regex matches the format "<clause_1> | <clause_2>"
                // std::regex pattern("\\s*<\\s*(.*?)\\s*\\|\\s*(.*?)\\s*>\\s*");

                // Regex matches the pattern "< clause_1 | pivot | clause_2 >"
                std::regex pattern("\\s*<\\s*(.*?)\\s*\\|\\s*(.*?)\\s*\\|\\s*(.*?)\\s*>\\s*");

                std::string clause_1_str;
                std::string clause_2_str;
                std::string pivot_str;

                std::smatch matches;
                if (std::regex_match(rest, matches, pattern)) {
                    clause_1_str    = matches[1].str();
                    clause_2_str    = matches[3].str();
                    pivot_str       = matches[2].str();

                    cnf::Clause clause_1 = parseClause(clause_1_str);
                    cnf::Clause clause_2 = parseClause(clause_2_str);

                    // TODO: This should be fixed int the converter. When the pivot has a different sign than the one in the first clause, it doesn't work.
                    if (
                        clause_1.getLiterals().find(std::stoi(pivot_str)) == clause_1.getLiterals().end() ||
                        clause_2.getLiterals().find(-std::stoi(pivot_str)) == clause_2.getLiterals().end()
                    ) {
                        // Switch the clauses
                        std::swap(clause_1, clause_2);
                    }

                    uint32_t pivot = std::stoi(pivot_str);

                    cnf::ResRule *rule = new cnf::ResRule(pivot, clause_1, clause_2);

                    this->rule_number++;
                    return rule;
                } else {
                    std::cerr << "Invalid msres format at line: " << this->line_number << std::endl;
                }
            }

            if (type == "split")
            {
                std::string rest = iss.str().substr(iss.tellg());
                // std::regex pattern("\\s*<\\s*(.*?)\\s*>\\s*");
                std::regex pattern("\\s*<\\s*(.*?)\\s*\\|\\s*(.*?)\\s*>\\s*");

                std::string clause_str;
                std::string pivot_str;

                std::smatch matches;
                if (std::regex_match(rest, matches, pattern)) {
                    clause_str = matches[1].str();
                    pivot_str  = matches[2].str();

                    uint32_t pivot = std::stoi(pivot_str);

                    cnf::Clause clause = parseClause(clause_str);

                    cnf::SplitRule *rule = new cnf::SplitRule(pivot, clause);

                    this->rule_number++;
                    return rule;
                }
                else
                {
                    std::cerr << "Invalid msres format at line: " << this->line_number << std::endl;
                }  
            }
        }
        
        return nullptr; // No valid rule found
    }

    // TODO: Error handling for invalid line types
    // TODO: o and v lines
}