#pragma once

#include <string>
#include <cstddef>

namespace nvyc::StringUtils {

    std::string trim(const std::string& str) {
        size_t start = str.find_first_not_of(' ');
        size_t end = str.find_last_not_of(' ');

        if(start == std::string::npos) return str;

        return str.substr(start, end - start + 1);
    }


}