#include "VeriPbSolverTypes.h"
#include <charconv>
#include <cassert>
#include <iostream>


namespace VeriPB{

/*******************
 * Functions for VeriPB variables and literals 
*/
VeriPB::Var variable(const VeriPB::Lit& l){return l.v;}
bool is_negated(const VeriPB::Lit& l){return l.negated;}
VeriPB::Lit neg(const VeriPB::Lit& l){VeriPB::Lit newl; newl.v = l.v; newl.negated = !l.negated; return newl;}
VeriPB::Lit create_literal(const VeriPB::Var& var, bool negated){VeriPB::Lit l; l.v = var; l.negated = negated; return l; }

VeriPB::Var toVeriPbVar(const VeriPB::Var& v){return v;}
VeriPB::Lit toVeriPbLit(const VeriPB::Lit& l){return l;}

VeriPB::VarIdx varidx(const VeriPB::Var& var){return var.v;}

/*******************
 * Functions for constraints:
*/

template <typename TLit, typename TCoeff, typename TRhs> 
inline TLit VeriPB::literal(const VeriPB::Constraint<TLit, TCoeff, TRhs>& cxn, const VeriPB::litindex& index){ 
    return cxn.literal(index);
}

template <typename TLit, typename TCoeff, typename TRhs> 
inline TCoeff VeriPB::coefficient(const VeriPB::Constraint<TLit, TCoeff, TRhs>& cxn, const VeriPB::litindex& index){ 
    return cxn.coefficient(index);
}

template <typename TLit, typename TCoeff, typename TRhs> 
inline VeriPB::Comparison comparison(const VeriPB::Constraint<TLit, TCoeff, TRhs>& cxn){
    return cxn.comparison();
}

template <typename TLit, typename TCoeff, typename TRhs> 
inline TRhs rhs(const VeriPB::Constraint<TLit, TCoeff, TRhs>& cxn){
    return cxn.rhs();
}

template <typename TLit, typename TCoeff, typename TRhs>
inline size_t size(const VeriPB::Constraint<TLit, TCoeff, TRhs>& cxn){
    return cxn.size();
}

template <typename TLit, typename TCoeff, typename TRhs>
inline TRhs sum_of_coefficients(const VeriPB::Constraint<TLit, TCoeff, TRhs>& cxn){
    return cxn.sum_of_coefficients();
}

/********************
 * Functions for linear terms
 */
template <typename TLit, typename TCoeff, typename TConst>
inline TLit literal(const VeriPB::LinTermBoolVars<TLit, TCoeff, TConst>& linterm, const VeriPB::litindex& index){
    return linterm.literal(index);
}

template <typename TLit, typename TCoeff, typename TConst>
inline TCoeff coefficient(const VeriPB::LinTermBoolVars<TLit, TCoeff, TConst>& linterm, const VeriPB::litindex& index){
    return linterm.coefficient(index);
}

template <typename TLit, typename TCoeff, typename TConst>
inline TCoeff get_constant(const VeriPB::LinTermBoolVars<TLit, TCoeff, TConst>& linterm){
    return linterm.constant();
}

template <typename TLit, typename TCoeff, typename TConst>
inline TConst sum_of_coefficients(const VeriPB::LinTermBoolVars<TLit, TCoeff, TConst>& linterm){
    return linterm._sum_coeffs;
}

/*******************
 * Functions to deal with numbers
 */

template <typename TNumber>
void VeriPB::write_number(const TNumber& n, std::ostream* s, const bool prefixspace){
    static constexpr size_t buffer_size = 8*sizeof(TNumber);
    static char             buffer[buffer_size]{}; 

    if(prefixspace)
        buffer[0] = ' ';
    std::to_chars_result result = std::to_chars(buffer + prefixspace, buffer + buffer_size, n);
    assert(result.ec != std::errc::value_too_large); 
    s->write(buffer, result.ptr - buffer);
}
#ifndef NOINIT_CONSTRAINTID
template void VeriPB::write_number<VeriPB::constraintid>(const VeriPB::constraintid&, std::ostream*, const bool);
#endif
#ifndef NOINIT_VARIDX
template void VeriPB::write_number<VeriPB::VarIdx>(const VeriPB::VarIdx&, std::ostream*, const bool);
#endif
#ifndef NOINIT_LITINDEX
template void VeriPB::write_number<VeriPB::litindex>(const VeriPB::litindex&, std::ostream*, const bool);
#endif
#ifndef NOINIT_DEFAULTMULT
template void VeriPB::write_number<VeriPB::defaultmultipliertype>(const VeriPB::defaultmultipliertype&, std::ostream*, const bool);
#endif

template <typename TNumber>
std::string number_to_string(const TNumber& n){
    return std::to_string(n);
}
#ifndef NOINIT_CONSTRAINTID
template std::string number_to_string<VeriPB::constraintid>(const VeriPB::constraintid&);
#endif
#ifndef NOINIT_VARIDX
template std::string number_to_string<VeriPB::VarIdx>(const VeriPB::VarIdx&);
#endif
#ifndef NOINIT_LITINDEX
template std::string number_to_string<VeriPB::litindex>(const VeriPB::litindex&);
#endif
#ifndef NOINIT_DEFAULTMULT
template std::string number_to_string<VeriPB::defaultmultipliertype>(const VeriPB::defaultmultipliertype&);
#endif

#ifndef NONUMBERCONVERSION
template <typename TNumber1, typename TNumber2>
TNumber2 convert_number(const TNumber1& n){
    return n;
}
#ifndef NOINIT_CONSTRAINTID
template VeriPB::constraintid convert_number<VeriPB::constraintid,VeriPB::constraintid>(const VeriPB::constraintid&);
#endif
#ifndef NOINIT_VARIDX
template VeriPB::VarIdx convert_number<VeriPB::VarIdx,VeriPB::VarIdx>(const VeriPB::VarIdx&);
#endif
#ifndef NOINIT_LITINDEX
template VeriPB::litindex convert_number<VeriPB::litindex,VeriPB::litindex>(const VeriPB::litindex&);
#endif
#endif

/*******************
 * Functions for models. 
 */

template <typename TModel>
VeriPB::Lit model_literal(const VeriPB::litindex& i, const TModel& model){
    return toVeriPbLit(model[i]);
}

template <typename TSeqLit>
size_t model_size(const TSeqLit& model){
    return model.size();
}

template <typename TVar, typename TModel>
VeriPB::ModelValue model_value(const TVar& var, const TModel& model, bool first_call){
    static int i; // Uses the optimistic assumption that model_value will be called multiple times for variables that are placed in an increasing index. 

    if(first_call)
        i = 0;

    int start_i = i;

    VeriPB::Var vvar = toVeriPbVar(var);
    VeriPB::Lit vlit = toVeriPbLit(model_literal(model, i));

    while(vvar != VeriPB::variable(vlit)){
        i++;
        if(i >= model_size(model))
            i=0;
        else if(i == start_i)
            return VeriPB::ModelValue::Undef;
        vlit = toVeriPbLit(model_literal(model, i));
    }
    if(is_negated(vlit))
        return VeriPB::ModelValue::False;
    else
        return VeriPB::ModelValue::True;
}


/*******************
 * Operators for VeriPB::Var and VeriPB::Lit
 */

inline bool operator==(const VeriPB::Var& lhs, const VeriPB::Var& rhs){
	return lhs.v == rhs.v && lhs.only_known_in_proof == rhs.only_known_in_proof;
}

inline bool operator!=(const VeriPB::Var& lhs, const VeriPB::Var& rhs){
	return !(lhs == rhs);
}

inline bool operator< (const VeriPB::Var& lhs, const VeriPB::Var& rhs){
    return (!lhs.only_known_in_proof && rhs.only_known_in_proof) 
                || (lhs.only_known_in_proof == rhs.only_known_in_proof && lhs.v < rhs.v);
}
inline bool operator<=(const VeriPB::Var& lhs, const VeriPB::Var& rhs){
    return (lhs < rhs) || (lhs == rhs);
}
inline bool operator> (const VeriPB::Var& lhs, const VeriPB::Var& rhs){
    return rhs < lhs;
}
inline bool operator>=(const VeriPB::Var& lhs, const VeriPB::Var& rhs){
    return rhs <= lhs;
}

inline bool operator==(const VeriPB::Lit& lhs, const VeriPB::Lit& rhs){
    return lhs.v == rhs.v && lhs.negated == rhs.negated;
}

inline bool operator!=(const VeriPB::Lit& lhs, const VeriPB::Lit& rhs){
    return !(lhs == rhs);
}

inline bool operator< (const VeriPB::Lit& lhs, const VeriPB::Lit& rhs){
    return lhs.v < rhs.v || (lhs.v == rhs.v && lhs.negated && !rhs.negated);
}
inline bool operator<=(const VeriPB::Lit& lhs, const VeriPB::Lit& rhs){
    return (lhs < rhs) || (lhs == rhs);
}
inline bool operator> (const VeriPB::Lit& lhs, const VeriPB::Lit& rhs){
    return rhs < lhs;
}
inline bool operator>=(const VeriPB::Lit& lhs, const VeriPB::Lit& rhs){
    return rhs <= lhs;
}

/**
 * Implementation for Linear Terms
 */

template <typename TLit, typename TCoeff, typename TConst>
inline TLit VeriPB::LinTermBoolVars<TLit, TCoeff, TConst>::literal(const litindex& idx) const {
    return _literals->at(idx);
}

template <typename TLit, typename TCoeff, typename TConst>
inline TCoeff VeriPB::LinTermBoolVars<TLit, TCoeff, TConst>::coefficient(const litindex& idx) const {
    if(_all_coeff_one)
        return 1;
    else
        return _coefficients->at(idx);
}

template <typename TLit, typename TCoeff, typename TConst>
inline TConst VeriPB::LinTermBoolVars<TLit, TCoeff, TConst>::constant() const {
    return _constant;
}

template <typename TLit, typename TCoeff, typename TConst>
inline size_t VeriPB::LinTermBoolVars<TLit, TCoeff, TConst>::size() const {
    return _literals->size();
}

template <typename TLit, typename TCoeff, typename TConst>
inline void VeriPB::LinTermBoolVars<TLit, TCoeff, TConst>::add_literal(const TLit& lit, const TCoeff& coeff){
    assert(!(_all_coeff_one && coeff > 1));

    _literals->push_back(lit);
    if(!_all_coeff_one)
        _coefficients->push_back(coeff);
    #ifdef NONUMBERCONVERSION
        _sum_coeffs += coeff; 
    #else 
        _sum_coeffs += convert_number<TCoeff, TConst>(coeff);
    #endif
}
template <typename TLit, typename TCoeff, typename TConst>
bool VeriPB::LinTermBoolVars<TLit, TCoeff, TConst>::delete_literal(const TLit& lit){
    int i=0;
    bool found = false;

    while(i < _literals->size() && lit != _literals->at(i)) i++;

    if(i < _literals->size() && lit == _literals->at(i)) found=true; 

    if(found)
        _sum_coeffs -= _coefficients->at(i);

    while(found && i+1 < _literals->size()) {
        assert(_literals->at(i+1) != lit);
        _literals->at(i) = _literals->at(i+1);
        if(!_all_coeff_one)
            _coefficients->at(i) = _coefficients->at(i+1);
        i++;
    }
    
    if(found && _literals->size() > 0)
        _literals->resize(_literals->size()-1);
    if(found && !_all_coeff_one &&  _literals->size() > 0)
        _coefficients->resize(_coefficients->size()-1);

    return found;
}
template <typename TLit, typename TCoeff, typename TConst>
bool VeriPB::LinTermBoolVars<TLit, TCoeff, TConst>::delete_literal(const litindex& index){
    bool found = index < _literals->size();
    if(found){
        _sum_coeffs -= _coefficients->at(index);
        _literals->erase(_literals->begin() + index);
        if(!_all_coeff_one)
            _coefficients->erase(_coefficients->begin() + index);

    }

    return found; 
}
template <typename TLit, typename TCoeff, typename TConst>
TConst VeriPB::LinTermBoolVars<TLit, TCoeff, TConst>::sum_of_coefficients() const {
    return _sum_coeffs;
}

template <typename TLit, typename TCoeff, typename TConst>
inline void VeriPB::LinTermBoolVars<TLit, TCoeff, TConst>::add_constant(const TConst& constant){
    _constant += constant;
}
template <typename TLit, typename TCoeff, typename TConst>
inline void VeriPB::LinTermBoolVars<TLit, TCoeff, TConst>::subtract_constant(const TConst& constant){
    _constant -= constant;
}
template <typename TLit, typename TCoeff, typename TConst>
void VeriPB::LinTermBoolVars<TLit, TCoeff, TConst>::clear(const bool all_coeff_one, const TConst& new_const){
    _all_coeff_one=all_coeff_one;
    _constant = new_const;
    _coefficients->clear();
    _literals->clear();
}

template <typename TLit, typename TCoeff, typename TConst>
inline VeriPB::LinTermBoolVars<TLit, TCoeff, TConst>::LinTermBoolVars(const bool all_coeff_one) : 
    _literals(new std::vector<TLit>),
    _all_coeff_one(all_coeff_one),
    _constant(0),
    _owned(true)
{
    if(!all_coeff_one)
        _coefficients = new std::vector<TCoeff>;
}

template <typename TLit, typename TCoeff, typename TConst>
inline VeriPB::LinTermBoolVars<TLit, TCoeff, TConst>::LinTermBoolVars(std::vector<TLit>* lits, std::vector<TCoeff>* coeff, TConst constant) :
    _literals(lits),
    _coefficients(coeff),
    _constant(constant),
    _all_coeff_one(false),
    _owned(false)
{ }

template <typename TLit, typename TCoeff, typename TConst>
inline VeriPB::LinTermBoolVars<TLit, TCoeff, TConst>::LinTermBoolVars(std::vector<TLit>* lits, TConst constant) :
    _literals(lits),
    _coefficients(nullptr),
    _constant(constant),
    _all_coeff_one(true),
    _owned(false)
{ }

template <typename TLit, typename TCoeff, typename TConst>
inline VeriPB::LinTermBoolVars<TLit, TCoeff, TConst>::~LinTermBoolVars(){
    if(_owned)
        delete _coefficients;
        delete _literals;
}

/**
 * Implementation for Constraint
 */


template <typename TLit, typename TCoeff, typename TRhs>
inline TLit VeriPB::Constraint<TLit, TCoeff, TRhs>::literal(const litindex& idx) const {
    return _linterm->literal(idx);
}

template <typename TLit, typename TCoeff, typename TRhs>
inline TCoeff VeriPB::Constraint<TLit, TCoeff, TRhs>::coefficient(const litindex& idx) const {
    return _linterm->coefficient(idx);
}
template <typename TLit, typename TCoeff, typename TRhs>
inline TRhs VeriPB::Constraint<TLit, TCoeff, TRhs>::rhs() const {
    assert(_linterm->constant() < _rhs);

    return _rhs - _linterm->constant();
}
template <typename TLit, typename TCoeff, typename TRhs>
inline VeriPB::Comparison VeriPB::Constraint<TLit, TCoeff, TRhs>::comparison() const {
    return _comp;
}

template <typename TLit, typename TCoeff, typename TRhs>
inline TRhs VeriPB::Constraint<TLit, TCoeff, TRhs>::sum_of_coefficients() const {
    return _linterm->sum_of_coefficients();
}

template <typename TLit, typename TCoeff, typename TRhs>
inline size_t VeriPB::Constraint<TLit, TCoeff, TRhs>::size() const {
    return _linterm->size();
}

template <typename TLit, typename TCoeff, typename TRhs>
inline void VeriPB::Constraint<TLit, TCoeff, TRhs>::add_literal(const TLit& lit, const TCoeff& coeff){
    _linterm->add_literal(lit, coeff);
}
template <typename TLit, typename TCoeff, typename TRhs>
inline void VeriPB::Constraint<TLit, TCoeff, TRhs>::add_RHS(const TRhs& rhs_to_add){
    _rhs += rhs_to_add;
}
template <typename TLit, typename TCoeff, typename TRhs>
inline void VeriPB::Constraint<TLit, TCoeff, TRhs>::subtract_RHS(const TRhs& rhs_to_subtract){
    _rhs -= rhs_to_subtract;
}
template <typename TLit, typename TCoeff, typename TRhs>
void VeriPB::Constraint<TLit, TCoeff, TRhs>::clear(const bool cardinality_constraint, const TRhs& new_RHS, const Comparison& new_comparison ){
    _linterm->clear(cardinality_constraint, 0);
    _rhs=new_RHS;
    _comp=new_comparison;
}

template <typename TLit, typename TCoeff, typename TRhs>
inline VeriPB::Constraint<TLit, TCoeff, TRhs>::Constraint(const bool cardinality_constraint, const TRhs& rhs, enum Comparison comp) : 
    _linterm(new VeriPB::LinTermBoolVars<TLit, TCoeff, TRhs>(cardinality_constraint)),
    _rhs(rhs),
    _comp(comp),
    _owned(true)
{ }

template <typename TLit, typename TCoeff, typename TRhs>
inline VeriPB::Constraint<TLit, TCoeff, TRhs>::Constraint(LinTermBoolVars<TLit, TCoeff, TRhs>* term, TRhs rhs, enum Comparison comp) :
    _linterm(term),
    _rhs(rhs),
    _comp(comp),
    _owned(false)

{ }
template <typename TLit, typename TCoeff, typename TRhs>
inline VeriPB::Constraint<TLit, TCoeff, TRhs>::Constraint(std::vector<TLit>* lits, std::vector<TCoeff>* coeff, TRhs rhs, enum Comparison comp) :
    _linterm(new VeriPB::LinTermBoolVars<TLit, TCoeff, TRhs>(lits, coeff, 0)),
    _rhs(rhs),
    _comp(comp),
    _owned(false)
{ }
template <typename TLit, typename TCoeff, typename TRhs>
inline VeriPB::Constraint<TLit, TCoeff, TRhs>::Constraint(std::vector<TLit>* lits, TRhs rhs, enum Comparison comp) :
    _linterm(new VeriPB::LinTermBoolVars<TLit, TCoeff, TRhs>(lits)),
    _rhs(rhs),
    _comp(comp),
    _owned(false)
{ }

template <typename TLit, typename TCoeff, typename TRhs>
inline VeriPB::Constraint<TLit, TCoeff, TRhs>::~Constraint(){
    if(_owned)
        delete _linterm;
}
}