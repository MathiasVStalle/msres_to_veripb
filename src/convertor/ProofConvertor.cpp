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
        this->claim_1(
            res_rule->get_constraint_id_1(),
            res_rule->get_constraint_id_2(),
            id_pair.second.first,
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

    VeriPB::constraintid ProofConvertor::claim_1(
        const uint32_t clause_id_1,
        const uint32_t clause_id_2,
        const VeriPB::constraintid constr_id, 
        const uint32_t num_new_clauses,
        const cnf::ResRule& rule,
        const std::vector<cnf::Clause>& new_clauses
    ) {
        VeriPB::CuttingPlanesDerivation cpder(this->pl, false);

        std::unordered_set<int32_t> literals_set_clause_1 = rule.getClause1().getLiterals();
        std::unordered_set<int32_t> literals_set_clause_2 = rule.getClause2().getLiterals();

        int32_t rhs = num_new_clauses - 2;

        // Remove pivot literal from both clauses
        uint32_t pivot = rule.get_pivot();
        literals_set_clause_1.erase(pivot);
        literals_set_clause_2.erase(-pivot);

        // Ordered set for iteration
        std::vector<int32_t> literals_clause_1(literals_set_clause_1.begin(), literals_set_clause_1.end());
        std::vector<int32_t> literals_clause_2(literals_set_clause_2.begin(), literals_set_clause_2.end());

        // Frequently used variables
        Lit x = this->vars[pivot];
        Lit s1 = this->blocking_vars[clause_id_1];
        Lit s2 = this->blocking_vars[clause_id_2];
        Lit s3 = this->blocking_vars[blocking_vars.size() - (num_new_clauses - 1)];

        // Excecute the proof steps
        VeriPB::constraintid cxn_1 = claim_1_step_1(cpder, x, s3, literals_clause_1, literals_clause_2);
        VeriPB::constraintid cxn_2 = claim_1_step_2(cpder, x, s3, literals_clause_1, literals_clause_2);
        VeriPB::constraintid cxn_3 = claim_1_step_3(cpder, cxn_1, cxn_2, literals_clause_1.size());
        std::vector<VeriPB::constraintid> subclaims = claim_1_step_4(
            cpder,
            constr_id,
            x,
            s2,
            s3,
            literals_clause_1,
            literals_clause_2
        );
        VeriPB::constraintid cxn_4 = claim_1_contradiction(
            cpder,
            x,
            s2,
            s3,
            literals_clause_1,
            literals_clause_2,
            subclaims
        );
        
        // Build the final claim
        cpder.start_from_literal_axiom(neg(x));
        cpder.multiply(rhs - 1);
        cpder.add_constraint(-1);
        VeriPB::constraintid subclaim = cpder.end();
        
        int32_t counter = 0;
        int32_t offset = blocking_vars.size() - literals_clause_1.size();
        for (int i = 0; i < literals_clause_2.size(); i++) {
            VeriPB::Lit sn = this->blocking_vars[offset - 1];
        
            // Take all the unused constraints and weaken them
            cpder.start_from_constraint(this->pl->get_reified_constraint_right_implication(variable(sn)));
            for (int j = 0; j < literals_clause_1.size(); j++) {
                cpder.weaken(variable(this->vars[std::abs(literals_clause_1[j])]));
            }
            int32_t not_neaded = literals_clause_2.size() - i - 1;
            for (int j = 0; j < literals_clause_2.size() - not_neaded; j++) {
                cpder.weaken(variable(this->vars[std::abs(literals_clause_2[j])]));
            }
            cpder.saturate();
            cpder.end();
            counter++;
        }

        // Add all the previous constraints
        cpder.start_from_constraint(-counter);
        for (int i = counter - 1; i > 0; i--) {
            cpder.add_constraint(-i);
        }
        cpder.add_constraint(subclaim);
        cpder.add_literal_axiom(s1);
        VeriPB::constraintid result = cpder.end();
        
        this->pl->flush_proof();
        
        return result;
    }

    VeriPB::constraintid ProofConvertor::claim_1_step_1(
        CuttingPlanesDerivation& cpder,
        Lit x,
        Lit s3, 
        std::vector<int32_t>& literals_1, 
        std::vector<int32_t>& literals_2
    ) {
        int32_t counter = 1;

        // Weaken a_1 and every b_i on s3
        cpder.start_from_constraint(pl->get_reified_constraint_left_implication(variable(s3)));
        cpder.weaken(variable(vars[std::abs(literals_1[0])]));
        for (int i = 0; i < literals_2.size(); i++) {
            cpder.weaken(variable(vars[std::abs(literals_2[i])]));
        }
        cpder.saturate();
        constraintid cxn = cpder.end();

        // Weaken on everything except a_n
        for (int i = literals_1.size() - 1; i >= 0; i--) {
            VeriPB::Lit sn = blocking_vars[blocking_vars.size() - i];
            cpder.start_from_constraint(pl->get_reified_constraint_left_implication(variable(sn)));

            cpder.weaken(variable(x));

            for (int j = 0; j < literals_1.size() - i - 1; j++) {
                cpder.weaken(variable(vars[std::abs(literals_1[j])]));
            }
            for (int j = 0; j < literals_2.size(); j++) {
                cpder.weaken(variable(vars[std::abs(literals_2[j])]));
            }
            cpder.saturate();
            cxn = cpder.end();
            counter++;
        }

        // Add all the previous constraints
        cpder.start_from_constraint(-counter);
        for (int i = counter - 1; i > 0; i--) {
            cpder.add_constraint(-i);
        }
        cxn = cpder.end();

        return cxn;
    }

    VeriPB::constraintid ProofConvertor::claim_1_step_2(
        CuttingPlanesDerivation& cpder,
        Lit x,
        Lit s3,
        std::vector<int32_t>& literals_1, 
        std::vector<int32_t>& literals_2
    ) {
        int32_t counter = 1;

        // Weaken on everything except a_1
        cpder.start_from_constraint(pl->get_reified_constraint_left_implication(variable(s3)));
        for (int i = 1; i < literals_1.size(); i++) {
            cpder.weaken(variable(vars[std::abs(literals_1[i])]));
        }
        for (int i = 0; i < literals_2.size(); i++) {
            cpder.weaken(variable(vars[std::abs(literals_2[i])]));
        }
        cpder.saturate();
        VeriPB::constraintid cxn = cpder.end();

        // Weaken on everything except a_1
        for (int i = literals_1.size() - 2; i >= 0; i--) {
            VeriPB::Lit sn = this->blocking_vars[blocking_vars.size() - i];
            cpder.start_from_constraint(this->pl->get_reified_constraint_left_implication(variable(sn)));

            cpder.weaken(variable(x));

            for (int j = 1; j < literals_1.size() - i; j++) {
                cpder.weaken(variable(this->vars[std::abs(literals_1[j])]));
            }
            for (int j = 0; j < literals_2.size(); j++) {
                cpder.weaken(variable(this->vars[std::abs(literals_2[j])]));
            }
            cpder.saturate();
            cpder.end();
            counter++;
        }

        cpder.start_from_constraint(-counter);
        for (int i = counter - 1; i > 0; i--) {
            cpder.add_constraint(-i);
        }
        cxn = cpder.end();

        return cxn;
    }

    VeriPB::constraintid ProofConvertor::claim_1_step_3(
        CuttingPlanesDerivation& cpder,
        constraintid cxn_1,
        constraintid cxn_2,
        int32_t counter
    ) {
        cpder.start_from_constraint(cxn_1);
        cpder.multiply(counter);
        cpder.end();

        cpder.start_from_constraint(cxn_2);
        cpder.add_constraint(-1);
        cpder.divide(counter);
        constraintid cxn = cpder.end();

        return cxn;
    }

    std::vector<constraintid> ProofConvertor::claim_1_step_4(
        CuttingPlanesDerivation& cpder,
        constraintid c_id_s2,
        Lit x,
        Lit s2,
        Lit s3,
        std::vector<int32_t>& literals_1, 
        std::vector<int32_t>& literals_2
    ) {
        std::vector<VeriPB::constraintid> subclaims;

        cpder.start_from_constraint(c_id_s2);
        cpder.add_constraint(-1);
        subclaims.push_back(cpder.end());

        // Repeat for every b_i
        for (int i = 0; i < literals_2.size(); i++) {
            int32_t counter = 1;

            // Weaken on everything except b_i
            cpder.start_from_constraint(pl->get_reified_constraint_left_implication(variable(s3)));
            for (int j = 0; j < literals_1.size(); j++) {
                cpder.weaken(variable(vars[std::abs(literals_1[j])]));
            }
            for (int j = 0; j < literals_2.size(); j++) {
                if (i != j) continue;

                cpder.weaken(variable(vars[std::abs(literals_2[j])]));
            }
            cpder.saturate();
            cpder.end();

            // Weaken on everything except b_i
            for (int j = literals_1.size() - 1; j >= 0; j--) {
                VeriPB::Lit sn = blocking_vars[blocking_vars.size() - j];
                cpder.start_from_constraint(pl->get_reified_constraint_left_implication(variable(sn)));

                cpder.weaken(variable(x));

                for (int k = 0; k < literals_1.size() - j; k++) {
                    cpder.weaken(variable(vars[std::abs(literals_1[k])]));
                }
                for (int k = 0; k < literals_2.size(); k++) {
                    if (i != k) continue;

                    cpder.weaken(variable(vars[std::abs(literals_2[k])]));
                }
                cpder.saturate();
                cpder.end();
                counter++;
            }

            // Add all the previous constraints
            cpder.start_from_constraint(-counter);
            for (int j = counter - 1; j > 0; j--) {
                cpder.add_constraint(-j);
            }
            cpder.end();

            // Add the missing literals
            cpder.start_from_constraint(-1);
            cpder.add_literal_axiom(s2);
            cpder.add_literal_axiom(neg(x));

            subclaims.push_back(cpder.end());
        }

        return subclaims;
    }

    VeriPB::constraintid ProofConvertor::claim_1_contradiction(
        CuttingPlanesDerivation& cpder,
        Lit x,
        Lit s2,
        Lit s3,
        std::vector<int32_t>& literals_1, 
        std::vector<int32_t>& literals_2,
        std::vector<VeriPB::constraintid>& subclaims
    ) {
        uint32_t num_new_clauses = literals_1.size() + literals_2.size() + 1;

        // make contradicting constraint
        Constraint<VeriPB::Lit, uint32_t, uint32_t> C;
        C.add_literal(neg(x), 1);
        C.add_literal(s2, 1);
        C.add_literal(neg(s3), 1);
        for (int i = literals_1.size() - 1; i >= 0; i--) {
            VeriPB::Lit sn = blocking_vars[blocking_vars.size() - i];
            C.add_literal(neg(sn), 1);
        }
        C.add_RHS((num_new_clauses + 1) / 2);

        // Proof by constradiction (1 ~x1 1 s2 1 ~s3 1 ~s6 ... 1 ~sn >= m)
        int32_t counter = 0;
        VeriPB::constraintid cxnneg = this->pl->start_proof_by_contradiction(C);
        for (auto& subclaim : subclaims) {
            cpder.start_from_constraint(cxnneg);
            cpder.add_constraint(subclaim);
            cpder.saturate();
            cpder.end();
            counter++;
        }

        // Add all the previous constraints
        cpder.start_from_constraint(-counter);
        for (int i = counter - 1; i > 0; i--) {
            cpder.add_constraint(-i);
        }
        cpder.saturate();
        cpder.end();

        cxnneg = pl->end_proof_by_contradiction();

        return cxnneg;
    }
}