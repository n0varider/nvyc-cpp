#pragma once

#include <iostream>

namespace nvyc {

    inline static int checkpoint_ = 0;
    inline static bool DEBUG_MODE = false;

    inline static unsigned int DEBUG_FLAG = 0;
    static constexpr int DEBUG_GEN         = 1 << 0;
    static constexpr int DEBUG_LEX         = 1 << 1;
    static constexpr int DEBUG_PARSE       = 1 << 2;
    static constexpr int DEBUG_EMISSION    = 1 << 3;

    inline void enable_debugging(unsigned int flag) {
        DEBUG_MODE = true;
        DEBUG_FLAG = flag;
    }

    inline void disable_debugging() {
        DEBUG_MODE = false;
        DEBUG_FLAG = 0;
    }

    template <typename T>
    inline void debug(const T& msg) {
        if(DEBUG_MODE) std::cout << "Debug Message: " << msg << std::endl;
    }

    inline void checkpoint() {
        if(DEBUG_MODE) std::cout << "Checkpoint " << checkpoint_++ << std::endl;
    }

    inline void reset() {
        checkpoint_ = 0;
    }

}
