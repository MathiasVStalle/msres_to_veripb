#include "ProofConvertor.h"

using namespace VeriPB;

namespace convertor {
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
            VeriPB::Lit sn = this->blocking_vars[offset - i];
        
            // Take all the unused constraints and weaken them
            cpder.start_from_constraint(this->pl->get_reified_constraint_left_implication(variable(sn)));
            for (int j = 0; j < literals_clause_1.size(); j++) {
                cpder.weaken(variable(this->vars[std::abs(literals_clause_1[j])]));
            }
            for (int j = 0; j < literals_clause_2.size() - i; j++) {
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
        pl->write_comment("Claim 1");
        pl->write_comment("");
        
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
        int32_t RHS = literals_1.size();

        // Initial constraint 
        for (int i = 1; i <= literals_1.size(); i++) {
            Lit sn = blocking_vars[blocking_vars.size() - i + 1];
            cpder.start_from_constraint(pl->get_reified_constraint_left_implication(variable(sn)));

            // Weaken on everything except the last a
            for (int j = 0; j < literals_1.size() - i; j++) {
                cpder.weaken(variable(vars[std::abs(literals_1[j])]));
            }
            for (int j = 0; j < literals_2.size(); j++) {
                cpder.weaken(variable(vars[std::abs(literals_2[j])]));
            }
            cpder.weaken(variable(x));
            cpder.saturate();
            cpder.end();
        }

        // Add the constraints
        cpder.start_from_literal_axiom(neg(s3));
        for (int i = 1; i <= literals_1.size(); i++) {
            cpder.add_constraint(-i);
        }
        constraintid intitial = cpder.end();
        pl->write_comment("Initial constraint");
        pl->write_comment("");


        std::vector<VeriPB::constraintid> subclaims;
        for (int i = 1; i <= literals_1.size(); i++) {
            cpder.start_from_constraint(pl->get_reified_constraint_left_implication(variable(s3)));
            // Weaken on everything except a_i
            for (int j = 0; j < literals_1.size(); j++) {
                if (i == j + 1) continue;
                cpder.weaken(variable(vars[std::abs(literals_1[j])]));
            }
            for (int j = 0; j < literals_2.size(); j++) {
                cpder.weaken(variable(vars[std::abs(literals_2[j])]));
            }
            cpder.saturate();
            cpder.end();

            // Weaken on everything except the last a_n where n <= i
            for (int j = 1; j <= literals_1.size(); j++) {
                if (i == j) continue;

                VeriPB::Lit sn = blocking_vars[blocking_vars.size() - literals_1.size() + j];
                cpder.start_from_constraint(pl->get_reified_constraint_left_implication(variable(sn)));
                cpder.weaken(variable(x));

                uint32_t n = (j <= i) ? j : i;
                for (int k = 0; k < literals_1.size(); k++) {
                    if (n == k + 1) continue;
                    cpder.weaken(variable(vars[std::abs(literals_1[k])]));
                }
                for (int k = 0; k < literals_2.size(); k++) {
                    cpder.weaken(variable(vars[std::abs(literals_2[k])]));
                }
                cpder.saturate();
                cpder.end();
            }

            // Add all the previous constraints
            cpder.start_from_constraint(-1);
            for (int j = 2; j <= literals_1.size(); j++) {
                cpder.add_constraint(-j);
            }

            // Add the missing literal
            Lit sn = blocking_vars[blocking_vars.size() - literals_1.size() + i];
            cpder.add_literal_axiom(neg(sn));

            subclaims.push_back(cpder.end());
            pl->write_comment("Subclaim " + std::to_string(i));
            pl->write_comment("");
        }


        // Proofs by contradiction
        constraintid subclaim_1 = intitial;
        for (int i = literals_1.size() - 1; i >= 0; i--) {
            constraintid subclaim_2 = subclaims[i];

            // Build the constraint
            Constraint<VeriPB::Lit, uint32_t, uint32_t> C;
            C.add_literal(neg(s3), 1);
            for (int j = 0; j < literals_1.size(); j++) {
                C.add_literal(neg(blocking_vars[blocking_vars.size() - j]), 1);
            }
            for (int j = 0; j < i; j++) {
                uint32_t coefficient = (j < i - 1) ? 1 : literals_1.size() - i;
                C.add_literal(vars[std::abs(literals_1[j])], 1);
            }
            C.add_RHS(literals_1.size());


            // Start the proof
            pl->write_comment("Proof by contradiction" + std::to_string(i));
            pl->write_comment("");
            pl->start_proof_by_contradiction(C);

            cpder.start_from_constraint(subclaim_1);
            cpder.add_constraint(-1);
            cpder.saturate();
            cpder.end();

            cpder.start_from_constraint(subclaim_2);
            cpder.add_constraint(-2);
            cpder.saturate();
            cpder.end();

            // Add the previous constraints
            cpder.start_from_constraint(-1);
            cpder.add_constraint(-2);
            cpder.saturate();
            cpder.end();

            pl->end_proof_by_contradiction();

            // Add the missing literal
            if (i != 0) {
                cpder.start_from_constraint(-1);
                cpder.add_literal_axiom(vars[std::abs(literals_1[i - 1])]);
                subclaim_1 = cpder.end();
            } else {
                pl->get_constraint_counter();
            }
        }

        return subclaim_1;
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
            VeriPB::Lit sn = blocking_vars[blocking_vars.size() - i];
            cpder.start_from_constraint(pl->get_reified_constraint_left_implication(variable(sn)));

            cpder.weaken(variable(x));

            for (int j = 1; j < literals_1.size() - i; j++) {
                cpder.weaken(variable(vars[std::abs(literals_1[j])]));
            }
            for (int j = 0; j < literals_2.size(); j++) {
                cpder.weaken(variable(vars[std::abs(literals_2[j])]));
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
        cpder.divide(counter + 1);
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
                if (i == j) continue;
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
                    if (i == k) continue;

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
        C.add_RHS(num_new_clauses - literals_2.size());

        // Proof by constradiction (1 ~x1 1 s2 1 ~s3 1 ~s6 ... 1 ~sn >= m)
        int32_t counter = 0;
        VeriPB::constraintid cxnneg = pl->start_proof_by_contradiction(C);
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
