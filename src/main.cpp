#include <iostream>

#include "../lib/VeriPB_Prooflogger/core/VeriPbSolverTypes.h"
#include "../lib/VeriPB_Prooflogger/core/MaxSATProoflogger.h"

void exampleProof() {
    VeriPB::Var x1 = {.v=1, .only_known_in_proof=false}, 
                x2 = {.v=2, .only_known_in_proof=false}, 
                x3 = {.v=3, .only_known_in_proof=false},
                x4 = {.v=4, .only_known_in_proof=false};

    VeriPB::Constraint<VeriPB::Lit, uint32_t, uint32_t> C1, C2(true, VeriPB::Comparison::LEQ), C3;
    C1.add_literal(VeriPB::create_literal(x1, false), 3);
    C1.add_literal(VeriPB::create_literal(x2, true), 2);
    C1.add_literal(VeriPB::create_literal(x3, true), 2);
    C1.add_RHS(3);

    C2.add_literal(VeriPB::create_literal(x1, false));
    C2.add_literal(VeriPB::create_literal(x2, false));
    C2.add_literal(VeriPB::create_literal(x3, false));
    C2.add_literal(VeriPB::create_literal(x4, false));
    C2.add_RHS(2);

    C3.add_literal(VeriPB::create_literal(x1, true));
    C3.add_literal(VeriPB::create_literal(x2, true));
    C3.add_RHS(1);

    VeriPB::VarManagerWithVarRewriting varMgr;
    varMgr.set_number_original_variables(4);

    varMgr.store_variable_name(x1, "x1p");

    VeriPB::MaxSATProoflogger<VeriPB::Lit, uint32_t, uint32_t> vPL("testproof.pbp", &varMgr);
    vPL.set_n_orig_constraints(0);
    vPL.write_proof_header();
    vPL.add_objective_literal(VeriPB::create_literal(x1, true), 1);
    vPL.add_objective_literal(VeriPB::create_literal(x2, true), 1);
    vPL.add_objective_literal(VeriPB::create_literal(x3, true), 1);
    vPL.add_objective_literal(VeriPB::create_literal(x4, true), 1);
    vPL.unchecked_assumption(C1);
    vPL.unchecked_assumption(C2);
    vPL.unchecked_assumption(C3);
    vPL.write_conclusion_NONE();

    // Flush the proof logger]
    vPL.flush_proof();
}

