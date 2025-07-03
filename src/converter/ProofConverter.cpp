#include <string>
#include <vector>
#include <unordered_set>
#include <utility>
#include <algorithm>

#include "ProofConverter.h"

#include "../cnf/Clause.h"
#include "../cnf/Rule.h"
#include "../cnf/ResRule.h"
#include "../cnf/SplitRule.h"
#include "../parser/WCNFParser.h"

using namespace VeriPB;

namespace converter {

    ProofConverter::ProofConverter(const std::string wcnf_file, const std::string msres_file, const std::string output_file)
        : msres_parser(msres_file), output_file(output_file) {

        // Add the clauses from the WCNF file
        std::vector<cnf::Clause> clauses = parser::WCNFParser::parseWCNF(wcnf_file);
        for (int i = 0; i < clauses.size(); i++) {
            this->wcnf_clauses.emplace(i + 1, clauses[i]);
        }

        this->pl = new VeriPB::ProofloggerOpt<VeriPB::Lit, uint32_t, uint32_t>(this->output_file, &this->var_mgr);
        this->pl->set_comments(true);
    }

    // TODO: Segmentation fault
    ProofConverter::~ProofConverter() {
        // if (this->pl != nullptr) {
        //     delete this->pl;
        //     this->pl = nullptr;
        // }
    }

    void ProofConverter::write_proof() {
        initialize_vars();


        uint32_t unit_clauses = std::count_if(
            this->wcnf_clauses.begin(),
            this->wcnf_clauses.end(),
            [](const auto& pair) {
                return pair.second.is_unit_clause();
            }
        );

        // Write the proof header
        this->var_mgr.set_number_original_variables(this->vars.size());
        this->pl->write_proof_header();
        this->pl->set_n_orig_constraints(this->wcnf_clauses.size() - unit_clauses);

        //TODO: Reification clauses
        this->reificate_original_clauses();
        this->pl->flush_proof();

        // Write the proof
        cnf::Rule *rule;

        while (true) {
            rule = this->msres_parser.next_rule();
            
            if (rule == nullptr) {
                break;
            }
            rule->print();
            this->write_proof(rule);
            pl->write_comment("Rule: ");
            pl->write_comment("");
            pl->write_comment("");
        }


        pl->write_conclusion_NONE();
        pl->flush_proof();
        delete rule;
    }

    // TODO: SpltRule is not yet implemented
    void ProofConverter::write_proof(const cnf::Rule* rule) {
        if (dynamic_cast<const cnf::ResRule*>(rule)) {
            const cnf::ResRule* res_rule = dynamic_cast<const cnf::ResRule*>(rule);
            this->write_res_rule(res_rule);
        } else {
            throw std::runtime_error("Unknown rule type");
        }
    }


    void ProofConverter::write_res_rule(const cnf::ResRule* rule) {
        // Add the new clause
        uint32_t num_new_clauses = this->blocking_vars.size();
        this->write_new_clauses(rule);
        num_new_clauses = this->blocking_vars.size() - num_new_clauses;

        uint32_t constraint_id_1 = constraint_ids.at(rule->getClause1());
        uint32_t constraint_id_2 = constraint_ids.at(rule->getClause2());

        // Generate the four claims
        constraintid claim_1 = this->claim_type_1(*rule, false);
        pl->write_comment("__Claim 1__");
        constraintid claim_2 = this->claim_type_1(*rule, true);
        pl->write_comment("__Claim 2__");
        constraintid claim_3 = this->claim_3(
            constraint_id_1,
            constraint_id_2,
            *rule,
            rule->apply()
        );
        pl->write_comment("__Claim 3__");
        constraintid claim_4 = this->claim_4(
            constraint_id_1,
            constraint_id_2,
            *rule,
            rule->apply()
        );
        pl->write_comment("__Claim 4__");

        assemble_proof(claim_1, claim_2, claim_3, claim_4, constraint_id_1, constraint_id_2, num_new_clauses);
        change_objective(constraint_id_1, constraint_id_2, num_new_clauses);
    }


    void ProofConverter::initialize_vars() {
        // Initialize the variables from the original clauses
        for (const auto &[_, clause] : this->wcnf_clauses) {
            for (const auto &literal : clause.getLiterals()) {
                uint32_t var = std::abs(literal);

                if (vars.find(var) == vars.end()) {
                    VeriPB::Var new_var{.v = var, .only_known_in_proof = false};
                    VeriPB::Lit new_lit{.v = new_var, .negated = false};

                    this->vars[var] = new_lit;
                    this->var_mgr.store_variable_name(variable(new_lit), "x" + std::to_string(var));
                }
            }
        }
    }

