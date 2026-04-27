#pragma once

#include "data/NodeType.hpp"
#include "BoxType.hpp"
#include <cstddef>
#include <string>

using nvyc::NodeType;
using nvyc::BoxType;

namespace nvyc::Memory {

    struct MemoryBox {
        size_t memorySize;
        NodeType dataType;
        std::string dataName;
        BoxType currentOwnedType;
    }

}