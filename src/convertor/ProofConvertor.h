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

#include "../../lib/VeriPB_Prooflogger/core/VeriPbSolverTypes.h"
#include "../../lib/VeriPB_Prooflogger/core/MaxSATProoflogger.h"

namespace convertor {
    class ProofConvertor
    {
        private:
            std::string output_file;

            std::unordered_map<uint32_t, cnf::Clause> wcnf_clauses;

            parser::MSResParser msres_parser;

            VeriPB::VarManagerWithVarRewriting var_mgr;
            VeriPB::ProofloggerOpt<VeriPB::Lit, uint32_t, uint32_t> *pl;

            std::unordered_map<uint32_t, VeriPB::Lit> vars;

            void write_proof(const cnf::Rule *rule);
            void write_res_rule(const cnf::ResRule *rule);
            void write_split_rule(const cnf::SplitRule *rule);

            void reificate();

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
            
    };
}

#endif