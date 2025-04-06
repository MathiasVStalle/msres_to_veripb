#ifndef VeriPBProofloggerOptimization_h
#define VeriPBProofloggerOptimization_h

#include "Prooflogger.h"

namespace VeriPB {

template <typename ObjLit, typename ObjCoeff, typename ObjConst>
class ProofloggerOpt : public Prooflogger
{
public:
    // ------------- Objective function manipulation -------------
    void set_objective(const LinTermBoolVars<ObjLit, ObjCoeff, ObjConst>* new_objective);
    void add_objective_literal(const ObjLit& lit, const ObjCoeff weight);
    bool remove_objective_literal(const ObjLit& lit);
    ObjCoeff get_objective_weight(const ObjLit& lit);
    void add_objective_constant(const ObjConst& weight);
    void subtract_objective_constant(const ObjConst& weight);
    void write_comment_objective_function();
    void check_model_improving_constraint(const constraintid& cxnid=undefcxn);
    ObjConst get_best_objective_value();

    // ------------- Solution Logging -------------
    template <typename TModel>
    ObjConst calculate_objective_value(const TModel& model);
    template <class TModel>
    constraintid log_solution(const TModel& model, const bool derive_excluding_constraint=true, const bool only_print_original_variables=true,  const bool log_as_comment=false);
    template <typename TModel>
    constraintid log_solution(const TModel& model, const ObjConst objective_value, const bool derive_excluding_constraint=true, const bool only_original_variables_necessary=true, const bool log_as_comment=false);
    template <class TModel>
    constraintid log_solution_if_improving(const TModel &model, const bool derive_excluding_constraint=true, bool only_original_variables_necessary=true, bool log_nonimproving_solution_as_comment=false);
    constraintid get_model_improving_constraint();
    void update_model_improving_constraint(const constraintid& newmic);
    
    // ------------- Objective update -------------
    void write_objective_update();
    //TODO-Dieter: Also create objective update with subproofs/hints.
    template <class TLinTerm>
    void write_objective_update_diff(TLinTerm& oldObj, TLinTerm& newObj);
    template <class TLit>
    void write_objective_update_diff_for_literal(TLit& literal_to_remove, ObjCoeff weight = 1, ObjConst constant_for_lit = 0, bool update_model_improving_constraint=false);
    template <class TLit> 
    void write_objective_update_diff_literal_replacement(TLit& literal_to_remove, TLit& literal_to_add, ObjCoeff weight=1, bool update_model_improving_constraint=false);

    // ------------- Conclusion -------------
    void write_conclusion_OPTIMAL();
    void write_conclusion_OPTIMAL(const constraintid& hint);
    void write_conclusion_BOUNDS(const ObjConst& LB, const ObjConst& UB);
    void write_conclusion_BOUNDS(const ObjConst& LB, const constraintid& hint, const ObjConst& UB);
    void write_conclusion_UNSAT_optimization();
    void write_conclusion_UNSAT_optimization(const constraintid& hint);
    
    // ------------- Constructor -------------
    ProofloggerOpt(const std::string& prooffile, VarManager* varMgr);
    ProofloggerOpt(const std::string& prooffile, VarManager* varMgr, int n_orig_constraints, bool keep_original_formula=false, bool comments=true);
    ProofloggerOpt(std::ostream* proof, VarManager* varMgr);
    ProofloggerOpt(std::ostream* proof, VarManager* varMgr, int n_orig_constraints, bool keep_original_formula=false, bool comments=true);
    ~ProofloggerOpt();
private:
    // ------------- Objective function -------------
    // Objective function
    LinTermBoolVars<ObjLit, ObjCoeff, ObjConst> _objective;
    ObjConst _best_objective_value;
    constraintid _model_improvement_constraint = 0; // Last model improvement constraint


};

}

#endif