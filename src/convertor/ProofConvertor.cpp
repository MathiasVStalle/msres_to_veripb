#include <string>
#include <vector>
#include <unordered_set>
#include <utility>

#include "ProofConvertor.h"

#include "../cnf/Clause.h"
#include "../cnf/Rule.h"
#include "../cnf/ResRule.h"
#include "../cnf/SplitRule.h"
#include "../parser/WCNFParser.h"

using namespace VeriPB;

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
        // Initializing all the variables from the original clauses
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
            this->pl->flush_proof();

            break; // TODO: Remove this line
        }

        delete rule;
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
        // TODO: Remove this
        cnf::ResRule* res_rule = new cnf::ResRule(rule->getClause1(), rule->getClause2(), 1, 2);

        // Get the starting constraint id's from both clauses
        std::pair<
            std::pair<VeriPB::constraintid, VeriPB::constraintid>, 
            std::pair<VeriPB::constraintid, VeriPB::constraintid>
        >
        id_pair = this->get_constraint_ids(
            res_rule->get_constraint_id_1(), 
            res_rule->get_constraint_id_2()
        );

        // Add the new clause
        uint32_t num_new_clauses = this->blocking_vars.size();
        this->write_new_clauses(rule);
        num_new_clauses = this->blocking_vars.size() - num_new_clauses;

        // Generate the four claims
        constraintid claim_1 = this->claim_1(
            res_rule->get_constraint_id_1(),
            res_rule->get_constraint_id_2(),
            id_pair.second.first,
            num_new_clauses,
            *res_rule,
            res_rule->apply()
        );
        constraintid claim_2 = this->claim_2(
            res_rule->get_constraint_id_1(),
            res_rule->get_constraint_id_2(),
            id_pair.first.first,
            num_new_clauses,
            *res_rule,
            res_rule->apply()
        );
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

                if (literal < 0) {
                    C.add_literal(neg(lit), 1);
                } else {
                    C.add_literal(lit, 1);
                }
            }

            this->pl->reification_literal_left_implication(neg(this->blocking_vars[i]), C, true);
        }
    }

    void ProofConvertor::write_new_clauses(const cnf::Rule* rule) {
        std::vector<cnf::Clause> new_clauses = rule->apply();

        uint32_t curr_clause_id = this->blocking_vars.size();

        VeriPB::Constraint<VeriPB::Lit, uint32_t, uint32_t> C;
        for (int i = 0; i < new_clauses.size(); i++) {
            const cnf::Clause& clause = new_clauses[i];

            // Add the new blocking variable
            VeriPB::Var var = this->var_mgr.new_variable_only_in_proof();
            VeriPB::Lit lit = create_literal(var, false); // TODO
            this->blocking_vars[i + curr_clause_id + 1] = lit;

            // Add the new clause to the proof logger
            C.clear();
            for (const auto& literal : clause.getLiterals()) {
                uint32_t var = std::abs(literal);
                VeriPB::Lit new_lit = this->vars[var];

                if (literal < 0) {
                    C.add_literal(neg(new_lit), 1);
                } else {
                    C.add_literal(new_lit, 1);
                }
            }
            C.add_RHS(1);
            this->pl->reification_literal_right_implication(neg(lit), C, true);
            this->pl->reification_literal_left_implication(neg(lit), C, true);
        }
    }

    // TODO: Implement this function further
    std::pair<
        std::pair<VeriPB::constraintid, VeriPB::constraintid>, 
        std::pair<VeriPB::constraintid, VeriPB::constraintid>
    > 
    ProofConvertor::get_constraint_ids(const uint32_t id_1, const uint32_t id_2) {
        std::pair<VeriPB::constraintid, VeriPB::constraintid> first_pair;
        std::pair<VeriPB::constraintid, VeriPB::constraintid> second_pair;

        if (id_1 <= this->wcnf_clauses.size()) {
            first_pair.first = (VeriPB::constraintid) id_1;
            first_pair.second = (VeriPB::constraintid) id_1 + this->wcnf_clauses.size();
        } else {
            // throw
            std::cerr << "Not yet implemented" << std::endl;
        }

        if (id_2 <= this->wcnf_clauses.size()) {
            second_pair.first = (VeriPB::constraintid) id_2;
            second_pair.second = (VeriPB::constraintid) id_2 + this->wcnf_clauses.size();
        } else {
            // throw
            std::cerr << "Not yet implemented" << std::endl;
        }

        return std::make_pair(first_pair, second_pair);
    }
}