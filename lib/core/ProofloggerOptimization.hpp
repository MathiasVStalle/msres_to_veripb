#include "ProofloggerOptimization.h"
#include "Prooflogger.hpp"

//=================================================================================================

using namespace VeriPB;

// ------------- Conclusion -------------
template <typename ObjLit, typename ObjCoeff, typename ObjConst>
void ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::write_conclusion_UNSAT_optimization(){
    *proof << "output NONE\n"
        << "conclusion BOUNDS INF INF\n"
        << "end pseudo-Boolean proof\n";
}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
void ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::write_conclusion_UNSAT_optimization(const constraintid& hint){
    *proof << "output NONE\n"
        << "conclusion BOUNDS INF : " << hint << " INF\n"
        << "end pseudo-Boolean proof\n";
}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
void ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::write_conclusion_OPTIMAL(){
    if(!_found_solution){
        write_conclusion_UNSAT_optimization();
    }
    else{
        *proof << "output NONE\n"
            << "conclusion BOUNDS " << number_to_string(_best_objective_value) << " " << number_to_string(_best_objective_value) << "\n"
            << "end pseudo-Boolean proof\n";
    }
}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
void ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::write_conclusion_OPTIMAL(const constraintid& hint){
    if(!_found_solution){
        write_conclusion_UNSAT_optimization();
    }
    else{
        *proof << "output NONE\n"
            << "conclusion BOUNDS " << number_to_string(_best_objective_value) << " : " << number_to_string(hint) << " " << number_to_string(_best_objective_value) << "\n"
            << "end pseudo-Boolean proof\n";
    }    
}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
void ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::write_conclusion_BOUNDS(const ObjConst& LB, const ObjConst& UB){
    *proof << "output NONE\n"
        << "conclusion BOUNDS " << number_to_string(LB) << " " << number_to_string(UB) << "\n"
        << "end pseudo-Boolean proof\n";
}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
void ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::write_conclusion_BOUNDS(const ObjConst&  LB, const constraintid& hint, const ObjConst&  UB){
    *proof << "output NONE\n"
        << "conclusion BOUNDS " << number_to_string(LB) << " : " << number_to_string(hint) << " " << number_to_string(UB) << "\n"
        << "end pseudo-Boolean proof\n";
}

// ------------- Objective function manipulation -------------

template<typename ObjLit, typename ObjCoeff, typename ObjConst>
void ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::set_objective(const LinTermBoolVars<ObjLit, ObjCoeff, ObjConst>* new_objective)
{
    assert(_objective.size() == 0 && _objective.constant() == 0);
    _objective = new_objective;
}


template<typename ObjLit, typename ObjCoeff, typename ObjConst>
void ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::add_objective_literal(const ObjLit& lit, const ObjCoeff weight){
    _objective.add_literal(lit, weight);
}

template<typename ObjLit, typename ObjCoeff, typename ObjConst>
bool ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::remove_objective_literal(const ObjLit& lit){
    return _objective.delete_literal(lit);
}

template<typename ObjLit, typename ObjCoeff, typename ObjConst>
ObjCoeff ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::get_objective_weight(const ObjLit& lit){
    int i=0;
    while(lit != _objective.literal(i)) i++;

    return _objective.coefficient(i);
}

template<typename ObjLit, typename ObjCoeff, typename ObjConst>
void ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::add_objective_constant(const ObjConst& weight){
    _objective.add_constant(weight);
}

template<typename ObjLit, typename ObjCoeff, typename ObjConst>
void ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::subtract_objective_constant(const ObjConst& weight){
    _objective.subtract_constant(weight);
}

template<typename ObjLit, typename ObjCoeff, typename ObjConst>
void ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::write_comment_objective_function()
{
    if(!_comments) return;

    *proof << "* f = ";
    for (int i = 0; i < _objective.size(); i++)
        write_weighted_literal(_objective.literal(i), _objective.coefficient(i));
    if(_objective.constant() != 0)
        *proof << " + " << number_to_string(_objective.constant());
    
    if(_found_solution)
        *proof << "; Current best solution: " << number_to_string(_best_objective_value);
    else 
        *proof << "; No solution found yet.";
    *proof << "\n";
}


