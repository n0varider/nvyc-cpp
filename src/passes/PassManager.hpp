#pragma once

#include "data/NodeStream.hpp"
#include "data/NASTNode.hpp"
#include "StreamValidationPass.hpp"
#include "processing/StreamRebuilder.hpp"
#include <vector>
#include <string>
#include <memory>

using nvyc::NodeStream;
using nvyc::NASTNode;

namespace nvyc::Passes {

    class PassManager {
        private:
            nvyc::Processing::StreamRebuilder& rebuilder;
            //nvyc::Passes::StreamValidationPass svp(rebuilder);

            // Lexical

            // Parser

            // Compilation

        public:
            PassManager(nvyc::Processing::StreamRebuilder& rb) : rebuilder(rb) {};

            bool executeLexicalPasses(NodeStream& stream);
            std::unique_ptr<NASTNode> executeParsingPasses(std::unique_ptr<NASTNode> node);
            bool executeCompilationPasses(std::vector<std::unique_ptr<NASTNode>>& nodes);
    };
}