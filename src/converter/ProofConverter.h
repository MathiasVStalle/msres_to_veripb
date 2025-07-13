#ifndef PROOFCONVERTOR_H
#define PROOFCONVERTOR_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "ProofConstraint.h"
#include "Hash.h"
#include "../cnf/Clause.h"
#include "../cnf/Rule.h"
#include "../cnf/ResRule.h"
#include "../cnf/SplitRule.h"

#include "../parser/MSResParser.h"

#include "VeriPbSolverTypes.h"
#include "MaxSATProoflogger.h"

namespace converter {
    // TODO: Cleanup of the ProofConverter calss
    class ProofConverter
    {
        private:
            std::string output_file;
            parser::MSResParser msres_parser;

            VeriPB::VarManagerWithVarRewriting var_mgr;
            VeriPB::ProofloggerOpt<VeriPB::Lit, uint32_t, uint32_t> *pl;

            std::unordered_map<uint32_t, const cnf::Clause> wcnf_clauses;                                       // Maps clause ID to the clause of the WCNF file
            std::unordered_map<uint32_t, VeriPB::Lit> vars;                                                     // Maps variable ID to the literal
            std::unordered_map<const cnf::Clause*, VeriPB::Lit, ClausePtrHash, ClausePtrEqual> blocking_vars;   // Maps clause to the blocking variable

            std::unordered_map<VeriPB::Lit, VeriPB::constraintid, LitHash, LitEqual> tautologies;               // Maps blocking literal to the unit literal
        
        public:
            /**
             * Constructor for ProofConvertor.
             * 
             * @param wcnf_file The name of the WCNF file to parse.
             * @param msres_file The name of the MSRes file to parse.
             */
            ProofConverter(const std::string wcnf_file, const std::string msres_file, const std::string output_file);

            /**
             * Destructor for ProofConvertor.
             */	
            ~ProofConverter();

            /**
             * Writes the Veripb proof to a file.
             * 
             * @param output_file The name of the output file to write the proof to.
             */
            void write_proof();

        private:
            /**
             * Writes the partial proof of the given rule to the proof logger.
             * 
             * @param rule The rule to write to the proof logger.
             */
            void write_proof(const cnf::Rule *rule); 

            /**
             * Writes the resolution rule to the proof logger.
             * 
             * @param rule The resolution rule to write to the proof logger.
             */
            void write_res_rule(const cnf::ResRule *rule);

            void write_split_rule(const cnf::SplitRule *rule);

            /**
             * Initializes the variables used in the proof conversion.
             *
             * This method extracts the variables from the original clauses and initializes them in the variable manager.
             */
            void initialize_vars();

            /**
             * Reifies the original clauses and stores the reification in the proof logger.
             */
            void reificate_original_clauses();

            /**
             * Applies the given rule and writes the new clauses to the proof logger.
             * 
             * @param rule The rule to apply.
             */
            void write_new_clauses(const cnf::Rule *rule, const std::vector<cnf::Clause> &new_clauses);

            /**
             * Assembles a proof for the given claims and clauses.
             * 
             * @param claim_1 The first claim.
             * @param claim_2 The second claim.
             * @param claim_3 The third claim.
             * @param claim_4 The fourth claim.
             * @param clause_1 The first clause of the applies rule.
             * @param clause_2 The second clause of the applies rule.
             * @param new_clauses The new clauses to be added to the proof by the applied rule.
             */
            void assemble_proof(
                VeriPB::constraintid claim_1,
                VeriPB::constraintid claim_2,
                VeriPB::constraintid claim_3,
                VeriPB::constraintid claim_4,
                const cnf::Clause &clause_1,
                const cnf::Clause &clause_2,
                const std::vector<cnf::Clause> &new_clauses
            );

            void change_objective(const cnf::Clause &clause_1, const cnf::Clause &clause_2, const std::vector<cnf::Clause> &new_clauses);

            void clause_to_constraint(const cnf::Clause &clause, VeriPB::Constraint<VeriPB::Lit, uint32_t, uint32_t> &C);

            VeriPB::constraintid proof_by_contradiction(VeriPB::constraintid claim_1, VeriPB::constraintid claim_2, VeriPB::Constraint<VeriPB::Lit, uint32_t, uint32_t> &C);
        };
}

#endif