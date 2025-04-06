#include "MaxSATProoflogger.h"
#include "ProofloggerOptimization.hpp"

// PL Library

//=================================================================================================
// Constructor

// TODO

//=================================================================================================
// MaxSAT to PBO translation
namespace VeriPB{

// ------------- Constructor -------------
template <typename ObjLit, typename ObjCoeff, typename ObjConst>
MaxSATProoflogger<ObjLit, ObjCoeff, ObjConst>::MaxSATProoflogger(const std::string& prooffile, VarManagerWithVarRewriting* varMgr) :
    proof(new std::ofstream(prooffile)), 
    _proofOwned(true),
    _varMgr(varMgr), 
    _varMgrRewrite(varMgr),
    _keep_original_formula(false), 
    _n_orig_constraints(0), 
    _constraint_counter(0), 
    _found_solution(false), 
    _comments(true),
    _cpder(new CuttingPlanesDerivation(this, true))
{}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
MaxSATProoflogger<ObjLit, ObjCoeff, ObjConst>::MaxSATProoflogger(const std::string& prooffile, VarManagerWithVarRewriting* varMgr, int n_orig_constraints, bool keep_original_formula=false, bool comments=true) :
    proof(new std::ofstream(prooffile)), 
    _proofOwned(true),
    _varMgr(varMgr),  
    _varMgrRewrite(varMgr),
    _keep_original_formula(keep_original_formula), 
    _n_orig_constraints(n_orig_constraints), 
    _constraint_counter(n_orig_constraints), 
    _found_solution(false), 
    _comments(comments),
    _cpder(new CuttingPlanesDerivation(this, true))
{}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
MaxSATProoflogger<ObjLit, ObjCoeff, ObjConst>::MaxSATProoflogger(std::ostream* proof, VarManagerWithVarRewriting* varMgr) :
    proof(proof),
    _proofOwned(false),
    _varMgr(varMgr),  
    _varMgrRewrite(varMgr),
    _keep_original_formula(false), 
    _n_orig_constraints(0), 
    _constraint_counter(0), 
    _found_solution(false), 
    _comments(true),
    _cpder(new CuttingPlanesDerivation(this, true))
{}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
MaxSATProoflogger<ObjLit, ObjCoeff, ObjConst>::MaxSATProoflogger(std::ostream* proof, VarManagerWithVarRewriting* varMgr, int n_orig_constraints, bool keep_original_formula=false, bool comments=true) :
    proof(proof),
    _proofOwned(false),
    _varMgr(varMgr),  
    _varMgrRewrite(varMgr),
    _keep_original_formula(keep_original_formula), 
    _n_orig_constraints(n_orig_constraints), 
    _constraint_counter(n_orig_constraints), 
    _found_solution(false), 
    _comments(comments),
    _cpder(new CuttingPlanesDerivation(this, true))
{}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
MaxSATProoflogger<ObjLit, ObjCoeff, ObjConst>::~MaxSATProoflogger(){
    if(_proofOwned)
        delete proof;
    delete _cpder;
}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
void MaxSATProoflogger<ObjLit, ObjCoeff, ObjConst>::set_variable_manager(VarManagerWithVarRewriting* varMgr){
    _varMgr = _varMgrRewrite = varMgr;
}

// ------------- Parsing MaxSAT instances -------------

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
template <class TLit>
void VeriPB::MaxSATProoflogger<ObjLit, ObjCoeff, ObjConst>::add_blocking_literal(TLit lit, constraintid cxn_id){
    _varMgrRewrite->store_variable_name(toVeriPbVar(variable(lit)), "_b" + std::to_string(cxn_id));

    // VeriPB expects the blocking literal added to the clauses to be a negated blocking variable. 
    // If the solver adds a non-negated blocking literal to clauses, all literals over this variable have to be negated throughout the proof.
    if(!is_negated(lit))
        _varMgrRewrite->store_rewrite_var_by_lit(toVeriPbVar(variable(lit)), toVeriPbLit(neg(lit)));
}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
constraintid VeriPB::MaxSATProoflogger<ObjLit, ObjCoeff, ObjConst>::add_unit_clause_blocking_literal(ObjLit blocking_lit, constraintid cxn_id, ObjLit unitclause, ObjCoeff weight_softclause, bool rewrite_objective){
    _varMgrRewrite->store_variable_name(toVeriPbVar(variable(vblocking_lit)), "_bu" + std::to_string(cxn_id));


    Constraint<ObjLit, ObjCoeff, ObjConst> cls(true,1); // Create clause
    cls.add_literal(unitclause);
    cls.add_literal(blocking_lit);    

    substitution witness = get_new_substitution();
    add_boolean_assignment(witness, variable(vblocking_lit), !is_negated(vblocking_lit));
    
    constraintid c_id = redundance_based_strengthening(cls, witness);
    
    if(rewrite_objective){
        bool origf = _keep_original_formula;
        _keep_original_formula = false;

        move_to_coreset(-1); // Is necessary for the objective update rule.

        cls.clear(true, 1);
        cls.add_literal(neg(unitclause));
        cls.add_literal(neg(blocking_lit));        

        witness = get_new_substitution();
        add_boolean_assignment(witness, variable(blocking_lit), is_negated(blocking_lit));
        
        constraintid c_inverse_id = redundance_based_strengthening(cls, witness);
        move_to_coreset(-1);

        add_objective_literal(blocking_lit, weight_softclause);
        ObjLit neglit = neg(unitclause);
        remove_objective_literal(neglit);
        
        LinTermBoolVars<ObjLit, ObjCoeff, ObjConst> linTermOld, linTermNew;
        linTermOld.add_literal(neg(unitclause), weight_softclause);
        linTermNew.add_literal(blocking_lit, weight_softclause);

        write_objective_update_diff(linTermOld, linTermNew);

        if(get_model_improving_constraint() != 0){ 
            _cpder->start_from_constraint(get_model_improving_constraint());
            _cpder->add_constraint(-1, weight_softclause);
            update_model_improving_constraint(_cpder->end());
        }

        // TODO: check this deletion.
        delete_constraint(cls, witness);
        _keep_original_formula = origf; 
    }

    
    
    return c_id;
}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
void MaxSATProoflogger<ObjLit, ObjCoeff, ObjConst>::derive_blocking_literal_value_by_redundance(constraintid wcnflinenumber, bool value){
    // VeriPB adds blocking literal 1 ~_b2 to a soft clause. Hence, if value == 1, we prove the clause 1 ~_b2 >= 1, otherwise, we prove the clause 1 _b2 >= 1. 
    *(proof) << "red 1 " << (value ? "~" : "") << "_b" << std::to_string(wcnflinenumber) << " >= 1; "  << "_b" << std::to_string(wcnflinenumber) << " -> " << (value ? 0 : 1) << "\n";
    ++_constraint_counter;
    move_to_coreset(-1);
}
//=================================================================================================
// Objective reformulation

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
template <class TVar>
constraintid MaxSATProoflogger<ObjLit, ObjCoeff, ObjConst>::add_core_lower_bound(
    const TVar &lazy_var, constraintid core_id, constraintid pb_definition_id,
    ObjCoeff weight)
{
    _cpder->start_from_constraint(core_id);
    _cpder->add_constraint(pb_definition_id);
    _cpder->divide(2);
    constraintid lower_bound_id = _cpder->end();
    counting_var_to_core_idx[varidx(toVeriPbVar(lazy_var))] = core_lower_bounds.size();
    core_lower_bounds.push_back(lower_bound_id);
    core_weights.push_back(weight);
    return lower_bound_id;
}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
template <class TVar>
constraintid MaxSATProoflogger<ObjLit, ObjCoeff, ObjConst>::update_core_lower_bound(const TVar &old_lazy_var, const TVar &new_lazy_var, constraintid pb_definition_id, ObjCoeff bound)
{
    int core_idx = counting_var_to_core_idx[varidx(toVeriPbVar(old_lazy_var))];

    // Update map
    counting_var_to_core_idx[varidx(toVeriPbVar(new_lazy_var))] = core_idx;

    // Update lower bound
    _cpder->start_from_constraint(core_lower_bounds[core_idx]);
    _cpder->multiply(bound);
    _cpder->add_constraint(pb_definition_id);
    _cpder->divide(bound+1);
    constraintid new_lower_bound_id = _cpder->end();
    delete_constraint_by_id(core_lower_bounds[core_idx]);
    core_lower_bounds[core_idx] = new_lower_bound_id;
    return new_lower_bound_id;
}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
constraintid MaxSATProoflogger<ObjLit, ObjCoeff, ObjConst>::derive_objective_reformulation_constraint(constraintid base_reform_id)
{
    _cpder->start_from_constraint(base_reform_id);
    for (int i = 0; i < core_lower_bounds.size(); i++)
    {
        _cpder->add_constraint(core_lower_bounds[i], core_weights[i]);
    }
    return _cpder->end();
}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
constraintid MaxSATProoflogger<ObjLit, ObjCoeff, ObjConst>::proof_log_objective_reformulation(constraintid base_reform_id, constraintid model_improve_id)
{
    // Add the lower bounds for all the cores we collected multiplied by the weight of the cores.
    constraintid objective_reform_id = derive_objective_reformulation_constraint(base_reform_id);

    // Add the model improving constraint to derive the desired constraint.
    _cpder->start_from_constraint(model_improve_id);
    _cpder->add_constraint(objective_reform_id);
    constraintid lower_bound_reformulated_objective = _cpder->end();
    delete_constraint_by_id(objective_reform_id);
    return lower_bound_reformulated_objective;
}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
constraintid MaxSATProoflogger<ObjLit, ObjCoeff, ObjConst>::base_reform_unit_core(constraintid base_reform_id, constraintid core_id, ObjCoeff weight)
{
    _cpder->start_from_constraint(core_id);
    _cpder->multiply(weight);
    _cpder->add_constraint(base_reform_id);
    constraintid new_base_reform_id = _cpder->end();
    delete_constraint_by_id(base_reform_id);
    return new_base_reform_id;
}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
constraintid MaxSATProoflogger<ObjLit, ObjCoeff, ObjConst>::reformulate_with_unprocessed_cores(constraintid base_reform_id, std::vector<constraintid> core_ids, std::vector<ObjCoeff> core_weights)
{
    start_intCP_derivation(base_reform_id);
    _cpder->start_from_constraint(base_reform_id);

    for (int i = 0; i < core_ids.size(); i++)
    {
        _cpder->add_constraint(core_ids[i], core_weights[i]);
    }

    constraintid reform_constraint_with_unprocessed_cores_id = _cpder->end();
    delete_constraint_by_id(base_reform_id);
    return reform_constraint_with_unprocessed_cores_id;
}

//=================================================================================================
// At-most-one constraints

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
template <class TSeqLit>
constraintid MaxSATProoflogger<ObjLit, ObjCoeff, ObjConst>::derive_at_most_one_constraint(const TSeqLit &am1_lits, const bool am1_sign)
{
    assert(am1_lits.size() > 1);
    std::vector<constraintid> binary_clauses;

    if(am1_lits.size() == 2){
        std::vector<VeriPB::Lit> lits; 
        lits.push_back(neg(toVeriPbLit(am1_lits[0])));
        lits.push_back(neg(toVeriPbLit(am1_lits[1])));
        return rup(lits, 1);
    }

    for (int new_lit_idx = 1; new_lit_idx < am1_lits.size(); new_lit_idx++)
    {
        if (new_lit_idx != 1) // first constraint is not multiplied
        {
            _cpder->multiply(new_lit_idx - 1);
        }

        for (int lit_idx = 0; lit_idx < new_lit_idx; lit_idx++)
        {
            constraintid binary_clause_id; 
            if(am1_sign)
                binary_clause_id = rup(std::vector<VeriPB::Lit>{neg(toVeriPbLit(am1_lits[new_lit_idx])), neg(toVeriPbLit(am1_lits[lit_idx]))});
            else
                binary_clause_id = rup(std::vector<VeriPB::Lit>{toVeriPbLit(am1_lits[new_lit_idx]), toVeriPbLit(am1_lits[lit_idx])});
            
            binary_clauses.push_back(binary_clause_id);

            // the first binary constraint to start the derivation
            if (new_lit_idx == 1)
            {
                _cpder->start_from_constraint(binary_clause_id);
            }
            else
            {
                _cpder->add_constraint(binary_clause_id);
            }
        }
        _cpder->divide(new_lit_idx);
    }
    constraintid am1_constraint_id = _cpder->end();
    delete_constraint_by_id(binary_clauses);
    return am1_constraint_id;
}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
template <class TSeqLit, class TLit>
constraintid MaxSATProoflogger<ObjLit, ObjCoeff, ObjConst>::introduce_at_most_one_selector(const TSeqLit &am1_lits, const TLit &selector_all_lit)
{
    std::vector<VeriPB::Lit> constraint_lits;
    for(int i = 0; i < am1_lits.size(); i++)
        constraint_lits[i] = toVeriPbLit(am1_lits[i]);
    
    VeriPB::Lit sel = toVeriPbLit(selector_all_lit);

    constraint_lits.push_back(sel);

    substitution witness = get_new_substitution();
    add_boolean_assignment(witness, variable(sel), !is_negated(sel));

    constraintid reified_am1_constraint_id = redundanceBasedStrengthening(constraint_lits, am1_lits.size(), witness);

    for (int i = 0; i < constraint_lits.size(); i++)
    {
        constraint_lits[i] = neg(constraint_lits[i]);
    }

    witness =  get_new_substitution();
    add_boolean_assignment(witness, variable(sel), is_negated(sel));
    redundanceBasedStrengthening(constraint_lits, 1, witness);

    return reified_am1_constraint_id;
}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
template <class TSeqLit, class TLit>
constraintid MaxSATProoflogger<ObjLit, ObjCoeff, ObjConst>::proof_log_at_most_one(constraintid base_reform_id, const TSeqLit &am1_lits, const TLit &selector_all_lit, ObjCoeff weight)
{
    constraintid am1_constraint = derive_at_most_one_constraint(am1_lits);
    constraintid am1_lower_bound = introduce_at_most_one_selector(am1_lits, selector_all_lit);
    delete_constraint_by_id(am1_constraint);

    _cpder->start_from_constraint(am1_lower_bound);
    _cpder->multiply(weight);
    _cpder->add_constraint(base_reform_id);
    constraintid new_base_reform_id = _cpder->end();
    
    delete_constraint_by_id(base_reform_id);
    delete_constraint_by_id(am1_lower_bound);
    return new_base_reform_id;
}
}