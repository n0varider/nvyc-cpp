#pragma once

#include "data/NodeStream.hpp"
#include "data/NodeType.hpp"
#include "data/Symbols.hpp"
#include <sstream>
#include <string>
#include <cstddef>
#include <vector>

using nvyc::NodeStream;
using nvyc::NodeType;

namespace nvyc::Processing {

    class StreamRebuilder {
        private:
            std::vector<std::string>& lines_ref;
        public:
            StreamRebuilder(std::vector<std::string>& lines) : lines_ref(lines) {}

            std::string getErrorLocation(size_t idx, size_t charAt);
            
    };

}