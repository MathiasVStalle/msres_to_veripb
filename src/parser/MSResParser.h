#ifndef MSRESPARSER_H
#define MSRESPARSER_H

#include <vector>
#include <string>

#include "../cnf/Rule.h"

namespace parser
{
    class MSResParser
    {
        public:


            /**
             * Parses a MSRes file and returns a vector of clauses.
             * 
             * @param filename The name of the MSRes file to parse.
             * @return A vector of CNF::Clause objects representing the clauses in the MSRes file.
             */
            static std::vector<cnf::Rule*> parseMSRes(const std::string& filename);
        
            
            MSResParser() = delete; // Prevent instantiation of this class
    };
}

#endif
