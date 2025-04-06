#ifndef VeriPBProoflogger_h
#define VeriPBProoflogger_h

#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <sstream>
#include <set>
#include <cassert>
#include <variant>
#include <charconv>
#include <climits>
#include <stdexcept>

#include<iostream>

// NOTE! Should include definition for types Var, Lit and Clause
#include "VeriPbSolverTypes.h"
#include "VariableManager.h"



//prooflogging Library

/**
 * TODOs
 * - Dominance rule
   - Instantiate important functions for VeriPB::Var, VeriPB::Lit, constraintid as TNumber, ...
   - Replace number_to_string by write_number as much as possible
   - Set n orig vars (or make varMgr public)
   - Constructors:
        * Only proof file name and open it manually/prooffilestream
        * VarMgr
    - Make proof goal abstract, so that proof goal is not a string anymore, but something readable.
    - Add functions to start and end proof easier.
    - Include in documentation:
        * compiler macros
 */

//=================================================================================================
// Prooflogger

namespace VeriPB {

    class Prooflogger;
    
    const constraintid undefcxn = 0;
    typedef std::pair<std::vector<std::pair<VeriPB::Var, VeriPB::Lit>>, std::vector<std::pair<VeriPB::Var, bool>>> substitution;

    struct subproof {
        std::string proofgoal;
        std::vector<std::string> derivations;
    };

    class CuttingPlanesDerivation
    {
        friend Prooflogger; 

        public:
        std::string toString() const;
        bool writing_directly_to_proof() const;
        bool isEmpty() const;

        void setProoflogger(Prooflogger* _pl);
        void clear(); // asserts that not writing to proof.

        void start_from_constraint(const constraintid& cxn_id);
        template <class TLit>
        void start_from_literal_axiom(const TLit& lit);
        template <class TNumber=VeriPB::defaultmultipliertype>
        void add(const CuttingPlanesDerivation* cp_to_add, const TNumber& mult=1); 
        void start_subderivation_from_constraint(const constraintid& cxn_id);
        template <class TLit>
        void start_subderivation_from_literal_axiom(const TLit& lit_axiom);
        void add_subderivation();
        template <class TNumber=VeriPB::defaultmultipliertype>
        void add_constraint(const constraintid& cxn_id, const TNumber& mult=1);
        template <class TLit, class TNumber=VeriPB::defaultmultipliertype>
        void add_literal_axiom(const TLit& lit_axiom, const TNumber& mult=1);
        template <class TNumber=VeriPB::defaultmultipliertype>
        void divide(const TNumber& n);
        void saturate();
        template <class TNumber=VeriPB::defaultmultipliertype>
        void multiply(const TNumber& n);
        template <class TVar>
        void weaken(const TVar& var);
        constraintid end(bool clear=true);

        CuttingPlanesDerivation(Prooflogger* pl = nullptr, bool write_directly_to_proof=false);

        private: 
        bool _write_directly_to_proof;
        bool _finished=true; // If derivation is written directly to the proof, a new derivation can only be started when the previous derivation is finished.
        
        Prooflogger* _pl;
        std::string* _buffer;
        bool _bufferOwned;

        CuttingPlanesDerivation(Prooflogger* pl, std::string* buffer);

        public:
        ~CuttingPlanesDerivation();

        CuttingPlanesDerivation& copyTo(CuttingPlanesDerivation&) const;
        CuttingPlanesDerivation& operator=(const CuttingPlanesDerivation&);
    };
    class Prooflogger
    {
    public:
        // ------------- Proof file manipulation -------------
        void set_proof_stream(std::ostream* proof);
        void set_keep_original_formula_on();
        void set_keep_original_formula_off();
        void write_proof_header();
        void set_n_orig_constraints(int nbconstraints);
        bool is_original_constraint(const constraintid& cxn);
        constraintid get_constraint_counter();
        constraintid increase_constraint_counter();
        void set_variable_manager(VarManager* varMgr);
        void flush_proof();

        // ------------- Solution Logging -------------
        bool logged_solution();
        template <class TModel>
        constraintid log_solution(const TModel& model, const bool derive_excluding_constraint=true, const bool only_print_original_variables=true,  const bool log_as_comment=false);


