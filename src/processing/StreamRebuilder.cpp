#include "StreamRebuilder.hpp"

#include "data/NodeStream.hpp"
#include "data/NodeType.hpp"
#include "data/Symbols.hpp"
#include "utils/StringUtils.hpp"
#include <sstream>
#include <string>
#include <cstddef>
#include <vector>

namespace nvyc::Processing {

    
    std::string StreamRebuilder::getErrorLocation(size_t idx, size_t charAt) {
        std::stringstream ss;

        // Add the line in question
        std::string line = lines_ref[idx];

        ss << nvyc::StringUtils::trim(line) << "\n";
        for(size_t i = 0; i < charAt; i++) {
            ss << " ";
        }
        ss << "^";

        return ss.str();
    }
}