#include <iostream>

#include "./cnf/Clause.h"
#include "./cnf/ResRule.h"
#include "./cnf/SplitRule.h"
#include "./cnf/Rule.h"
#include "./parser/MSResParser.h"
#include "./parser/WCNFParser.h"
#include "./converter/ProofConverter.h"

int check_proofs_are_same(const std::string& output_file, const std::string& test_file) {
    std::ifstream output_stream(output_file);
    std::ifstream test_stream(test_file);

    if (!output_stream.is_open() || !test_stream.is_open()) {
        std::cerr << "Error opening files for comparison." << std::endl;
        return 1;
    }

    std::string output_line, test_line;
    while (std::getline(output_stream, output_line) && std::getline(test_stream, test_line)) {
        if (output_line != test_line) {
            std::cerr << "Proofs differ at line: " << output_line << " vs " << test_line << std::endl;
            return 1;
        }
    }

    std::cout << "Proofs are the same." << std::endl;
    return 0;
}

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

    std::string wcnf_file = argv[1];
    std::string msres_file = argv[2];
    std::string output_file = argv[3];
    
    std::cout << "Stop" << std::endl;
    
    converter::ProofConverter proof_convertor(wcnf_file, msres_file, output_file);
    proof_convertor.write_proof();

    check_proofs_are_same(output_file, "example/unit_clause/test.pbp");
    return 0;
}