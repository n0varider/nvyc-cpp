#pragma once

#include "BoxHandle.hpp"

using nvyc::Memory::BoxHandle;

namespace nvyc::Memory {

    struct HeapVariable {
        BoxHandle handle;
        
    };

}