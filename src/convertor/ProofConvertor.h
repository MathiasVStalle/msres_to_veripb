#ifndef PROOFCONVERTOR_H
#define PROOFCONVERTOR_H

#include <string>
#include <vector>

#include "../cnf/Clause.h"

#include "../parser/MSResParser.h"

#include "../../lib/VeriPB_Prooflogger/core/VeriPbSolverTypes.h"
#include "../../lib/VeriPB_Prooflogger/core/MaxSATProoflogger.h"

namespace convertor {
    class ProofConvertor
    {
        private:
            std::string output_file;

            std::vector<cnf::Clause> wcnf_clauses;
            parser::MSResParser msres_parser;

            VeriPB::VarManagerWithVarRewriting var_mgr;
            VeriPB::ProofloggerOpt<VeriPB::Lit, uint32_t, uint32_t> *pl;

            uint32_t varcounter = 0;

        public:
            /**
             * Constructor for ProofConvertor.
             * 
             * @param wcnf_file The name of the WCNF file to parse.
             * @param msres_file The name of the MSRes file to parse.
             */
            ProofConvertor(const std::string wcnf_file, const std::string msres_file);

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
            void write_proof(const cnf::Rule* rule);
    };
}

#endif