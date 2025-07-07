#ifndef PROOFCONVERTOR_H
#define PROOFCONVERTOR_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

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

            // (n, c) where n is the constraint id and c is the clause
            std::unordered_map<uint32_t, cnf::Clause> wcnf_clauses;
            std::unordered_map<uint32_t, VeriPB::Lit> vars;

            std::unordered_map<cnf::Clause, uint32_t> constraint_ids;
            std::unordered_map<uint32_t, VeriPB::Lit> blocking_vars;

            // The amount of constraints in each partial proof (reification of the wcnf clauses not included)
            std::vector<uint32_t> proof_sizes;

            VeriPB::VarManagerWithVarRewriting var_mgr;
            VeriPB::ProofloggerOpt<VeriPB::Lit, uint32_t, uint32_t> *pl;

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
            void write_new_clauses(const cnf::Rule *rule);

            
            void assemble_proof(
                VeriPB::constraintid claim_1,
                VeriPB::constraintid claim_2,
                VeriPB::constraintid claim_3,
                VeriPB::constraintid claim_4,
                uint32_t clause_id_1,
                uint32_t clause_id_2,
                uint32_t num_new_clauses
            );

            void change_objective(
                uint32_t clause_id_1,
                uint32_t clause_id_2,
                uint32_t num_new_clauses
            );

            std::vector<VeriPB::Lit> get_total_vars(
                const std::vector<int32_t>& literals_1, 
                const std::vector<int32_t>& literals_2
            );

            void clause_to_constraint(const cnf::Clause &clause, VeriPB::Constraint<VeriPB::Lit, uint32_t, uint32_t> &C);
        };
}

#endif