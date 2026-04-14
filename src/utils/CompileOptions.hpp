#pragma once

#include "error/Debug.hpp"
#include "error/Error.hpp"
#include <string>
#include <vector>

namespace nvyc {
    class CompileOptions {
        private:
            bool debug = false;
            unsigned int debug_flags = 0;
            bool emit_ll = false;
            bool emit_o = false;
            bool emit_asm = false;
            std::vector<std::string> inputFiles;
            std::string outputFile;
            char** options;
            int argCount;

        public:
            CompileOptions(int count, char* o[]) 
            : argCount(count), options(o) 
            {
                for(int i = 1; i < argCount; i++) {
                    std::string val = options[i];

                    if(val == "-o") {
                        if(i + 1 < argCount) {
                            outputFile = options[i+1];
                            i++;
                        }
                        else nvyc::Error::nvyerr_failcompile(1, "Output file not provided. Please use -o <file>");
                    }

                    else if(val == "-emit-ll") emit_ll = true;
                    else if(val == "-emit-o") emit_o = true;
                    else if(val == "-emit-S") emit_asm = true;
                    else if(val.starts_with("-debug")) {
                        nvyc::enable_debugging(9);
                        debug = true;
                    }

                    else inputFiles.push_back(options[i]);
                }
            }

            bool get_emit_ll() {
                return emit_ll;
            }

            bool get_emit_o() {
                return emit_o;
            }

            bool get_debug() {
                return debug;
            }

            bool get_emit_S() {
                return emit_asm;
            }

            std::string& getOutput() {
                return outputFile;
            }

            std::vector<std::string>& getInput() {
                return inputFiles;
            }

    }; // class CompileOptions
} // namespace nvyc