#ifndef MSRESPARSER_H
#define MSRESPARSER_H

#include <vector>
#include <string>
#include <fstream>

#include "../cnf/Rule.h"

namespace parser
{
    class MSResParser
    {
        private:
            std::string filename;
            u_int32_t line_number; // Starting from 0
            u_int32_t rule_number; // Starting from 0
            std::ifstream *file_stream;

            /**
             * Parses a clause from a string.
             *
             * @param line The string containing the clause.
             * @return A Clause object representing the parsed clause.
             */
            cnf::Clause parseClause(const std::string &line);

        public:
            /**
             * Constructor for MSResParser.
             *
             * @param filename The name of the MSRes file to parse.
             */
            MSResParser(const std::string filename);

            /**
             * Destructor for MSResParser.
             */
            ~MSResParser();

            /**
             * returns the next rule in the file.
             *
             * @return A pointer to the next rule in the file.
             */
            cnf::Rule *next_rule();
    };
}

#endif
