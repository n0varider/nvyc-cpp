#pragma once

#include "data/NodeStream.hpp"
#include "processing/StreamRebuilder.hpp"

using nvyc::NodeStream;

namespace nvyc::Passes {

    class StreamValidationPass {
        private:
            nvyc::Processing::StreamRebuilder& rebuilder;

        public:
            StreamValidationPass(nvyc::Processing::StreamRebuilder& rb) : rebuilder(rb) {}
            bool validTokens(NodeStream& stream);

    };

}