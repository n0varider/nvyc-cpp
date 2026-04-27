#pragma once

#include <vector>
#include <string>
#include "NodeType.hpp"

namespace nvyc {

    struct FunctionData {
        std::string module;
        std::vector<NodeType> argTypes;
        std::vector<std::string> argNames;
        NodeType returnType;

        FunctionData(
            std::string mod, 
            std::vector<NodeType> argT, 
            std::vector<std::string> argV,
            NodeType ret
        ) :
        module(std::move(mod)), 
        argTypes(std::move(argT)),
        argNames(std::move(argV)), 
        returnType(ret) {}
    };

}