        // ------------- Conclusion -------------
        void write_conclusion_NONE();
        void write_conclusion_UNSAT();
        void write_conclusion_SAT();
        void write_fail();

        // ------------- Cutting Planes derivations -------------
        friend CuttingPlanesDerivation;

        constraintid copy_constraint(const constraintid cxn);

        CuttingPlanesDerivation get_cuttingplanes_derivation(bool write_directly_to_proof=false, bool use_internal_buffer=false);

        // ------------- Comments -------------
        void write_comment(const char *comment);
        void write_comment(const std::string &comment);

        // ------------- Rules for checking constraints -------------
        template <class TConstraint>
        void equals_rule(const TConstraint& cxn, const constraintid cxn_id);
        template <class TConstraint>
        void check_last_constraint(const TConstraint& cxn);
        template <class TConstraint>
        void check_constraint_exists(const TConstraint& cxn);
        template <class TConstraint>
        void check_implied(const TConstraint& cxn);
        template <class TConstraint>
        void check_implied(const TConstraint& cxn, constraintid cxn_id);

        // ------------- Rules for adding syntactically implied constraints -------------
        template <class TConstraint>
        constraintid derive_if_implied(const TConstraint& cxn);
        template <class TConstraint>
        constraintid derive_if_implied(const TConstraint& cxn, const constraintid& cxn_id);

        // ------------- Unchecked Assumptions -------------
        template <class TConstraint>
        constraintid unchecked_assumption(const TConstraint& cxn);

        // ------------- Reverse Unit Propagation -------------
        template <class TConstraint>
        constraintid rup(const TConstraint& cxn, bool core_constraint=false);
        template <class TConstraint>
        constraintid rup(const TConstraint& cxn, const std::vector<constraintid>& hints, const bool core_constraint=false);

        template <class TConstraint>
        constraintid rup_clause(const TConstraint& lits);
        template <class TConstraint>
        constraintid rup_clause(const TConstraint& lits, std::vector<constraintid>& hints);
        
        template <class TLit>
        constraintid rup_unit_clause(const TLit& lit, bool core_constraint=true);
        template <class TLit>
        constraintid rup_unit_clause(const TLit& lit, std::vector<constraintid>& hints, bool core_constraint=true);
        
        template <class TLit>
        constraintid rup_binary_clause(const TLit& lit1, const TLit& lit2, bool core_constraint=false);
        template <class TLit>
        constraintid rup_binary_clause(const TLit& lit1, const TLit& lit2, std::vector<constraintid>& hints,  bool core_constraint=false);
        
        template <class TLit> 
        constraintid rup_ternary_clause(const TLit& lit1, const TLit& lit2, const TLit& lit3, bool core_constraint=false);
        template <class TLit> 
        constraintid rup_ternary_clause(const TLit& lit1, const TLit& lit2, const TLit& lit3, std::vector<constraintid>& hints, bool core_constraint=false);

        // ------------- Redundance Based Strenghtening -------------
        void strenghten_to_core_on();
        void strenghten_to_core_off();

        substitution get_new_substitution();
        template <class TVar>
        void add_boolean_assignment(substitution &s, const TVar& var, const bool value);
        template <class TVar, class TLit>
        void add_literal_assignment(substitution &s, const TVar& var, const TLit& value);
        void add_substitution(substitution &sub, const substitution &sub_to_add);
        template <class TVar>
        bool has_boolean_assignment(const substitution &s, const TVar& var);
        template <class TVar>
        bool has_literal_assignment(const substitution &s, const TVar& var);
        template <class TVar>
        bool get_boolean_assignment(substitution &s, const TVar& var);
        template <class TVar>
        VeriPB::Lit get_literal_assignment(substitution &s, const TVar& var);
        size_t get_substitution_size(const substitution &s);

