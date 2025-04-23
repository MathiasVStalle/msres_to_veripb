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
        this->claim_1(
            id_pair.first.first, 
            id_pair.second.first,
            num_new_clauses,
            *res_rule,
            res_rule->apply()
        );
    }

    void ProofConvertor::claim_1(
        const uint32_t clause_id,
        const VeriPB::constraintid constr_id, 
        const uint32_t num_new_clauses,
        const cnf::ResRule& rule,
        const std::vector<cnf::Clause>& new_clauses
    ) {
        int32_t counter = 0;

        VeriPB::CuttingPlanesDerivation cpder(this->pl, false);
        VeriPB::Lit s2 = this->blocking_vars[clause_id];
        VeriPB::Lit s3 = this->blocking_vars[blocking_vars.size() - (num_new_clauses - 1)];

        std::unordered_set<int32_t> literals_set_clause_1 = rule.getClause1().getLiterals();
        std::unordered_set<int32_t> literals_set_clause_2 = rule.getClause2().getLiterals();

        // Remove pivot literal from both clauses
        uint32_t pivot = rule.get_pivot();
        literals_set_clause_1.erase(pivot);
        literals_set_clause_2.erase(-pivot);

        // Ordered set for iteration
        std::vector<int32_t> literals_clause_1(literals_set_clause_1.begin(), literals_set_clause_1.end());
        std::vector<int32_t> literals_clause_2(literals_set_clause_2.begin(), literals_set_clause_2.end());


        // Weaken on a_1 and every b_i
        cpder.start_from_constraint(this->pl->get_reified_constraint_left_implication(variable(s3)));
        cpder.weaken(variable(this->vars[std::abs(literals_clause_1[0])]));
        for (int i = 0; i < literals_clause_2.size(); i++) {
            cpder.weaken(variable(this->vars[std::abs(literals_clause_2[i])]));
        }
        cpder.saturate();
        VeriPB::constraintid cxn_1 = cpder.end();

        // Weaken on everything except a_n
        counter = 1;
        VeriPB::Lit x = this->vars[pivot];
        for (int i = literals_clause_1.size() - 1; i >= 0; i--) {
            VeriPB::Lit sn = this->blocking_vars[blocking_vars.size() - i];
            cpder.start_from_constraint(this->pl->get_reified_constraint_left_implication(variable(sn)));

            cpder.weaken(variable(x));

            for (int j = 0; j < literals_clause_1.size() - i - 1; j++) {
                cpder.weaken(variable(this->vars[std::abs(literals_clause_1[j])]));
            }
            for (int j = 0; j < literals_clause_2.size(); j++) {
                cpder.weaken(variable(this->vars[std::abs(literals_clause_2[j])]));
            }
            cpder.saturate();
            cxn_1 = cpder.end();
            counter++;
        }

        cpder.start_from_constraint(-counter);
        for (int i = counter - 1; i > 0; i--) {
            cpder.add_constraint(-i);
        }
        cxn_1 = cpder.end();
        counter = 0;

        // Weaken on everything except a_1
        cpder.start_from_constraint(this->pl->get_reified_constraint_left_implication(variable(s3)));
        for (int i = 1; i < literals_clause_1.size(); i++) {
            cpder.weaken(variable(this->vars[std::abs(literals_clause_1[i])]));
        }
        for (int i = 0; i < literals_clause_2.size(); i++) {
            cpder.weaken(variable(this->vars[std::abs(literals_clause_2[i])]));
        }
        cpder.saturate();
        VeriPB::constraintid cxn_2 = cpder.end();
        counter = 1;

        // Weaken on everything except a_1
        for (int i = literals_clause_1.size() - 2; i >= 0; i--) {
            VeriPB::Lit sn = this->blocking_vars[blocking_vars.size() - i];
            cpder.start_from_constraint(this->pl->get_reified_constraint_left_implication(variable(sn)));

            cpder.weaken(variable(x));

            for (int j = 1; j < literals_clause_1.size() - i; j++) {
                cpder.weaken(variable(this->vars[std::abs(literals_clause_1[j])]));
            }
            for (int j = 0; j < literals_clause_2.size(); j++) {
                cpder.weaken(variable(this->vars[std::abs(literals_clause_2[j])]));
            }
            cpder.saturate();
            cxn_1 = cpder.end();
            counter++;
        }

        cpder.start_from_constraint(-counter);
        for (int i = counter - 1; i > 0; i--) {
            cpder.add_constraint(-i);
        }
        cxn_2 = cpder.end();
        


        cpder.start_from_constraint(cxn_1);
        cpder.multiply(counter);
        cxn_1 = cpder.end();

        cpder.start_from_constraint(cxn_2);
        cpder.add_constraint(-1);
        cpder.divide(counter);
        cxn_1 = cpder.end();

        std::vector<VeriPB::constraintid> subclaims;

        cpder.start_from_constraint(constr_id);
        cpder.add_constraint(-1);
        subclaims.push_back(cpder.end());



        for (int i = 0; i < literals_clause_2.size(); i++) {
            // Weaken on everything except b_i
            cpder.start_from_constraint(this->pl->get_reified_constraint_left_implication(variable(s3)));
            for (int j = 0; j < literals_clause_1.size(); j++) {
                cpder.weaken(variable(this->vars[std::abs(literals_clause_1[j])]));
            }
            for (int j = 0; j < literals_clause_2.size(); j++) {
                if (i != j) continue;

                cpder.weaken(variable(this->vars[std::abs(literals_clause_2[j])]));
            }
            cpder.saturate();
            cxn_1 = cpder.end();
            counter = 1;

            // Weaken on everything except b_i
            for (int j = literals_clause_1.size() - 1; j >= 0; j--) {
                VeriPB::Lit sn = this->blocking_vars[blocking_vars.size() - j];
                cpder.start_from_constraint(this->pl->get_reified_constraint_left_implication(variable(sn)));

                cpder.weaken(variable(x));

                for (int k = 0; k < literals_clause_1.size() - j; k++) {
                    cpder.weaken(variable(this->vars[std::abs(literals_clause_1[k])]));
                }
                for (int k = 0; k < literals_clause_2.size(); k++) {
                    if (i != k) continue;

                    cpder.weaken(variable(this->vars[std::abs(literals_clause_2[k])]));
                }
                cpder.saturate();
                cxn_1 = cpder.end();
                counter++;
            }

            cpder.start_from_constraint(-counter);
            for (int j = counter - 1; j > 0; j--) {
                cpder.add_constraint(-j);
            }
            subclaims.push_back(cpder.end());
        }



        // Proof by constradiction (1 ~x1 1 s2 1 ~s3 1 ~s6 ... 1 ~sn >= m)
        VeriPB::Constraint<VeriPB::Lit, uint32_t, uint32_t> C;
        C.add_literal(neg(x), 1);
        C.add_literal(s2, 1);
        C.add_literal(neg(s3), 1);
        for (int i = literals_clause_1.size() - 1; i >= 0; i--) {
            VeriPB::Lit sn = this->blocking_vars[blocking_vars.size() - i];
            C.add_literal(neg(sn), 1);
        }
        C.add_RHS((num_new_clauses + 1) / 2);

        counter = 0;
        VeriPB::constraintid cxnneg = this->pl->start_proof_by_contradiction(C);
        for (auto& subclaim : subclaims) {
            cpder.start_from_constraint(cxnneg);
            cpder.add_constraint(subclaim);
            cpder.saturate();
            cpder.end();
            counter++;
        }
        cpder.start_from_constraint(-counter);
        for (int i = counter - 1; i > 0; i--) {
            cpder.add_constraint(-i);
        }
        cpder.saturate();
        cpder.end();
        VeriPB::constraintid cxnneg2 = pl->end_proof_by_contradiction();

        this->pl->flush_proof();
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