#include <string>
#include <vector>
#include <unordered_set>
#include <utility>
#include <algorithm>

#include "Hash.h"
#include "ProofConverter.h"
#include "ResClaimTypeA.h"
#include "ResClaimTypeB.h"
#include "SplitClaim.h"
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

    ProofConverter::~ProofConverter() {
        if (this->pl != nullptr) {
            delete this->pl;
            this->pl = nullptr;
        }
    }

    void ProofConverter::write_proof() {
        initialize_vars();

        uint32_t n_original = this->wcnf_clauses.size();
        for (const auto &[_, clause] : this->wcnf_clauses) {
            if (clause.is_unit_clause()) {
                n_original--;
            }
        }

        // Write the proof header
        var_mgr.set_number_original_variables(vars.size());
        pl->write_proof_header();
        pl->set_n_orig_constraints(n_original);

        // Add the clauses from the WCNF file
        reificate_original_clauses();
        pl->flush_proof();

        // Write the proof
        cnf::Rule *rule;
        while (true) {
            rule = this->msres_parser.next_rule();
            
            if (rule == nullptr)
                break;

            write_proof(rule);
            pl->flush_proof();

            delete rule;
        }

        pl->write_conclusion_NONE();
        pl->flush_proof();
        delete rule;
    }


    void ProofConverter::write_proof(const cnf::Rule* rule) {
        if (dynamic_cast<const cnf::ResRule*>(rule)) {
            const cnf::ResRule* res_rule = dynamic_cast<const cnf::ResRule*>(rule);
            this->write_res_rule(res_rule);
        } else if (dynamic_cast<const cnf::SplitRule*>(rule)) {
            const cnf::SplitRule* split_rule = dynamic_cast<const cnf::SplitRule*>(rule);
            this->write_split_rule(split_rule);
        } else {
            throw std::runtime_error("Unknown rule type: " + std::string(typeid(*rule).name()));
        }
    }

    void ProofConverter::write_res_rule(const cnf::ResRule* rule) {
        // Add the new clause
        std::vector<cnf::Clause> new_clauses = rule->apply();
        this->write_new_clauses(new_clauses);

        std::vector<std::pair<VeriPB::Lit, cnf::Clause>> clauses;

        cnf::Clause clause_1 = rule->get_clause_1();
        cnf::Clause clause_2 = rule->get_clause_2();

        clauses = {
            std::make_pair(blocking_vars[clause_1], clause_1),
            std::make_pair(blocking_vars[clause_2], clause_2)
        };
        for (const auto &clause : new_clauses) {
            clauses.push_back(
                std::make_pair(blocking_vars[clause], clause)
            );
        }

        std::function<VeriPB::Lit(int32_t)> var_supplier = [this](int32_t x) -> VeriPB::Lit { return vars[std::abs(x)]; };
        std::function<bool(VeriPB::Lit)> tautology_predicate = [this](VeriPB::Lit lit) -> bool { return tautologies.contains(lit); };
        std::function<bool(VeriPB::Lit)> hard_clause_predicate = [this](VeriPB::Lit lit) -> bool { return hard_clauses.contains(lit); };

        ResClaimTypeA c_1 = ResClaimTypeA(*rule, clauses, var_supplier, tautology_predicate, hard_clause_predicate, false);
        ResClaimTypeA c_2 = ResClaimTypeA(*rule, clauses, var_supplier, tautology_predicate, hard_clause_predicate, true);
        
        // Generate the four claims
        constraintid claim_1 = c_1.write(*pl);
        pl->write_comment("__Claim 1__");
        constraintid claim_2 = c_2.write(*pl);
        pl->write_comment("__Claim 2__");

        constraintid claim_3;
        constraintid claim_4;


        // TODO: Clean up
        if (clause_1.is_hard_clause()) {
            ResClaimTypeB cl = ResClaimTypeB(*rule, clauses, var_supplier, tautology_predicate, hard_clause_predicate, false);
            claim_3 = cl.write(*pl);
            assemble_proof(claim_1, claim_2, claim_3, clause_1, clause_2, new_clauses);
        } else if (clause_2.is_hard_clause()) {
            ResClaimTypeB cl = ResClaimTypeB(*rule, clauses, var_supplier, tautology_predicate, hard_clause_predicate, true);
            claim_3 = cl.write(*pl);
            assemble_proof(claim_1, claim_2, claim_3, clause_1, clause_2, new_clauses);
        } else {
            ResClaimTypeB c_3 = ResClaimTypeB(*rule, clauses, var_supplier, tautology_predicate, hard_clause_predicate, false);
            ResClaimTypeB c_4 = ResClaimTypeB(*rule, clauses, var_supplier, tautology_predicate, hard_clause_predicate, true);
            
            claim_3 = c_3.write(*pl);
            pl->write_comment("__Claim 3__");
            claim_4 = c_4.write(*pl);
            pl->write_comment("__Claim 4__");

            assemble_proof(claim_1, claim_2, claim_3, claim_4, clause_1, clause_2, new_clauses);
        }
        change_objective(clause_1, clause_2, new_clauses);
    }

    // TODO: If the pivot doesn't exist if the prooflogger, add it
    void ProofConverter::write_split_rule(const cnf::SplitRule* rule) {
        std::vector<cnf::Clause> new_clauses = rule->apply();
        this->write_new_clauses(new_clauses);

        std::vector<std::pair<VeriPB::Lit, cnf::Clause>> clauses;

        cnf::Clause clause = rule->get_clause();
        clauses = {
            std::make_pair(blocking_vars[clause], clause),
            std::make_pair(blocking_vars[new_clauses[0]], new_clauses[0]),
            std::make_pair(blocking_vars[new_clauses[1]], new_clauses[1])
        };

        std::function<bool(VeriPB::Lit)> tautology_predicate = [this](VeriPB::Lit lit) -> bool { return tautologies.contains(lit); };
        std::function<bool(VeriPB::Lit)> hard_clause_predicate = [this](VeriPB::Lit lit) -> bool { return hard_clauses.contains(lit); };

        SplitClaim c_1 = SplitClaim(clauses, tautology_predicate, hard_clause_predicate, false);
        SplitClaim c_2 = SplitClaim(clauses, tautology_predicate, hard_clause_predicate, true);
        constraintid claim_1 = c_1.write(*pl);
        constraintid claim_2 = c_2.write(*pl);
        pl->move_to_coreset_by_id(claim_1);
        pl->move_to_coreset_by_id(claim_2);
        change_objective(clause, new_clauses[0], new_clauses[1]);
    }


    void ProofConverter::initialize_vars() {
        // Initialize the variables from the original clauses
        for (const auto &[_, clause] : this->wcnf_clauses) {
            for (const auto &literal : clause.get_literals()) {
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

    // TODO: Simplify this function and add a clauses parameter
    void ProofConverter::reificate_original_clauses() {
        std::vector<std::pair<VeriPB::Lit, VeriPB::Lit>> unit_clauses;

        // Saving the reification of the original clauses
        for (int i = 1, cxn = 1; i <= wcnf_clauses.size(); i++) {
            const cnf::Clause& clause = wcnf_clauses.at(i);

            VeriPB::Var var = var_mgr.new_variable_only_in_proof();
            VeriPB::Lit lit = create_literal(var, false);
            var_mgr.store_variable_name(var, "_b" + std::to_string(i));
            blocking_vars[clause] = lit;

            if (clause.is_unit_clause() && !clause.is_hard_clause()) {
                int32_t unit_lit_int = *clause.get_literals().begin();
                VeriPB::Lit unit_lit = vars[std::abs(unit_lit_int)];
                unit_lit = (unit_lit_int < 0) ? neg(unit_lit) : unit_lit;

                unit_clauses.push_back(std::make_pair(lit, unit_lit));
            } else {
                pl->store_reified_constraint_right_implication(var, cxn++);
            }

            if (clause.is_hard_clause()) {
                hard_clauses.insert(lit);
            }
        }

        // Saving the reification in the other direction
        VeriPB::Constraint<VeriPB::Lit, uint32_t, uint32_t> C;
        for (int i = 1; i <= wcnf_clauses.size(); i++) {
            const cnf::Clause &clause = wcnf_clauses.at(i);

            if (clause.is_hard_clause()) {
                continue;
            }

            C.clear();
            C.add_RHS(1);

            // TODO: use clause_to_constraint function
            for (const auto &literal : clause.get_literals_set()) {
                uint32_t var = std::abs(literal);
                VeriPB::Lit lit = vars[var];

                if (literal < 0) {
                    C.add_literal(neg(lit), 1);
                } else {
                    C.add_literal(lit, 1);
                }
            }

            pl->reification_literal_left_implication(blocking_vars[clause], C, true);

            // Move to the coresets
            if (clause.is_unit_clause()) {
                pl->move_to_coreset_by_id(-1);
            }
        }

        for (const auto &pair : unit_clauses) {
            VeriPB::Lit blocking_lit = pair.first;
            VeriPB::Lit unit_lit = pair.second;

            // Add the right implication for the unit clause
            C.clear();
            C.add_literal(unit_lit, 1);
            C.add_RHS(1);
            pl->reification_literal_right_implication(blocking_lit, C, true);

            // Move to the coresets
            pl->move_to_coreset_by_id(-1);

            // Change the objective function to include the blocking variable instead of the unit literal
            LinTermBoolVars<VeriPB::Lit, uint32_t, uint32_t> c_old;
            LinTermBoolVars<VeriPB::Lit, uint32_t, uint32_t> c_new;
            c_old.add_literal(neg(unit_lit), 1);
            c_new.add_literal(neg(blocking_lit), 1);
            pl->write_objective_update_diff(c_old, c_new);
        }
    }

    void ProofConverter::write_new_clauses(const std::vector<cnf::Clause>& new_clauses) {
        VeriPB::Constraint<VeriPB::Lit, uint32_t, uint32_t> C;
        for (int i = 0; i < new_clauses.size(); i++) {
            const cnf::Clause& clause = new_clauses[i];

            // Add the new blocking variable
            VeriPB::Var var = var_mgr.new_variable_only_in_proof();
            VeriPB::Lit lit = create_literal(var, false);
            blocking_vars[clause] = lit;

            if (clause.is_tautology()) {
                tautologies[lit] = pl->redundance_based_strengthening_unit_clause(lit);
                pl->store_reified_constraint_left_implication(var, tautologies[lit]);
                continue;
            }

            // Add the new clause to the proof logger
            clause_to_constraint(clause, C);
            pl->reification_literal_right_implication(lit, C, true);
            pl->reification_literal_left_implication(lit, C, true);
        }
    }

    // TODO: Split this function into a smaller function that assembles two clauses into one contraint
    void ProofConverter::assemble_proof(
        VeriPB::constraintid claim_1, VeriPB::constraintid claim_2, VeriPB::constraintid claim_3, VeriPB::constraintid claim_4,
        const cnf::Clause &clause_1, const cnf::Clause &clause_2,
        const std::vector<cnf::Clause> &new_clauses
    ) {
        // s1 + s2 >= s3 + s4 + ... + s_n
        Constraint<VeriPB::Lit, uint32_t, uint32_t> C;
        C.add_literal(neg(blocking_vars[clause_1]), 1);
        C.add_literal(neg(blocking_vars[clause_2]), 1);

        for (auto& clause : new_clauses) {
            C.add_literal(blocking_vars[clause], 1);
        }
        C.add_RHS(new_clauses.size());

        CuttingPlanesDerivation cpder(pl, false);
        VeriPB::constraintid cn = proof_by_contradiction(claim_1, claim_2, C);
        pl->move_to_coreset_by_id(-1);

        // s1 + s2 <= s3 + s4 + ... + s_n
        C.clear();
        C.add_literal(blocking_vars[clause_1], 1);
        C.add_literal(blocking_vars[clause_2], 1);
        for (auto& clause : new_clauses) {
            C.add_literal(neg(blocking_vars[clause]), 1);
        }
        C.add_RHS(2);

        cn = proof_by_contradiction(claim_3, claim_4, C);
        pl->move_to_coreset_by_id(-1);
    }

    void ProofConverter::assemble_proof(
        VeriPB::constraintid claim_1, VeriPB::constraintid claim_2, VeriPB::constraintid claim_3,
        const cnf::Clause &clause_1, const cnf::Clause &clause_2,
        const std::vector<cnf::Clause> &new_clauses
    ) {
        // s1 + s2 >= s3 + s4 + ... + s_n
        Constraint<VeriPB::Lit, uint32_t, uint32_t> C;
        if (!clause_1.is_hard_clause()) {
            C.add_literal(neg(blocking_vars[clause_1]), 1);
        }
        
        if (!clause_2.is_hard_clause()) {
            C.add_literal(neg(blocking_vars[clause_2]), 1);
        }

        for (auto& clause : new_clauses) {
            C.add_literal(blocking_vars[clause], 1);
        }
        C.add_RHS(new_clauses.size());

        CuttingPlanesDerivation cpder(pl, false);
        VeriPB::constraintid cn = proof_by_contradiction(claim_1, claim_2, C);
        pl->move_to_coreset_by_id(-1);

        // s1 + s2 <= s3 + s4 + ... + s_n
        pl->move_to_coreset_by_id(claim_3);
    }

    // TODO: Remove the clauses that are replaced from the blocking_vars map
    void ProofConverter::change_objective(const cnf::Clause &clause_1, const cnf::Clause &clause_2, const std::vector<cnf::Clause> &new_clauses) {
        LinTermBoolVars<VeriPB::Lit, uint32_t, uint32_t> c_old;
        LinTermBoolVars<VeriPB::Lit, uint32_t, uint32_t> c_new;

        if (!clause_1.is_hard_clause()) {
            c_old.add_literal(neg(blocking_vars[clause_1]), 1);
        }

        if (!clause_2.is_hard_clause()) {
            c_old.add_literal(neg(blocking_vars[clause_2]), 1);
        }

        for (auto& clause : new_clauses) {
            c_new.add_literal(neg(blocking_vars[clause]), 1); // TODO: Don't add tautologies
        }
        pl->write_objective_update_diff(c_old, c_new);
    }

    void ProofConverter::change_objective(const cnf::Clause &clause_1, const cnf::Clause &clause_2, const cnf::Clause &clause_3) {
        LinTermBoolVars<VeriPB::Lit, uint32_t, uint32_t> c_old;
        LinTermBoolVars<VeriPB::Lit, uint32_t, uint32_t> c_new;

        c_old.add_literal(neg(blocking_vars[clause_1]), 1);

        c_new.add_literal(neg(blocking_vars[clause_2]), 1);
        c_new.add_literal(neg(blocking_vars[clause_3]), 1);
        pl->write_objective_update_diff(c_old, c_new);
    }

    void ProofConverter::clause_to_constraint(const cnf::Clause &clause, VeriPB::Constraint<VeriPB::Lit, uint32_t, uint32_t> &C) {
        C.clear();
        C.add_RHS(1);

        for (const auto& literal : clause.get_literals_set()) {
            uint32_t var = std::abs(literal);
            VeriPB::Lit lit = vars[var];

            if (literal < 0) {
                C.add_literal(neg(lit), 1);
            } else {
                C.add_literal(lit, 1);
            }
        }
    }

    void ProofConverter::clause_to_neg_constraint(const cnf::Clause &clause, VeriPB::Constraint<VeriPB::Lit, uint32_t, uint32_t> &C) {
        C.clear();
        C.add_RHS(clause.get_literals_set().size());

        for (const auto& literal : clause.get_literals_set()) {
            uint32_t var = std::abs(literal);
            VeriPB::Lit lit = vars[var];

            if (literal < 0) {
                C.add_literal(lit, 1);
            } else {
                C.add_literal(neg(lit), 1);
            }
        }
    }

    VeriPB::constraintid ProofConverter::proof_by_contradiction(VeriPB::constraintid claim_1, VeriPB::constraintid claim_2, VeriPB::Constraint<VeriPB::Lit, uint32_t, uint32_t> &C) {
        
        pl->write_comment("__Proof by contradiction__");
        CuttingPlanesDerivation cpder(pl, false);
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

        return pl->end_proof_by_contradiction();
    }
}