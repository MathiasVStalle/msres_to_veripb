#include <iostream>

#include "../lib/core/VeriPbSolverTypes.h"
#include "../lib/core/MaxSATProoflogger.h"

void exampleProof() {

    // The given formulas's
    // 1. 1 s1 1 x1 1 a1 1 a2 >= 1;
    // 2. 1 s2 1 ~x1 1 b1 1 b2 >= 1;

    VeriPB::Var x1 = {.v=1, .only_known_in_proof=false}, 
                a1 = {.v=2, .only_known_in_proof=false}, 
                a2 = {.v=3, .only_known_in_proof=false};

    VeriPB::Constraint<VeriPB::Lit, uint32_t, uint32_t> C1;
    C1.add_literal(VeriPB::create_literal(x1, true), 1);
    C1.add_literal(VeriPB::create_literal(a1, true), 1);
    C1.add_literal(VeriPB::create_literal(a2, true), 1);
    C1.add_RHS(3);

    VeriPB::VarManagerWithVarRewriting varMgr;
    varMgr.set_number_original_variables(3);

    VeriPB::MaxSATProoflogger<VeriPB::Lit, uint32_t, uint32_t> vPL("testproof.pbp", &varMgr);
    vPL.add_blocking_literal(VeriPB::create_literal(x1, true), 1);
    
    vPL.set_n_orig_constraints(0);
    vPL.write_proof_header();

    vPL.add_objective_literal(VeriPB::create_literal(x1, true), 1);

    vPL.unchecked_assumption(C1);
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