    void ProofConverter::reificate_original_clauses() {
        // Saving the reification of the original clauses
        for (int i = 1; i <= this->wcnf_clauses.size(); i++) {
            const cnf::Clause& clause = this->wcnf_clauses.at(i);

            VeriPB::Var var = this->var_mgr.new_variable_only_in_proof();
            var_mgr.store_variable_name(var, "_b" + std::to_string(i)); 
            VeriPB::Lit lit{.v = var, .negated = false};

            this->blocking_vars[i] = lit;
            this->constraint_ids[clause] = i;
            
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

            this->pl->reification_literal_left_implication(this->blocking_vars[i], C, true);
        }
    }

    void ProofConverter::write_new_clauses(const cnf::Rule* rule) {
        std::vector<cnf::Clause> new_clauses = rule->apply();

        uint32_t curr_clause_id = this->blocking_vars.size();

        VeriPB::Constraint<VeriPB::Lit, uint32_t, uint32_t> C;
        for (int i = 0; i < new_clauses.size(); i++) {
            const cnf::Clause& clause = new_clauses[i];

            // Add the new blocking variable
            VeriPB::Var var = this->var_mgr.new_variable_only_in_proof();
            VeriPB::Lit lit = create_literal(var, false); // TODO
            this->blocking_vars[i + curr_clause_id + 1] = lit;
            this->constraint_ids[clause] = i + curr_clause_id + 1;

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

    void ProofConverter::assemble_proof(
        VeriPB::constraintid claim_1, 
        VeriPB::constraintid claim_2,
        VeriPB::constraintid claim_3,
        VeriPB::constraintid claim_4,
        uint32_t clause_id_1,
        uint32_t clause_id_2,
        uint32_t num_new_clauses
    ) {
        // s1 + s2 >= s3 + s4 + ... + s_n
        Constraint<VeriPB::Lit, uint32_t, uint32_t> C;
        C.add_literal(neg(blocking_vars[clause_id_1]), 1);
        C.add_literal(neg(blocking_vars[clause_id_2]), 1);
        for (int i = 0; i < num_new_clauses; i++) {
            C.add_literal(neg(blocking_vars[blocking_vars.size() - i]), 1);
        }
        C.add_RHS(num_new_clauses);

        CuttingPlanesDerivation cpder(pl, false);
        pl->write_comment("__Proof by contradiction__");
        pl->start_proof_by_contradiction(C);

        cpder.start_from_constraint(-1);
        cpder.add_constraint(claim_1);
        cpder.saturate();
        cpder.end();

        cpder.start_from_constraint(-2);
        cpder.add_constraint(claim_2);
        cpder.saturate();
        cpder.end();

        cpder.start_from_constraint(-2);
        cpder.add_constraint(-1);
        cpder.end();

        pl->end_proof_by_contradiction();
        pl->move_to_coreset_by_id(-1);


        // s1 + s2 <= s3 + s4 + ... + s_n
        C.clear();
        C.add_literal(blocking_vars[clause_id_1], 1);
        C.add_literal(blocking_vars[clause_id_2], 1);
        for (int i = 0; i < num_new_clauses; i++) {
            C.add_literal(blocking_vars[blocking_vars.size() - i], 1);
        }
        C.add_RHS(2);
        pl->write_comment("__Proof by contradiction__");
        pl->start_proof_by_contradiction(C);

        cpder.start_from_constraint(-1);
        cpder.add_constraint(claim_3);
        cpder.saturate();
        cpder.end();

        cpder.start_from_constraint(-2);
        cpder.add_constraint(claim_4);
        cpder.saturate();
        cpder.end();

        cpder.start_from_constraint(-2);
        cpder.add_constraint(-1);
        cpder.end();

        pl->end_proof_by_contradiction();
        pl->move_to_coreset_by_id(-1);
    }

    void ProofConverter::change_objective(uint32_t clause_id_1, uint32_t clause_id_2, uint32_t num_new_clauses) {
        // Objective aanpassen
        LinTermBoolVars<VeriPB::Lit, uint32_t, uint32_t> c_old;
        LinTermBoolVars<VeriPB::Lit, uint32_t, uint32_t> c_new;

        c_old.add_literal(neg(blocking_vars[clause_id_1]), 1);
        c_old.add_literal(neg(blocking_vars[clause_id_2]), 1);

        for (int i = 0; i < num_new_clauses; i++) {
            c_new.add_literal(blocking_vars[blocking_vars.size() - i], 1);
        }

        pl->write_objective_update_diff(c_old, c_new);
    }
}