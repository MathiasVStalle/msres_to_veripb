#ifndef MaxSATProoflogger_h
#define MaxSATProoflogger_h

#include "ProofloggerOptimization.h"

// PL library

namespace VeriPB{

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
class MaxSATProoflogger : public ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>
{
private: 
    std::vector<constraintid> core_lower_bounds;
    std::vector<ObjCoeff> core_weights;
    std::map<VeriPB::VarIdx, int> counting_var_to_core_idx;

    VarManagerWithVarRewriting* _varMgrRewrite; 

public:
    /// @brief Constructor of the MaxSAT prooflogger object.
    MaxSATProoflogger(const std::string& prooffile, VarManagerWithVarRewriting* varMgr);
    MaxSATProoflogger(const std::string& prooffile, VarManagerWithVarRewriting* varMgr, int n_orig_constraints, bool keep_original_formula=false, bool comments=true);
    MaxSATProoflogger(std::ostream* proof, VarManagerWithVarRewriting* varMgr);
    MaxSATProoflogger(std::ostream* proof, VarManagerWithVarRewriting* varMgr, int n_orig_constraints, bool keep_original_formula=false, bool comments=true);
    ~MaxSATProoflogger();

    //=============================================================================================
    // Variable manager needs to allow for rewriting.
    void set_variable_manager(VarManager* varMgr) = delete;
    void set_variable_manager(VarManagerWithVarRewriting* varMgr);
    
    // ------------- Parsing MaxSAT instances -------------
    template <class TLit>
    void add_blocking_literal(TLit lit, constraintid wcnflinenumber);

    constraintid add_unit_clause_blocking_literal(ObjLit var, constraintid wcnflinenumber, ObjLit unitclause, ObjCoeff weight_softclause, bool rewrite_objective = false);
    
    void derive_blocking_literal_value_by_redundance(constraintid wcnflinenumber, bool value);

    // Functions for proof logging OLL:

    /// @brief Add the lower bound on the input variables of a core to the database of lower bounds for the collected cores. This functions is intended to add the lower bound up to the counter variable with index 2.
    /// @tparam TVar Type of the boolean variables.
    /// @param lazy_var Counter variable (output of the totalizer) with index 2.
    /// @param core_id Constraint ID of core or pseudo-Boolean definition for counter variable with index 1 in the direction counter variable implies constraint.
    /// @param pb_definition_id Constraint ID of pseudo-Boolean definition for counter variable with index 2 in the direction counter variable implies constraint.
    /// @param weight Weight of the core.
    /// @return Constraint ID of the lower bound.
    template <class TVar>
    constraintid add_core_lower_bound(const TVar &lazy_var, constraintid core_id, constraintid pb_definition_id, ObjCoeff weight);

    /// @brief Update the lower bound on the input literals with the definition for the new counting variable for an index `bound`.
    /// @tparam TVar Type of the boolean variables.
    /// @param old_lazy_var Counting variable (output of the totalizer) with index `bound - 1`
    /// @param new_lazy_var Counting variable (output of the totalizer) with index `bound`
    /// @param pb_definition_id Constraint ID of pseudo-Boolean definition for counter variable with index `bound` in the direction counter variable implies constraint.
    /// @param bound The index of the new counter variable.
    /// @return Constraint ID of the updated lower bound on the core.
    template <class TVar>
    constraintid update_core_lower_bound(const TVar &old_lazy_var, const TVar &new_lazy_var, constraintid pb_definition_id, ObjCoeff bound);

    /// @brief Derive the objective reformulation constraint. This constraint says that the original objective is lower bounded by the reformulated objective.
    ///
    /// This is done by adding the lower bounds on the collected cores multiplied by the weight of the cores to the base objective reformulation constraint.
    ///
    /// @param base_reform_id Constraint ID of the base objective reformulation constraint. This is the constraint that says that the original objective is lower bounded by the reformulated objective after reformulating with the fixed lower bounds like at-most-one constraints and unit cores.
    /// @return Constraint ID of the objective reformulation constraint.
    constraintid derive_objective_reformulation_constraint(constraintid base_reform_id);

