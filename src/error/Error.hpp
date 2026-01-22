#pragma once

#include <iostream>

namespace nvyc::Error {

    inline const std::string NULLERR = "NULLERR";

    inline void nvyerr_out(const std::string& msg) {
        std::cout << msg << std::endl;
    }

    inline void nvyerr_failcompile(int ec, std::string msg) {
        std::cout << "nvy > Failed to compile with code " << ec << "\n";
        if(msg != NULLERR) std::cout << msg << std::endl;
    }

}