#pragma once

#include <unordered_set>
#include <cstddef>

namespace nvyc {

    template <typename T>
    class FunctionConstant {
        private:
            std::unordered_set<T> options;
    
        public:
            FunctionConstant(std::unordered_set<T> fconsts) :
                options(std::move(fconsts)) {}

            void add(const T option) {
                options.insert(option);
            }

            bool isValid(const T option) {
                return options.contains(option);
            }
        
    };
}