    /// @brief Proof log the objective reformulation lower bound for arguing about the gap between the lower bound and the upper bound.
    ///
    /// This is done in the following way:
    /// (1) Start with the base objective reformulation constraint that says that the original objective is lower bounded by the reformulated objective after reformulating with the fixed lower bounds like at-most-one constraints and unit cores.
    /// (2) Add the lower bounds for all the cores we collected multiplied by the weight of the cores.
    /// (3) Add the model improving constraint to derive the desired constraint.
    ///
    /// @param base_reform_id Constraint ID of the base objective reformulation constraint. This is the constraint that says that the original objective is lower bounded by the reformulated objective after reformulating with the fixed lower bounds like at-most-one constraints and unit cores.
    /// @param model_improve_id Constraint ID of the model improving constraint.
    /// @return Constraint ID of the constraint to argue about the gap between the lower and upper bound. The slack (without any assignment) of this constraint is equal to `upper bound - lower bound`.
    constraintid proof_log_objective_reformulation(constraintid base_reform_id, constraintid model_improve_id);

    /// @brief Add a unit core to the base objective reformulation constraint. This function deletes the old base objective reformulation constraint and returns the new base objective reformulation constraint.
    /// @param base_reform_id Constraint ID of the base objective reformulation constraint. This is the constraint that says that the original objective is lower bounded by the reformulated objective after reformulating with the fixed lower bounds like at-most-one constraints and unit cores.
    /// @param core_id Constraint ID of the core constraint.
    /// @param weight Weight of the core.
    /// @return Constraint ID of the updated base objective reformulation constraint.
    constraintid base_reform_unit_core(constraintid base_reform_id, constraintid core_id, ObjCoeff weight);

    /// @brief Add unprocessed cores to the objective reformulation constraint. This deletes the base objective reformulation constraint and returns the new objective reformulation constraint.
    /// @param base_reform_id Constraint ID of the base objective reformulation constraint. This is the constraint that says that the original objective is lower bounded by the reformulated objective after reformulating with the fixed lower bounds like at-most-one constraints and unit cores.
    /// @param core_ids Constraint IDs of the unprocessed cores.
    /// @param core_weights Weights of the unprocessed cores.
    /// @return Constraint ID of the updated base objective reformulation constraint. This is the constraint that says that the original objective is lower bounded by the reformulated objective after reformulating with the fixed lower bounds like at-most-one constraints and unit cores.
    constraintid reformulate_with_unprocessed_cores(constraintid base_reform_id, std::vector<constraintid> core_ids, std::vector<ObjCoeff> core_weights);

    //=============================================================================================
    // Functions for proof logging at-most-one constraints

    /// @brief Derive the at-most-one constraint as a pseudo-Boolean constraint, which is that the sum of the literals is at least the number of literals - 1.
    /// @tparam TLit Type of the boolean literal.
    /// @param am1_lits Literals of the at-most-one constraint.
    /// @param am1_sign There can be at most one literal in am1_lits have the sign of am1_sign. Default false for intrinsic at most one constraints in MaxSAT.
    /// @return Constraint ID of the at-most-one constraint.
    template <class TSeqLit>
    constraintid derive_at_most_one_constraint(const TSeqLit &am1_lits, const bool am1_sign=false);
    /// @brief Introduce a new variable that represents all variables in the at-most-one constraint being true.
    /// @tparam TLit Type of the boolean literal.
    /// @param am1_lits Literals of the at-most-one constraint.
    /// @param selector_all_lit Selector literal for the at-most-one constraint. This literal will be true if and only if all the literals in `am1_lits` are true.
    /// @return Constraint ID of the constraint required for objective reformulation, which is a lower bound on the literals in the at-most-one constraint.
    template <class TSeqLit, class TLit>
    constraintid introduce_at_most_one_selector(const TSeqLit &am1_lits, const TLit &selector_all_lit);

    /// @brief Proof log the objective reformulation with intrinsic at-most-one constraints. This function requires that the at-most-one constraint was detected by propagation. This function deletes the old base objective reformulation constraint and returns the new base objective reformulation constraint.
    /// @tparam TLit Type of the boolean literal.
    /// @param base_reform_id Constraint ID of the base objective reformulation constraint. This is the constraint that says that the original objective is lower bounded by the reformulated objective after reformulating with the fixed lower bounds like at-most-one constraints and unit cores.
    /// @param am1_lits Literals of the at-most-one constraint.
    /// @param selector_all_lit Selector literal for the at-most-one constraint. This literal will be true if and only if all the literals in `am1_lits` are true.
    /// @param weight Weight of the at most one constraint.
    /// @return Constraint ID of the updated base objective reformulation constraint. This is the constraint that says that the original objective is lower bounded by the reformulated objective after reformulating with the fixed lower bounds like at-most-one constraints and unit cores.
    template <class TSeqLit, class TLit>
    constraintid proof_log_at_most_one(constraintid base_reform_id, const TSeqLit &am1_lits, const TLit &selector_all_lit, ObjCoeff weight);
};
}
#endif