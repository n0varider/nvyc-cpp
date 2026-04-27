#pragma once

#include <vector>
#include <string>
#include "NodeType.hpp"
#include "FunctionData.hpp"

namespace nvyc {

    struct StructData {
        std::string module;
        std::vector<NodeType> argTypes;
        std::vector<std::string> argNames;
        std::vector<nvyc::FunctionData> functions;
        NodeType returnType;

        StructData(
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