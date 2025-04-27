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

namespace convertor {
    class ProofConvertor
    {
        private:
            std::string output_file;
            parser::MSResParser msres_parser;

            // (n, c) where n is the constraint id and c is the clause
            std::unordered_map<uint32_t, cnf::Clause> wcnf_clauses;
            std::unordered_map<uint32_t, VeriPB::Lit> vars;
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
            ProofConvertor(const std::string wcnf_file, const std::string msres_file, const std::string output_file);

            /**
             * Destructor for ProofConvertor.
             */	
            ~ProofConvertor();

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
             * Reifies the original clauses and stores the reification in the proof logger.
             */
            void reificate();

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

            VeriPB::constraintid claim_1(
                const uint32_t clause_id_1,
                const uint32_t clause_id_2,
                const cnf::ResRule& rule,
                const std::vector<cnf::Clause>& new_clauses
            );

            VeriPB::constraintid claim_2(
                uint32_t clause_id_1,
                uint32_t clause_id_2,
                const cnf::ResRule& rule,
                const std::vector<cnf::Clause>& new_clauses
            );


            VeriPB::constraintid claim_1_step_1(
                VeriPB::CuttingPlanesDerivation& cpder, 
                VeriPB::Lit x,
                VeriPB::Lit s3, 
                std::vector<int32_t>& literals_1, 
                std::vector<int32_t>& literals_2
            );

            std::vector<VeriPB::constraintid> claim_1_step_2(
                VeriPB::CuttingPlanesDerivation& cpder,
                VeriPB::Lit x,
                VeriPB::Lit s2,
                VeriPB::Lit s3,
                std::vector<int32_t>& literals_1, 
                std::vector<int32_t>& literals_2
            );

            VeriPB::constraintid claim_1_contradiction(
                VeriPB::CuttingPlanesDerivation& cpder,
                VeriPB::Lit x,
                VeriPB::Lit s2,
                VeriPB::Lit s3,
                std::vector<int32_t>& literals_1, 
                std::vector<int32_t>& literals_2,
                std::vector<VeriPB::constraintid>& subclaims
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
    };
}

#endif