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
