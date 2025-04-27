#include "ProofConvertor.h"

using namespace VeriPB;

namespace convertor {
    VeriPB::constraintid ProofConvertor::claim_3(
        uint32_t clause_id_1,
        uint32_t clause_id_2,
        const cnf::ResRule& rule,
        const std::vector<cnf::Clause>& new_clauses
    ) {
        VeriPB::CuttingPlanesDerivation cpder(this->pl, false);

        std::unordered_set<int32_t> literals_set_clause_1 = rule.getClause1().getLiterals();
        std::unordered_set<int32_t> literals_set_clause_2 = rule.getClause2().getLiterals();

        int32_t rhs = new_clauses.size() - 2;

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
        Lit s3 = this->blocking_vars[blocking_vars.size() - (new_clauses.size() - 1)];


        // Step 1
        cpder.start_from_constraint(pl->get_reified_constraint_right_implication(variable(s3)));
        for (int i = 0; i < literals_clause_1.size(); i++) {
            Lit sn = blocking_vars[blocking_vars.size() - i];
            cpder.add_constraint(pl->get_reified_constraint_right_implication(variable(sn)));
            cpder.saturate();
        }
        constraintid cxn_1 = cpder.end();

        // Step 2
        cpder.start_from_constraint(pl->get_reified_constraint_left_implication(variable(s2)));
        cpder.weaken(variable(x));
        cpder.saturate();
        cpder.end();

        cpder.start_from_literal_axiom(neg(x));
        cpder.add_literal_axiom(s3);
        for (int i = 0; i < literals_clause_1.size(); i++) {
            Lit sn = blocking_vars[blocking_vars.size() - i];
            cpder.add_literal_axiom(sn);
        }
        cpder.multiply((int32_t) literals_clause_2.size());
        cpder.add_constraint(-1);
        constraintid cxn_2 = cpder.end();
        pl->write_comment("Step 2");
        pl->write_comment("");


        // Case Splitting
        Constraint<VeriPB::Lit, uint32_t, uint32_t> C;
        C.add_literal(neg(x), 1);
        C.add_literal(s2, 1);
        C.add_literal(s3, 1);
        for (int i = 0; i < literals_clause_1.size(); i++) {
            C.add_literal(blocking_vars[blocking_vars.size() - i], 1);
        }
        C.add_RHS(1);

        pl->start_proof_by_contradiction(C);
        
        cpder.start_from_constraint(-1);
        cpder.multiply((int32_t)literals_clause_2.size());
        cpder.add_constraint(cxn_2);
        cpder.saturate();
        cpder.end();

        cpder.start_from_constraint(-2);
        cpder.add_constraint(cxn_1);
        cpder.saturate();
        cpder.end();

        cpder.start_from_constraint(-2);
        cpder.add_constraint(-1);
        cpder.end();

        constraintid cxnneg = pl->end_proof_by_contradiction();

        cpder.start_from_constraint(pl->get_reified_constraint_left_implication(variable(s1)));
        for (int i = 0; i < literals_clause_1.size(); i++) {
            cpder.weaken(variable(vars[std::abs(literals_clause_1[i])]));
        }
        cpder.saturate();
        cpder.end();

        cpder.start_from_constraint(-2);
        cpder.add_constraint(-1);
        for (int i = 0; i < literals_clause_2.size(); i++) {
            cpder.add_literal_axiom(blocking_vars[blocking_vars.size() - literals_clause_1.size() - i]);
        }
        pl->write_comment("Step 3");
        pl->write_comment("");
        return cpder.end();
    }
}