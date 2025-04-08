#include "../lib/core/MaxSATProoflogger.hpp"

template class VeriPB::LinTermBoolVars<VeriPB::Lit, unsigned int, unsigned int>;
template class VeriPB::Constraint<VeriPB::Lit, unsigned int, unsigned int>;
template class VeriPB::MaxSATProoflogger<VeriPB::Lit, unsigned int, unsigned int>;

template long VeriPB::Prooflogger::unchecked_assumption<VeriPB::Constraint<VeriPB::Lit, unsigned int, unsigned int>>(VeriPB::Constraint<VeriPB::Lit, unsigned int, unsigned int> const &);