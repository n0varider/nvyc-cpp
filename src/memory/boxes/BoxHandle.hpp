#pragma once

#include "BoxType.hpp"
#include "MemoryBox.hpp"

using nvyc::Memory::MemoryBox;
using nvyc::BoxType;

namespace nvyc::Memory {

    struct BoxHandle {
        bool canMoveBox;
        bool isOwner;
        MemoryBox box;
    };
}