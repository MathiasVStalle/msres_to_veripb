#include "VariableManager.h"

/**********
 * Implementation for default VarManager
 */

namespace VeriPB{

void VarManager::write_var_name(const VeriPB::Var& var, std::ostream* s, bool add_prefix_space){
    if(add_prefix_space)
        *s << ' ';
    if(var.only_known_in_proof)
        *s << "_p";
    else if(is_aux_var(var))
        *s << 'y';
    else
        *s << 'x';
    VeriPB::write_number(varidx(var), s, false);
}

std::string VarManager::var_name(const VeriPB::Var& var){
    std::string prefix;
    if(var.only_known_in_proof)
        prefix =  "_p";
    else if(is_aux_var(var))
        prefix =  'y';
    else
        prefix = 'x';
    return prefix + VeriPB::number_to_string<VeriPB::VarIdx>(varidx(var));
}


void VarManager::write_literal(const VeriPB::Lit& lit, std::ostream* s, bool add_prefix_space){
    if(add_prefix_space)
        *s << ' ';
    if(is_negated(lit))
        *s << '~';
    write_var_name(lit.v, s);
}

std::string VarManager::literal_to_string(const VeriPB::Lit& lit){
    return (is_negated(lit) ? "~" : "") + var_name(lit.v);
}

void VarManager::write_var_to_lit(const VeriPB::Var& var, const VeriPB::Lit& lit, std::ostream* s, bool write_arrow, bool add_prefix_space){
    write_var_name(var, s, add_prefix_space);
    if(write_arrow)
        *s << " ->";
    write_literal(lit, s, true);
}

void VarManager::write_var_to_bool(const VeriPB::Var& var, const bool val, std::ostream* s, bool write_arrow, bool add_prefix_space){
    write_var_name(var, s, add_prefix_space);
    if(write_arrow)
        *s << " ->";
    *s << ' ' << val;
}

void VarManager::set_number_original_variables(size_t n){
    assert(_n_orig_vars == 0);
    _n_orig_vars = n;
}

bool VarManager::is_aux_var(const VeriPB::Var& var){
    return ((var.v > _n_orig_vars) && _n_orig_vars > 0) || var.only_known_in_proof;
}

VeriPB::Var VarManager::new_variable_only_in_proof(){
    return {.v=++_n_vars_only_known_in_proof, .only_known_in_proof=true};
}

size_t VarManager::get_number_variables_only_in_proof(){
    return _n_vars_only_known_in_proof;
}

/***********
 * Implementation for VarManagerWithVarRewriting
 */

void VarManagerWithVarRewriting::write_var_name(const VeriPB::Var& var, std::ostream* s, bool add_prefix_space){
    if(has_meaningful_name(var)){
        if(add_prefix_space)
            *s << ' ';

        if(var.only_known_in_proof)
            *s << _proofVarsNameStorage[var.v];
        else
            *s << _solverVarsNameStorage[var.v];
    }
    else{
        VarManager::write_var_name(var, s, add_prefix_space);
    }
}

std::string VarManagerWithVarRewriting::var_name(const VeriPB::Var& var){
    if(has_meaningful_name(var)){
        if(var.only_known_in_proof)
            return _proofVarsNameStorage[var.v];
        else
            return _solverVarsNameStorage[var.v];
    }
    else{
        return VarManager::var_name(var);
    }
}

VeriPB::Lit VarManagerWithVarRewriting::lit_to_rewrite_to(const VeriPB::Lit& lit){
    VeriPB::Lit l = lit;
    while(true) {
        VeriPB::Var v = l.v;

        if(!needs_rewrite(v)) {
           return lit;
        }

        const std::vector<VeriPB::Lit>* rewriteStorage = v.only_known_in_proof ? &_proofVarsRewriteStorage : &_solverVarsRewriteStorage;
        const VeriPB::Lit lit_to_rewrite_to = is_negated(lit) ? neg((*rewriteStorage)[v.v]) : (*rewriteStorage)[v.v];

        if(lit_to_rewrite_to.v == v) {
            return lit;
        }
        l = lit_to_rewrite_to;
    }
}

void VarManagerWithVarRewriting::write_literal(const VeriPB::Lit& lit, std::ostream* s, bool add_prefix_space){
    if(add_prefix_space)
        *s << ' ';

    if(!needs_rewrite(lit.v)) // Typically, literals will not have to be rewritten. Therefore, in that case, write the literal as fast as possible.
        VarManager::write_literal(lit_to_rewrite_to(lit), s, add_prefix_space);
    else
        VarManager::write_literal(lit_to_rewrite_to(lit), s, add_prefix_space);
}

std::string VarManagerWithVarRewriting::literal_to_string(const VeriPB::Lit& lit){

    return VarManager::literal_to_string(lit_to_rewrite_to(lit));
}

void VarManagerWithVarRewriting::write_var_to_lit(const VeriPB::Var& var, const VeriPB::Lit& lit, std::ostream* s, bool write_arrow, bool add_prefix_space){
    write_var_name(var, s, add_prefix_space);
    if(write_arrow)
        *s << " ->";
    VeriPB::Lit rewritten_lit = lit_to_rewrite_to({.v=var, .negated=false});
    VeriPB::Lit lit_to_write = {.v=lit.v, .negated=(rewritten_lit.negated ? !lit.negated : lit.negated)};
    write_literal(lit_to_write, s, true);
}   

void VarManagerWithVarRewriting::write_var_to_bool(const VeriPB::Var& var, const bool val, std::ostream*, bool write_arrow, bool add_prefix_space){
    VeriPB::Lit rewritten_lit = lit_to_rewrite_to({.v=var, .negated=false});
    
}


void VarManagerWithVarRewriting::store_variable_name(const VeriPB::Var& var, const std::string& name){
    std::vector<std::string>* nameStorage = var.only_known_in_proof ? &_proofVarsNameStorage : &_solverVarsNameStorage;
    std::vector<bool>* meaningfulnameFlag = var.only_known_in_proof ? &_onlyproofVarsSpecialNameFlag : &_solverVarsSpecialNameFlag;

    // Increase storage if necessary.
    if(var.v >= nameStorage->size() && INIT_NAMESTORAGE > var.v ){
        nameStorage->resize(INIT_NAMESTORAGE);
    }
    else if(var.v >= nameStorage->size()) {
        nameStorage->resize(2 * var.v);
    }
    if(var.v >= meaningfulnameFlag->size() && INIT_NAMESTORAGE > var.v ){
        meaningfulnameFlag->resize(INIT_NAMESTORAGE);
    }
    else if(var.v >= meaningfulnameFlag->size()) {
        meaningfulnameFlag->resize(2 * var.v);
    }

    (*meaningfulnameFlag)[var.v] = true;
    (*nameStorage)[var.v] = name;
}
void VarManagerWithVarRewriting::store_rewrite_var_by_lit(const VeriPB::Var& var, const VeriPB::Lit& lit){
    std::vector<VeriPB::Lit>* rewriteStorage = var.only_known_in_proof ? &_solverVarsRewriteStorage : &_proofVarsRewriteStorage;
    std::vector<bool>* rewriteFlag = var.only_known_in_proof ? &_proofVarsRewriteFlag : &_solverVarsRewriteFlag;

    if(var.v >=  rewriteStorage->size() && INIT_NAMESTORAGE > var.v){
        rewriteStorage->resize(INIT_NAMESTORAGE, VeriPB::lit_undef);
    }
    else if(var.v >= rewriteStorage->size()){
        rewriteStorage->resize(2 * var.v, VeriPB::lit_undef);
    }

    if(var.v >=  rewriteFlag->size() && INIT_NAMESTORAGE > var.v){
        rewriteFlag->resize(INIT_NAMESTORAGE);
    }
    else if(var.v >= rewriteFlag->size()){
        rewriteFlag->resize(2 * var.v);
    }

    (*rewriteFlag)[var.v] = true;
    (*rewriteStorage)[var.v] = lit;
}

bool VarManagerWithVarRewriting::has_meaningful_name(const VeriPB::Var& var){
    std::vector<bool>* meaningfulnameFlag = var.only_known_in_proof ? &_onlyproofVarsSpecialNameFlag : &_solverVarsSpecialNameFlag;

    return var.v < meaningfulnameFlag->size() && meaningfulnameFlag->at(var.v);
}

bool VarManagerWithVarRewriting::needs_rewrite(const VeriPB::Var& var){
    std::vector<bool>* rewriteFlag = var.only_known_in_proof ? &_solverVarsRewriteFlag : &_proofVarsRewriteFlag;

    return var.v < rewriteFlag->size() && rewriteFlag->at(var.v);
}
}