template<typename ObjLit, typename ObjCoeff, typename ObjConst>
void ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::check_model_improving_constraint(const constraintid& cxnid){
    assert(_found_solution);

    VeriPB::Constraint<ObjLit, ObjCoeff, ObjConst> mic(&_objective, _best_objective_value-1, Comparison::LEQ);
    
    if(cxnid == undefcxn)
        check_constraint_exists(mic);
    else 
        equals_rule(mic, cxnid);
}

template<typename ObjLit, typename ObjCoeff, typename ObjConst>
ObjConst ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::get_best_objective_value(){
    return _best_objective_value;
}

// ------------- Solution improving -------------

/// @brief Calculate the objective value of the objective stored in VeriPBProoflogger for a model. This function uses the optimistic assumption that the literals are sorted.
/// @param model Vector of assignments to literals.
/// @return Objective value for model.
// TODO-Dieter: Add example of using lbools.
template <typename ObjLit, typename ObjCoeff, typename ObjConst>
template <typename TModel>
ObjConst ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::calculate_objective_value(const TModel& model)
{
    ObjConst objective_value = _objective.constant();
    ModelValue v;
    for (int i = 0; i < _objective.size(); i++)
    {
        ObjLit objlit = _objective.literal(i);
        v = model_value(variable(objlit), model, i==0);
        if (v == ModelValue::True)
        {
            #ifdef NONUMBERCONVERSION
                objective_value += _objective.coefficient(i); 
            #else
                objective_value += convert_numer(_objective.coefficient(i));
            #endif
        }
        else if(v == ModelValue::Undef){
            throw out_of_range("[CalculateObjectiveValue] Objective literal " + _varMgr->literal_to_string(objlit) + " not assigned a value." );
        }
    }
    return objective_value;
}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
template <typename TModel>
constraintid ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::log_solution(const TModel& model, const ObjConst objective_value, const bool derive_excluding_constraint, const bool only_original_variables_necessary, const bool log_as_comment)
{
    if(log_as_comment && !_comments) return get_model_improving_constraint();

    write_comment("Solution with objective value: " + number_to_string(objective_value));
    _log_solution(model, (derive_excluding_constraint ? "soli" : "sol"), only_original_variables_necessary, log_as_comment);
    
    if(!log_as_comment){ // Veripb automatically adds an improvement constraint so counter needs to be incremented
        _model_improvement_constraint = ++_constraint_counter;

        if(objective_value < _best_objective_value)
            _best_objective_value = objective_value;
    }

    return get_model_improving_constraint(); 
}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
template <class TModel>
constraintid ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::log_solution(const TModel& model, const bool derive_excluding_constraint, const bool only_print_original_variables,  const bool log_as_comment){
    _log_solution(model, (derive_excluding_constraint ? "soli" : "sol"), only_print_original_variables, log_as_comment);
    
    if(log_as_comment){
        return _model_improvement_constraint;
    }
    else{
        ObjConst objVal = calculate_objective_value(model);
        if(objVal < _best_objective_value){
            _best_objective_value = objVal;
            _model_improvement_constraint = ++_constraint_counter;
            return _model_improvement_constraint;
        }
        else{
            return undefcxn;
        }
    }
}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
template <class TModel>
constraintid ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::log_solution_if_improving(const TModel &model, const bool derive_excluding_constraint, bool only_original_variables_necessary, bool log_nonimproving_solution_as_comment)
{
    ObjConst current_objective_value = calculate_objective_value(model);
    if (current_objective_value < _best_objective_value)
    {
        if(_comments){
            write_comment_objective_function();
            write_comment("Objective update from " + number_to_string(_best_objective_value) + " to " + number_to_string(current_objective_value));
        }
        _log_solution(model, (derive_excluding_constraint ? "soli" : "sol"), only_original_variables_necessary, false );
    }
    else if(_comments && log_nonimproving_solution_as_comment){
        write_comment_objective_function();
        write_comment("Non-improving solution:");
        log_solution(model, current_objective_value, derive_excluding_constraint, only_original_variables_necessary, true);
    }

    return get_model_improving_constraint();
}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
constraintid ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::get_model_improving_constraint()
{
    return _model_improvement_constraint;
}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
void ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::update_model_improving_constraint(const constraintid& newmic){
    _model_improvement_constraint = newmic;
    if(_comments)
        write_comment("Model improving constraint: " + number_to_string(newmic));
}

