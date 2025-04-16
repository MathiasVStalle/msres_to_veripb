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
    cnf::Rule* rule_1 = parser.next_rule();
    cnf::Rule* rule_2 = parser.next_rule();

    if (rule_1 != nullptr) {
        rule_1->print();
        delete rule_1;
    } else {
        std::cout << "No more rules" << std::endl;
    }

    if (rule_2 != nullptr) {
        rule_2->print();
        delete rule_2;
    } else {
        std::cout << "No more rules" << std::endl;
    }

    std::cout << "Ended" << std::endl;
    return 0;
}