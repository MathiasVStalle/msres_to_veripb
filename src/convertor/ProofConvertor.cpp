#include <string>
#include <unordered_set>

#include "ProofConvertor.h"

#include "../cnf/Clause.h"
#include "../parser/WCNFParser.h"

namespace convertor {

    ProofConvertor::ProofConvertor(std::string wcnf_file, std::string msres_file)
        : msres_parser(msres_file), output_file(wcnf_file) {

        this->wcnf_clauses = parser::WCNFParser::parseWCNF(wcnf_file);
        this->pl = new VeriPB::ProofloggerOpt<VeriPB::Lit, uint32_t, uint32_t>(wcnf_file, &this->var_mgr);
        this->pl->set_comments(true);
    }

    ProofConvertor::~ProofConvertor() {
        if (this->pl != nullptr) {
            delete this->pl;
        }
    }

    void ProofConvertor::write_proof() {
        cnf::Rule *rule;

        while (true) {
            rule = this->msres_parser.next_rule();
            
            if (rule == nullptr) {
                break;
            }

            // this->write_proof(rule);

            delete rule;
        }

    }
}