// ------------- Objective update -------------

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
void ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::write_objective_update(){
    *proof << "obju new";
    for (int i = 0; i < _objective.size(); i++)
        write_weighted_literal(_objective.literal(i), _objective.coefficient(i));
    if(_objective.constant() != 0)
        *proof << number_to_string(_objective.constant());
    if(_objective.size() == 0 && _objective.constant() == 0)
        *proof << ' ';
    *proof << ";\n";
}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
template <class TLinTerm>
void ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::write_objective_update_diff(TLinTerm& oldObj, TLinTerm& newObj){
    *proof << "obju diff";
    for(int i = 0; i < size(oldObj); i++){
        *proof << " -";
        write_number(coefficient(oldObj,i), false);
        _varMgr->write_literal(literal(oldObj, i), proof, true);
    }
    for(int i = 0; i < size(newObj); i++){
        write_number(coefficient(newObj, i));
        _varMgr->write_literal(literal(newObj, i), proof, true);
    }

    *proof << " -";
    write_number(get_constant(oldObj), false);
    write_number(get_constant(newObj), true);
    *proof << ";\n";
}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
template <class TLit>
void ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::write_objective_update_diff_for_literal(TLit& literal_to_remove, ObjCoeff weight, ObjConst constant_for_lit, bool write_update_model_improving_constraint){
    write_comment("write_objective_update_diff_for_literal. Weight = " + number_to_string(weight));
    *proof << "obju diff -";
    write_number(weight, false);
    _varMgr->write_literal(literal_to_remove, proof, true);
    if(constant_for_lit > 0) 
        write_number(constant_for_lit, true);
    *proof << ";\n";

    if(write_update_model_improving_constraint && get_model_improving_constraint() != 0){
        write_comment("Update model-improving constraint:");
        if(constant_for_lit > 0)
            rup_unit_clause(literal_to_remove, false);

        _cpder->start_from_constraint(get_model_improving_constraint());
        
        if(constant_for_lit > 0){
            _cpder->add_constraint(-1, weight);
        }
        else{
            _cpder->add_literal_axiom(literal_to_remove, weight);
        }
        constraintid cxn = _cpder->end();
        update_model_improving_constraint(cxn);
    }
}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
template <class TLit> 
void ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::write_objective_update_diff_literal_replacement(TLit& literal_to_remove, TLit& literal_to_add, ObjCoeff weight, bool write_update_model_improving_constraint){
    *proof << "obju diff -";
    write_number(weight, false);
    _varMgr->write_literal(literal_to_remove, proof, true);
    write_number(weight);
    _varMgr->write_literal(literal_to_add, proof, true);
    *proof << ";\n";

    if(write_update_model_improving_constraint && get_model_improving_constraint() != 0){
        write_comment("Update model-improving constraint:");
        constraintid cxn_newlit_leq_oldlit = rup_binary_clause(neg(literal_to_add), literal_to_remove);
        
        _cpder->start_from_constraint(get_model_improving_constraint());
        _cpder->add_constraint(cxn_newlit_leq_oldlit, weight);
        constraintid cxn = _cpder->end();
        update_model_improving_constraint(cxn);
    }
}

// ------------- Constructor -------------

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::ProofloggerOpt(const std::string& prooffile, VarManager* varMgr) :
    Prooflogger(prooffile, varMgr)
{}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::ProofloggerOpt(const std::string& prooffile, VarManager* varMgr, int n_orig_constraints, bool keep_original_formula, bool comments) :
    Prooflogger(prooffile, varMgr, n_orig_constraints, keep_original_formula, comments)
{}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::ProofloggerOpt(std::ostream* proof, VarManager* varMgr) :
    Prooflogger(proof, varMgr)
{}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::ProofloggerOpt(std::ostream* proof, VarManager* varMgr, int n_orig_constraints, bool keep_original_formula, bool comments) :
    Prooflogger(proof, varMgr, n_orig_constraints, keep_original_formula, comments)
{}

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
ProofloggerOpt<ObjLit, ObjCoeff, ObjConst>::~ProofloggerOpt()
{}