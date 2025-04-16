#ifndef WCNFPARSER_H
#define WCNFPARSER_H

#include <vector>
#include <string>

#include "../cnf/Clause.h"

namespace parser
{
    class WCNFParser
    {
        public:
            /**
             * Parses a WCNF file and returns a vector of clauses.
             * 
             * @param filename The name of the WCNF file to parse.
             * @return A vector of CNF::Clause objects representing the clauses in the WCNF file.
             */
            static std::vector<cnf::Clause> parseWCNF(const std::string& filename);    
            
            WCNFParser() = delete; // Prevent instantiation of this class
    };
}

#endif