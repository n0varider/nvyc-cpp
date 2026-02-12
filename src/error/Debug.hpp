#pragma once

#include <iostream>

#define DEBUG_MODE 1

namespace nvyc {

    inline static int checkpoint_ = 0;


    inline void debug(const std::string& msg) {
        if(DEBUG_MODE) std::cout << "DEBUG: " << msg << std::endl;
    }

    inline void checkpoint() {
        if(DEBUG_MODE) std::cout << "Checkpoint " << checkpoint_++ << std::endl;
    }

    inline void reset() {
        checkpoint_ = 0;
    }

}
