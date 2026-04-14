#pragma once

#include <string>
#include <vector>
#include "data/NodeType.hpp"
#include "data/NASTNode.hpp"

using nvyc::NodeType;

namespace nvyc::Passes {

    std::unique_ptr<NASTNode> mangleFunctions(std::unique_ptr<NASTNode> module);
    std::string mangleFunction(const std::string& moduleName, const std::string& functionName, std::vector<NodeType>& args, std::vector<std::string>& names);
    std::string resolveFunctionCall(const NASTNode* node);

}