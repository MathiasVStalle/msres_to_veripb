#include <string>
#include <vector>
#include <unordered_set>

#include "ProofConvertor.h"

#include "../cnf/Clause.h"
#include "../cnf/Rule.h"
#include "../cnf/ResRule.h"
#include "../cnf/SplitRule.h"
#include "../parser/WCNFParser.h"

namespace convertor {

    ProofConvertor::ProofConvertor(const std::string wcnf_file, const std::string msres_file, const std::string output_file)
        : msres_parser(msres_file), output_file(output_file) {

        std::vector<cnf::Clause> clauses = parser::WCNFParser::parseWCNF(wcnf_file);
        for (int i = 0; i < clauses.size(); i++) {
            this->wcnf_clauses.emplace(i + 1, clauses[i]);
        }

        this->pl = new VeriPB::ProofloggerOpt<VeriPB::Lit, uint32_t, uint32_t>(this->output_file, &this->var_mgr);
        this->pl->set_comments(true);
    }

    // Flag: Segmentation fault
    ProofConvertor::~ProofConvertor() {
        // if (this->pl != nullptr) {
        //     delete this->pl;
        //     this->pl = nullptr;
        // }
    }

    void ProofConvertor::write_proof() {
        // Initializing all the variables
        for (const auto& pair : this->wcnf_clauses) {
            const cnf::Clause& clause = pair.second;

            for (const auto& literal : clause.getLiterals()) {
                uint32_t var = std::abs(literal);

                if (this->vars.find(var) == this->vars.end()) {
                    VeriPB::Var new_var{.v = var, .only_known_in_proof = false};
                    VeriPB::Lit new_lit{.v = new_var, .negated = false};
                    
                    this->vars[var] = new_lit;
                    this->var_mgr.store_variable_name(variable(new_lit), "x" + std::to_string(var));
                }
            }
        }

        // Write the proof header
        this->var_mgr.set_number_original_variables(this->vars.size());
        this->pl->write_proof_header();
        this->pl->set_n_orig_constraints(this->wcnf_clauses.size());

        // Print the maps for debugging
        std::cout << "Clauses:" << std::endl;
        for (const auto& pair : this->wcnf_clauses) {
            std::cout << "Clause " << pair.first << ": ";
            const cnf::Clause& clause = pair.second;
            clause.print();
        }

        //TODO: Reification clauses
        this->reificate();
        this->pl->flush_proof();

        // Write the proof
        cnf::Rule *rule;

        while (true) {
            rule = this->msres_parser.next_rule();
            
            if (rule == nullptr) {
                break;
            }

            this->write_proof(rule);

            delete rule;
        }
    }

    // TODO: SpltRule is not yet implemented
    void ProofConvertor::write_proof(const cnf::Rule* rule) {
        if (dynamic_cast<const cnf::ResRule*>(rule)) {
            const cnf::ResRule* res_rule = dynamic_cast<const cnf::ResRule*>(rule);
            this->write_res_rule(res_rule);
        } else {
            throw std::runtime_error("Unknown rule type");
        }
    }

    void ProofConvertor::write_res_rule(const cnf::ResRule* rule) {
    
    }

    void ProofConvertor::reificate() {
        // Saving the reification of the original clauses
        for (int i = 1; i <= this->wcnf_clauses.size(); i++) {
            const cnf::Clause& clause = this->wcnf_clauses.at(i);

            VeriPB::Var var = this->var_mgr.new_variable_only_in_proof();
            VeriPB::Lit lit{.v = var, .negated = false};

            this->blocking_vars[i] = lit;
            
            this->pl->store_reified_constraint_right_implication(variable(lit), i);
        }

        // Saving the reification in the other direction
        VeriPB::Constraint<VeriPB::Lit, uint32_t, uint32_t> C;
        for (int i = 1; i <= this->wcnf_clauses.size(); i++) {
            const cnf::Clause &clause = this->wcnf_clauses.at(i);

            C.clear();
            C.add_RHS(1);

            for (const auto &literal : clause.getLiterals())
            {
                uint32_t var = std::abs(literal);
                VeriPB::Lit lit = this->vars[var];

                if (literal < 0)
                {
                    C.add_literal(neg(lit), 1);
                }
                else
                {
                    C.add_literal(lit, 1);
                }
            }

            this->pl->reification_literal_left_implication(neg(this->blocking_vars[i]), C, true);
        }
    }
}