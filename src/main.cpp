#include <iostream>

#include "../lib/core/VeriPbSolverTypes.h"
#include "../lib/core/MaxSATProoflogger.h"

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

    VeriPB::VarManager varMgr;
    varMgr.set_number_original_variables(4);

    VeriPB::ProofloggerOpt<VeriPB::Lit, uint32_t, uint32_t> vPL("testproof.pbp", &varMgr);
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

int main() {
    std::cout << "Started" << std::endl;
    exampleProof();
    std::cout << "Ended" << std::endl;
    return 0;
}