#pragma once

#include "PassManager.hpp"
#include "data/NodeStream.hpp"
#include "data/NASTNode.hpp"
#include "ParserPasses.hpp"
#include <vector>
#include <memory>

using nvyc::NodeStream;
using nvyc::NASTNode;

namespace nvyc::Passes {

    bool PassManager::executeLexicalPasses(NodeStream& stream) {
        return 0;
    }

    std::unique_ptr<NASTNode> PassManager::executeParsingPasses(std::unique_ptr<NASTNode> node) {
        node = nvyc::Passes::mangleFunctions(std::move(node));
        return node;
    }

    bool PassManager::executeCompilationPasses(std::vector<std::unique_ptr<NASTNode>>& nodes) {
        return 0;
    }

    
}