        // Assumption here is that these can prove the implication F ^ not C |- F\w ^ C\w trivially,
        // i.e., for every constraint C' in F\w ^ C\w, C' is either in F or that w only assigns literals to true in C'.
        template <class TConstraint>
        constraintid redundance_based_strengthening(const TConstraint& cxn, const substitution& witness);
        template <class TLit>
        constraintid redundance_based_strengthening_unit_clause(const TLit& lit);
        
        template <class TConstraint>
        constraintid redundance_based_strengthening(const TConstraint& cxn, const substitution& witness, const std::vector<subproof>& subproofs);
        

        template <class TConstraint>
        constraintid start_redundance_based_strengthening_with_subproofs(const TConstraint& cxn, const substitution& witness);
        constraintid start_new_subproof(const std::string& proofgoal);
        void end_subproof();
        constraintid end_redundance_based_strengthening_with_subproofs();

        // ------------- Reification -------------
        template <class TLit, class TConstraint>
        constraintid reification_literal_right_implication(const TLit& lit, const TConstraint& cxn, const bool store_reified_constraint=false);
        template <class TLit, class TConstraint>
        constraintid reification_literal_left_implication(const TLit& lit, const TConstraint& cxn, const bool store_reified_constraint=false);
        
        template <class TVar>
        void delete_reified_constraint_left_implication(const TVar& var);
        template <class TVar>
        void delete_reified_constraint_right_implication(const TVar& var);

        template <class TVar>
        constraintid get_reified_constraint_left_implication(const TVar& var);
        template <class TVar>
        constraintid get_reified_constraint_right_implication(const TVar& var);
        template <class TVar>
        void store_reified_constraint_left_implication(const TVar& var, const constraintid& cxnId);
        template <class TVar>
        void store_reified_constraint_right_implication(const TVar& var, const constraintid& cxnId);

        /**
         * Remove the right reification constraint from the reification constraint store without deleting it in the proof. 
         * Only needed for not maintaining a constraint id in memory that will not be used in the proof anymore.
        */
        template <class TVar>
        void remove_reified_constraint_right_implication_from_constraintstore(const TVar& var);
        /**
         * Remove the left reification constraint from the reification constraint store without deleting it in the proof. 
         * Only needed for not maintaining a constraint id in memory that will not be used in the proof anymore.
        */
        template <class TVar>
        void remove_reified_constraint_left_implication_from_constraintstore(const TVar& var);

        // ------------- Proof by contradiction -------------
        /**
         * Derives a constraint C by assuming the negation of C and showing by means of a cutting planes derivation that a 
         * conflict is derived if the negation of C is derived.
         * This is done by proving C using substitution redundancy with an empty witness. 
         * If C is proven by substitution redundancy using any witness w, it should be proven that for a formula F : F ^ ~C |= F\w  ^ C\w.
         * Hence, if w is empty, then it has to be proven that F ^ ~C |= C, which is indeed a contradiction. 
         * 
        */
        template <class TConstraint>
        constraintid start_proof_by_contradiction(const TConstraint& cxn);
        constraintid end_proof_by_contradiction();
        
        // ------------- Proof by case splitting -------------
        /**
         * If it is possible to derive the following two constraints:
         *      k x + t >= k
         *      k ~x + t >= k
         * then it is possible to derive t >= k. 
         * 
         * Lits and weights are then the literals and weights in t, whereas the RHS coincides with k.   
         * case1 and case2 are the constraintid's for the constraints depicted above.
        */
        template <class TConstraint>
        constraintid prove_by_casesplitting(const TConstraint& cxn, const constraintid& case1, const constraintid& case2);

        // ------------- Deleting & Overwriting Constraints -------------
        void delete_constraint_by_id(const constraintid constraint_id, bool overrule_keeporiginalformula=false);
        void delete_constraint_by_id(const constraintid constraint_id, const substitution& witness, bool overrule_keeporiginalformula=false); // TODO-Dieter
        void delete_constraint_by_id(const constraintid constraint_id, const substitution& witness, const subproof& subproof, bool overrule_keeporiginalformula=false); // TODO-Dieter
        void delete_constraint_by_id(const constraintid constraint_id, const substitution& witness, const std::vector<subproof>& subproofs, bool overrule_keeporiginalformula=false); // TODO-Dieter
        void delete_constraint_by_id(const std::vector<constraintid> &constraint_ids, bool overrule_keeporiginalformula=false);
        
