#include "SplitClaim.h"

namespace converter {

    SplitClaim::SplitClaim(
        const std::vector<std::pair<VeriPB::Lit, cnf::Clause>> &clauses,
        const std::function<bool(VeriPB::Lit)> &tautology_predicate,
        const std::function<bool(VeriPB::Lit)> &hard_clause_predicate,
        bool negated_pivot
    ) : Claim(negated_pivot, tautology_predicate, hard_clause_predicate) {
        if (clauses.size() != 3) {
            throw std::runtime_error("SplitClaim must have exactly three clauses.");
        }

        original_blocking_lit = clauses[0].first;
        new_blocking_lit_1 = clauses[1].first;
        new_blocking_lit_2 = clauses[2].first;
    }

    VeriPB::constraintid SplitClaim::write(VeriPB::Prooflogger &pl) {
        VeriPB::CuttingPlanesDerivation cpder(&pl, false);

        VeriPB::constraintid original_constraint;
        VeriPB::constraintid new_constraint_1;
        VeriPB::constraintid new_constraint_2;

        if (!is_negated_pivot()) {
            original_constraint = pl.get_reified_constraint_right_implication(variable(original_blocking_lit));
            new_constraint_1 = pl.get_reified_constraint_left_implication(variable(new_blocking_lit_1));
            new_constraint_2 = pl.get_reified_constraint_left_implication(variable(new_blocking_lit_2));
        } else {
            original_constraint = pl.get_reified_constraint_left_implication(variable(original_blocking_lit));
            new_constraint_1 = pl.get_reified_constraint_right_implication(variable(new_blocking_lit_1));
            new_constraint_2 = pl.get_reified_constraint_right_implication(variable(new_blocking_lit_2));
        }

        cpder.start_from_constraint(original_constraint);
        cpder.add_constraint(new_constraint_1);
        cpder.saturate();
        VeriPB::constraintid cxn_1 = cpder.end();

        cpder.start_from_constraint(original_constraint);
        cpder.add_constraint(new_constraint_2);
        cpder.saturate();
        VeriPB::constraintid cxn_2 = cpder.end();

        cpder.start_from_constraint(cxn_1);
        cpder.add_constraint(cxn_2);
        cpder.saturate();
        if (!is_negated_pivot()) cpder.divide(2);
        return cpder.end();
    }
}