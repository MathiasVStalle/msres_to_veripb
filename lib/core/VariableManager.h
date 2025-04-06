#ifndef VeriPB_VariableManager_h
#define VeriPB_VariableManager_h

#include "VeriPbSolverTypes.h"
#include <sstream>
#include <cassert>

#ifndef INIT_NAMESTORAGE
#define INIT_NAMESTORAGE 500
#endif

namespace VeriPB {

class VarManager {
public:
    virtual void write_var_name(const VeriPB::Var&, std::ostream*, bool add_prefix_space=false);
    virtual std::string var_name(const VeriPB::Var&);
    virtual void write_literal(const VeriPB::Lit&, std::ostream*, bool add_prefix_space=false);
    virtual std::string literal_to_string(const VeriPB::Lit&);
    virtual void write_var_to_lit(const VeriPB::Var&, const VeriPB::Lit&, std::ostream*, bool write_arrow=false, bool add_prefix_space=false);
    virtual void write_var_to_bool(const VeriPB::Var&, const bool, std::ostream*, bool write_arrow=false, bool add_prefix_space=false);

    void set_number_original_variables(size_t n);
    virtual bool is_aux_var(const VeriPB::Var&);

    VeriPB::Var new_variable_only_in_proof();
    size_t get_number_variables_only_in_proof();
private: 
    VeriPB::VarIdx _n_vars_only_known_in_proof=0;
    size_t _n_orig_vars = 0;
};

class VarManagerWithVarRewriting : public VarManager {
public: 
    virtual void write_var_name(const VeriPB::Var&, std::ostream*, bool add_prefix_space=false) override;
    virtual std::string var_name(const VeriPB::Var&) override;
    virtual void write_literal(const VeriPB::Lit&, std::ostream*, bool add_prefix_space=false) override;
    virtual std::string literal_to_string(const VeriPB::Lit&) override;
    virtual void write_var_to_lit(const VeriPB::Var&, const VeriPB::Lit&, std::ostream*, bool write_arrow=false, bool add_prefix_space=false) override;
    virtual void write_var_to_bool(const VeriPB::Var&, const bool, std::ostream*, bool write_arrow=false, bool add_prefix_space=false) override;


    void store_variable_name(const VeriPB::Var&, const std::string&);
    void store_rewrite_var_by_lit(const VeriPB::Var&, const VeriPB::Lit&);
private:
    bool has_meaningful_name(const VeriPB::Var&);
    bool needs_rewrite(const VeriPB::Var&);
    VeriPB::Lit lit_to_rewrite_to(const VeriPB::Lit&);

    std::vector<bool> _solverVarsSpecialNameFlag;
    std::vector<bool> _onlyproofVarsSpecialNameFlag;
    std::vector<std::string> _solverVarsNameStorage; 
    std::vector<std::string> _proofVarsNameStorage;

    std::vector<bool> _solverVarsRewriteFlag;
    std::vector<bool> _proofVarsRewriteFlag;
    std::vector<VeriPB::Lit> _solverVarsRewriteStorage;
    std::vector<VeriPB::Lit> _proofVarsRewriteStorage;
};


}
#endif