#ifndef MSRES_FOR_VERIPB_CONVERTER_HASH_H
#define MSRES_FOR_VERIPB_CONVERTER_HASH_H

#include "../cnf/Clause.h"

#include "VeriPbSolverTypes.h"
#include "MaxSATProoflogger.h"

struct LitHash {
    std::size_t operator()(const VeriPB::Lit &lit) const {
        return lit.v.v ^ (lit.negated ? 1 : 0);
    }
};

struct LitEqual {
    bool operator()(const VeriPB::Lit &lit_1, const VeriPB::Lit &lit_2) const {
        return lit_1.v.v == lit_2.v.v && lit_1.negated == lit_2.negated;
    }
};

struct VarHash {
    std::size_t operator()(const VeriPB::Var &var) const {
        return var.v;
    }
};

struct VarEqual {
    bool operator()(const VeriPB::Var &var_1, const VeriPB::Var &var_2) const {
        return var_1.v == var_2.v;
    }
};

struct ClausePtrHash {
    size_t operator()(const cnf::Clause* c) const {
        return std::hash<cnf::Clause>()(*c);
    }
};

struct ClausePtrEqual {
    bool operator()(const cnf::Clause* lhs, const cnf::Clause* rhs) const {
        return *lhs == *rhs;  // Assuming operator== compares content
    }
};

#endif // MSRES_FOR_VERIPB_CONVERTER_HASH_H