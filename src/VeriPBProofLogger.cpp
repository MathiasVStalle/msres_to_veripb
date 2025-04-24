#include "../lib/VeriPB_Prooflogger/core/MaxSATProoflogger.hpp"



template class VeriPB::LinTermBoolVars<VeriPB::Lit, unsigned int, unsigned int>;
template class VeriPB::Constraint<VeriPB::Lit, unsigned int, unsigned int>;
template class VeriPB::ProofloggerOpt<VeriPB::Lit, unsigned int, unsigned int>;
template class VeriPB::MaxSATProoflogger<VeriPB::Lit, unsigned int, unsigned int>;


template long VeriPB::Prooflogger::unchecked_assumption<VeriPB::Constraint<VeriPB::Lit, unsigned int, unsigned int>>(VeriPB::Constraint<VeriPB::Lit, unsigned int, unsigned int> const &);
template void VeriPB::Prooflogger::store_reified_constraint_right_implication<VeriPB::Var>(VeriPB::Var const&, long const&);
template void VeriPB::Prooflogger::store_reified_constraint_left_implication<VeriPB::Var>(VeriPB::Var const&, long const&);
template long VeriPB::Prooflogger::reification_literal_right_implication<VeriPB::Lit, VeriPB::Constraint<VeriPB::Lit, unsigned int, unsigned int> >(VeriPB::Lit const&, VeriPB::Constraint<VeriPB::Lit, unsigned int, unsigned int> const&, bool);
template long VeriPB::Prooflogger::reification_literal_left_implication<VeriPB::Lit, VeriPB::Constraint<VeriPB::Lit, unsigned int, unsigned int> >(VeriPB::Lit const&, VeriPB::Constraint<VeriPB::Lit, unsigned int, unsigned int> const&, bool);
template long VeriPB::Prooflogger::get_reified_constraint_left_implication<VeriPB::Var>(VeriPB::Var const&);
template long VeriPB::Prooflogger::get_reified_constraint_right_implication<VeriPB::Var>(VeriPB::Var const&);
template void VeriPB::CuttingPlanesDerivation::weaken<VeriPB::Var>(VeriPB::Var const&);
template void VeriPB::CuttingPlanesDerivation::multiply<int>(int const&);
template void VeriPB::CuttingPlanesDerivation::divide<int>(int const&);
template void VeriPB::CuttingPlanesDerivation::start_from_literal_axiom<VeriPB::Lit>(VeriPB::Lit const&);
template void VeriPB::CuttingPlanesDerivation::add_literal_axiom<VeriPB::Lit, unsigned int>(VeriPB::Lit const&, unsigned int const&);
template void VeriPB::CuttingPlanesDerivation::add<unsigned int>(VeriPB::CuttingPlanesDerivation const*, unsigned int const&);
template long VeriPB::Prooflogger::start_proof_by_contradiction<VeriPB::Constraint<VeriPB::Lit, unsigned int, unsigned int> >(VeriPB::Constraint<VeriPB::Lit, unsigned int, unsigned int> const&);