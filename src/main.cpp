#include <iostream>

#include "./cnf/Clause.h"
#include "./cnf/ResRule.h"
#include "./cnf/SplitRule.h"
#include "./cnf/Rule.h"
#include "./parser/MSResParser.h"
#include "./parser/WCNFParser.h"
#include "./convertor/ProofConvertor.h"

int main() {
    std::cout << "Started" << std::endl;
    
    parser::MSResParser parser("example_proof.msres");
    cnf::Rule *rule = parser.next_rule();

    rule->print();

    // Example usage of the ProofConvertor
    convertor::ProofConvertor proof_convertor("example.wcnf", "example_proof.msres", "output.pbp");
    proof_convertor.write_proof();

    std::cout << "Ended" << std::endl;
    return 0;
}