void exampleProof2(){

    // Setting up Proof logging library
    VeriPB::VarManagerWithVarRewriting varMgr;
    VeriPB::ProofloggerOpt<VeriPB::Lit, uint32_t, uint32_t> pl("proofTest2.pbp", &varMgr);
    pl.set_comments(true);

    // Setting up the context, this has to be done by the parser later.
    uint32_t varcounter;
    VeriPB::Var varx1 {.v=++varcounter, .only_known_in_proof=false},
                vara1 {.v=++varcounter, .only_known_in_proof=false},
                vara2 {.v=++varcounter, .only_known_in_proof=false},
                varb1 {.v=++varcounter, .only_known_in_proof=false},
                varb2 {.v=++varcounter, .only_known_in_proof=false};
    
    VeriPB::Var vars1 = varMgr.new_variable_only_in_proof(),
                vars2 = varMgr.new_variable_only_in_proof();

    VeriPB::Lit x1 {.v=varx1, .negated=false},
                a1 {.v=vara1, .negated=false},
                a2 {.v=vara2, .negated=false},
                b1 {.v=varb1, .negated=false},
                b2 {.v=varb2, .negated=false},
                s1 {.v=vars1, .negated=false},
                s2 {.v=vars2, .negated=false};

    varMgr.store_variable_name(variable(x1), "x1");
    varMgr.store_variable_name(variable(a1), "a1");
    varMgr.store_variable_name(variable(a2), "a2");
    varMgr.store_variable_name(variable(b1), "b1");
    varMgr.store_variable_name(variable(b2), "b2");

    varMgr.store_variable_name(variable(s1), "s1");
    varMgr.store_variable_name(variable(s2), "s2");

    // Schrijven van proof header
    varMgr.set_number_original_variables(5);
    pl.write_proof_header();
    pl.set_n_orig_constraints(2); 

    // Opslaan van reif van originele clauses
    pl.store_reified_constraint_right_implication(variable(s1), 1);
    pl.store_reified_constraint_right_implication(variable(s2), 2);

    // Hergebruiken van constraint, niet perce nodig.
    VeriPB::Constraint<VeriPB::Lit, uint32_t, uint32_t> C;

    // Afleiden Reificatie clauses andere richting
    pl.write_comment("Afleiden Reificatie clauses andere richting");
    C.add_literal(x1, 1);
    C.add_literal(a1, 1);
    C.add_literal(a2, 1);
    C.add_RHS(1);

    pl.reification_literal_left_implication(neg(s1), C, true);

    C.clear();
    C.add_literal(neg(x1), 1);
    C.add_literal(b1, 1);
    C.add_literal(b2, 1);
    C.add_RHS(1);
    pl.reification_literal_left_implication(neg(s2), C, true);
    
    // Definitie nieuwe variabelen
    
    VeriPB::Var vars3 = varMgr.new_variable_only_in_proof();
    varMgr.store_variable_name(vars3, "s3"); // Not necessary in final implementation.
    VeriPB::Lit s3 = create_literal(vars3, false);
    C.clear();
    C.add_literal(a1, 1);
    C.add_literal(a2, 1);
    C.add_literal(b1, 1);
    C.add_literal(b2, 1);
    C.add_RHS(1);
    pl.reification_literal_left_implication(neg(s3), C, true);
    pl.reification_literal_right_implication(neg(s3), C, true);

    VeriPB::Var vars4 = varMgr.new_variable_only_in_proof();
    varMgr.store_variable_name(vars4, "s4"); // Not necessary in final implementation.
    VeriPB::Lit s4 = create_literal(vars4, false);
    C.clear();
    C.add_literal(x1, 1);
    C.add_literal(a1, 1);
    C.add_literal(a2, 1);
    C.add_literal(neg(b1), 1);
    C.add_RHS(1);
    pl.reification_literal_left_implication(neg(s4), C, true);
    pl.reification_literal_right_implication(neg(s4), C, true);

    VeriPB::Var vars5 = varMgr.new_variable_only_in_proof();
    varMgr.store_variable_name(vars5, "s5"); // Not necessary in final implementation.
    VeriPB::Lit s5 = create_literal(vars5, false);
    C.clear();
    C.add_literal(x1, 1);
    C.add_literal(a1, 1);
    C.add_literal(a2, 1);
    C.add_literal(b1, 1);
    C.add_literal(neg(b2), 1);
    C.add_RHS(1);
    pl.reification_literal_left_implication(neg(s5), C, true);
    pl.reification_literal_right_implication(neg(s5), C, true);

    VeriPB::Var vars6 = varMgr.new_variable_only_in_proof();
    varMgr.store_variable_name(vars6, "s6"); // Not necessary in final implementation.
    VeriPB::Lit s6 = create_literal(vars6, false);
    C.clear();
    C.add_literal(x1, 1);
    C.add_literal(neg(a1), 1);
    C.add_literal(b1, 1);
    C.add_literal(b2, 1);
    C.add_RHS(1);
    pl.reification_literal_left_implication(neg(s6), C, true);
    pl.reification_literal_right_implication(neg(s6), C, true);

    VeriPB::Var vars7 = varMgr.new_variable_only_in_proof();
    varMgr.store_variable_name(vars7, "s7"); // Not necessary in final implementation.
    VeriPB::Lit s7 = create_literal(vars7, false);
    C.clear();
    C.add_literal(x1, 1);
    C.add_literal(a1, 1);
    C.add_literal(neg(a2), 1);
    C.add_literal(b1, 1);
    C.add_literal(b2, 1);
    C.add_RHS(1);
    pl.reification_literal_left_implication(neg(s7), C, true);
    pl.reification_literal_right_implication(neg(s7), C, true);

    VeriPB::CuttingPlanesDerivation cpder(&pl, true);
    cpder.start_from_constraint(pl.get_reified_constraint_left_implication(variable(s3)));
    cpder.weaken(variable(a1));
    cpder.weaken(variable(b1));
    cpder.weaken(variable(b2));
    cpder.saturate();
    VeriPB::constraintid cxn_bladibla = cpder.end();

    // Unchecked assumptions for proof by contradiction
    VeriPB::constraintid subsubclaim_11, subsubclaim_12, subsubclaim_13;

    // Proving 1 ~x1 1 s2 1 ~s3 1 ~s6 1 ~s7 >= 3  by contradiction
    C.clear();
    C.add_literal(neg(x1), 1);
    C.add_literal(s2, 1);
    C.add_literal(neg(s3), 1);
    C.add_literal(neg(s6), 1);
    C.add_literal(neg(s7), 1);
    C.add_RHS(3);

    VeriPB::constraintid cxnneg = pl.start_proof_by_contradiction(C);
    cpder.start_from_constraint(cxnneg);
    cpder.add_constraint(subsubclaim_11);
    cpder.saturate();
    cpder.end();
    cpder.start_from_constraint(cxnneg);
    cpder.add_constraint(subsubclaim_12);
    cpder.saturate();
    cpder.end();
    cpder.start_from_constraint(cxnneg);
    cpder.add_constraint(subsubclaim_13);
    cpder.saturate();
    cpder.end();
    cpder.start_from_constraint(-1);
    cpder.add_constraint(-2);
    cpder.add_constraint(-3);
    cpder.saturate();
    cpder.end();
    VeriPB::constraintid bladibla = pl.end_proof_by_contradiction();

    pl.flush_proof();
}


int main() {
    std::cout << "Started" << std::endl;
    // exampleProof();
    exampleProof2();
    std::cout << "Ended" << std::endl;
    return 0;
}