#include "ProofConstraint.h"

namespace converter {

    ProofConstraint::ProofConstraint(
        const VeriPB::Var &blocking_variable, 
        const bool tautology,
        const std::unordered_set<VeriPB::Lit, LitHash, LitEqual> &duplicate_literals,
        const std::span<const VeriPB::Var> &active_vars,
        const std::span<const VeriPB::Var> &inactive_vars
    ) : blocking_var(blocking_variable), tautology(tautology), duplicate_literals(duplicate_literals), active_vars(active_vars), inactive_vars(inactive_vars) {}

    VeriPB::Var ProofConstraint::get_blocking_variable() {
        return blocking_var;
    }

    bool ProofConstraint::is_tautology() {
        return tautology;
    }

    const std::unordered_set<VeriPB::Lit, LitHash, LitEqual>& ProofConstraint::get_duplicate_literals() const {
        return duplicate_literals;
    }

    std::span<const VeriPB::Var> ProofConstraint::get_active_vars() {
        return active_vars;
    }

    std::span<const VeriPB::Var> ProofConstraint::get_inactive_vars() {
        return inactive_vars;
    }
}