        template <class TConstraint> 
        void delete_constraint(const TConstraint& cxn, const bool overrule_keeporiginalformula=false);
        template <class TConstraint>
        void delete_constraint(const TConstraint& cxn, const substitution& witness, bool overrule_keeporiginalformula=false);
        template <class TConstraint> 
        void delete_constraint(const TConstraint& cxn, const substitution& witness, const subproof& subproof, bool overrule_keeporiginalformula=false); // TODO-Dieter
        template <class TConstraint> 
        void delete_constraint(const TConstraint& cxn, const substitution& witness, const std::vector<subproof>& subproofs, bool overrule_keeporiginalformula=false); // TODO-Dieter
        
        // Removal by del find where a literal occuring multiple times in lits is only written once.
        template <class TConstraint>
        void delete_clause(const TConstraint& cxn, bool overrule_keeporiginalformula=false);

        template <class TConstraint>
        constraintid overwrite_constraint(const constraintid& orig_cxn_id, const TConstraint& new_cxn, bool origclause_in_coreset=false);
        template <class TConstraint>
        constraintid overwrite_constraint(const TConstraint& orig_cxn, const TConstraint& new_cxn, bool origclause_in_coreset=false);

        void move_to_coreset(const constraintid& cxn, bool overrule_keeporiginalformula=false);
        template <class TConstraint>
        void move_to_coreset(const TConstraint& cxn, bool overrule_keeporiginalformula=false);
    
        // ------------- Constructor -------------
        Prooflogger(const std::string& prooffile, VarManager* varMgr);
        Prooflogger(const std::string& prooffile, VarManager* varMgr, int n_orig_constraints, bool keep_original_formula=false, bool comments=true);
        Prooflogger(std::ostream* proof, VarManager* varMgr);
        Prooflogger(std::ostream* proof, VarManager* varMgr, int n_orig_constraints, bool keep_original_formula=false, bool comments=true);
        ~Prooflogger();
    protected:
        // ------------- Variable Manager -------------
        VarManager* _varMgr;        

        // ------------- Formula information -------------
        bool _keep_original_formula = false; // If true, the proof logging library will never delete any constraint that is an original constraint and will never move a constraint to the core set. 
        int _n_orig_constraints = 0;
        constraintid _constraint_counter = 0;
        bool _found_solution=false; // TODO: Keep track of bookkeeping of already found solution.

        // ------------- Formula stream -------------
        std::ostream* proof;
        bool _proofOwned;
        int _write_buffer_size = 32 * 1024 * 1024;
        char* _write_buffer = new char[_write_buffer_size]; // Buffer for the proof.

        
        // ------------- Cutting plane derivations -------------
        // TODO: add to the constructors.
        std::string _cuttingplanes_buffer;
        CuttingPlanesDerivation* _cpder;

        // ------------- Commenting -------------
        bool _comments=true;  //TODO: add compile definition instead
       

        // ------------- Writing -------------
        template <class TLit, class TNumber=VeriPB::defaultmultipliertype>
        void write_weighted_literal(const TLit &literal, const TNumber& weight = 1, const bool& add_prefix_space=true);
        template <typename TModel>
        void _log_solution(const TModel& model, const std::string& log_command="sol", const bool only_original_variables_necessary=false, const bool log_as_comment=false);
        template <typename TConstraint>
        void write_constraint(const TConstraint& cxn);
        template <typename TClause>
        void write_clause(const TClause& cxn);
 
        void write_hints(const std::vector<constraintid>& hints);
        void write_substitution(const substitution &witness);
        
        // ------------- Reification Variables -------------
        std::vector<constraintid> _reifiedConstraintLeftImpl;
        std::vector<constraintid> _reifiedConstraintRightImpl;
        std::vector<constraintid> _reifiedConstraintLeftImplOnlyProofVars;
        std::vector<constraintid> _reifiedConstraintRightImplOnlyProofVars;
    };
}

//=================================================================================================
#endif
