#include <iostream>

#include "./cnf/Clause.h"
#include "./cnf/ResRule.h"
#include "./cnf/SplitRule.h"
#include "./cnf/Rule.h"
#include "./parser/MSResParser.h"
#include "./parser/WCNFParser.h"
#include "./convertor/ProofConvertor.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <wcnf_file> <msres_file> <output_file>" << std::endl;
        return 1;
    }

    // Check for .wcnf extension
    if (std::string(argv[1]).find(".wcnf") == std::string::npos) {
        std::cerr << "Error: The first argument must be a .wcnf file." << std::endl;
        return 1;
    }

    std::vector<cnf::Clause> wcnf_clauses = parser::WCNFParser::parseWCNF(argv[1]);
    for (const auto& clause : wcnf_clauses) {
        std::cout << "WCNF Clause: ";
        clause.print();
    }

    std::string wcnf_file = argv[1];
    std::string msres_file = argv[2];
    std::string output_file = argv[3];
    
    convertor::ProofConvertor proof_convertor(wcnf_file, msres_file, output_file);
    proof_convertor.write_proof();

    return 0;
}