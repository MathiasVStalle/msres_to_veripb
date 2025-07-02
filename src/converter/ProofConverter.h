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

            /**
             * Gives the constraint ids in the veripb proof logger for the given constraints.
             * 
             * @param constraint_1 The MaxSAT resolution constraint id for the first constraint.
             * @param constraint_2 The MaxSAT resolution constraint id for the second constraint.
             * @return A pair of pairs containing the constraint ids in the veripb proof logger. Both left and right implication are included.
             */
            std::pair<
                std::pair<VeriPB::constraintid, VeriPB::constraintid>, 
                std::pair<VeriPB::constraintid, VeriPB::constraintid>
            > 
            get_constraint_ids(const uint32_t constraint_1, const uint32_t constraint_2);

            VeriPB::constraintid claim_type_1(
                const cnf::ResRule& rule
            );

            VeriPB::constraintid claim_2(
                uint32_t clause_id_1,
                uint32_t clause_id_2,
                const cnf::ResRule& rule,
                const std::vector<cnf::Clause>& new_clauses
            );


            VeriPB::constraintid claim_2_step_1(
                VeriPB::CuttingPlanesDerivation& cpder,
                VeriPB::Lit x,
                VeriPB::Lit s3, 
                std::vector<int32_t>& literals_1, 
                std::vector<int32_t>& literals_2
            );

            std::vector<VeriPB::constraintid> claim_2_step_2(
                VeriPB::CuttingPlanesDerivation& cpder,
                VeriPB::Lit x,
                VeriPB::Lit s1,
                VeriPB::Lit s3,
                std::vector<int32_t>& literals_1, 
                std::vector<int32_t>& literals_2
            );

            VeriPB::constraintid claim_2_contradiction(
                VeriPB::CuttingPlanesDerivation& cpder,
                VeriPB::Lit x,
                VeriPB::Lit s1,
                VeriPB::Lit s3,
                std::vector<int32_t>& literals_1, 
                std::vector<int32_t>& literals_2,
                std::vector<VeriPB::constraintid>& subclaims
            );


            VeriPB::constraintid claim_3(
                uint32_t clause_id_1,
                uint32_t clause_id_2,
                const cnf::ResRule& rule,
                const std::vector<cnf::Clause>& new_clauses
            );

            VeriPB::constraintid claim_4(
                uint32_t clause_id_1,
                uint32_t clause_id_2,
                const cnf::ResRule& rule,
                const std::vector<cnf::Clause>& new_clauses
            );

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


            // TODO: active_blocking_vars and active_constraints are of the same size, so they can be combined into one vector
            std::vector<VeriPB::constraintid> build_iterative_subclaims(
                VeriPB::Lit x,
                std::vector<VeriPB::Lit>& total_vars,
                std::vector<VeriPB::Lit>& active_blocking_vars,
                std::vector<VeriPB::constraintid>& active_constraints,
                uint32_t unactive_constraints_amount
            );

            VeriPB::constraintid iterative_proofs_by_contradiction(
                std::vector<int32_t>& active_literals,
                std::vector<VeriPB::Lit>& active_blocking_vars,
                std::vector<VeriPB::constraintid>& subclaims
            );

            // TODO: s2 variable should not be needed
            // TODO: If the total amount of subclaims is large, dynamic allocation should be used
            std::vector<VeriPB::constraintid> build_conjunctive_subclaims(
                VeriPB::Lit x,
                VeriPB::Lit s2,
                std::vector<VeriPB::Lit>& total_vars,
                std::vector<VeriPB::constraintid>& active_constraints,
                uint32_t unactive_constraints_amount
            );


            std::vector<VeriPB::Lit> get_total_vars(
                const std::vector<int32_t>& literals_1, 
                const std::vector<int32_t>& literals_2
            );

            VeriPB::constraintid weaken_all_except(VeriPB::constraintid id, std::vector<VeriPB::Lit> &literals, uint32_t except);
            VeriPB::constraintid weaken_all_except(
                VeriPB::constraintid id, 
                std::vector<VeriPB::Lit>& literals, 
                uint32_t begin,
                uint32_t end
            );

            VeriPB::constraintid add_all(std::vector<VeriPB::constraintid> constraints);
            VeriPB::constraintid add_all_from_literal(std::vector<VeriPB::constraintid> constraints, VeriPB::Lit var);
            VeriPB::constraintid add_all_prev(int32_t range);
            VeriPB::constraintid add_all_prev_from_literal(int32_t range, VeriPB::Lit var);

            VeriPB::constraintid build_proof_by_contradiction(VeriPB::Constraint<VeriPB::Lit, uint32_t, uint32_t>& C, VeriPB::constraintid claim_1, VeriPB::constraintid claim_2);
            VeriPB::constraintid build_proof_by_contradiction(VeriPB::Constraint<VeriPB::Lit, uint32_t, uint32_t>& C, std::vector<VeriPB::constraintid>& claims);
        };
}

#endif