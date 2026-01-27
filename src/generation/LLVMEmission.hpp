#pragma once

#include "data/NASTNode.hpp"
#include "data/NodeType.hpp"
#include "utils/GenerationUtils.hpp"

namespace nvyc {

    void compile(std::vector<std::unique_ptr<NASTNode>> nodes);

    static constexpr int EXPR_ARITH = 0;
    static constexpr int EXPR_LOGIC = 1;

    // Will need to pass builders around to build within same BB
    void compileFunction(std::unique_ptr<NASTNode> node);
    void compileVardef(std::unique_ptr<NASTNode> node);
    void compileNative(std::unique_ptr<NASTNode> node);
    void compileFunctionCall(std::unique_ptr<NASTNode> node);
    void compileAssign(std::unique_ptr<NASTNode> node);
    void compileReturn(std::unique_ptr<NASTNode> node);
    void compileStruct(std::unique_ptr<NASTNode> node);
    void compileExpression(std::unique_ptr<NASTNode> node, int exprType);
    void compileConditional(std::unique_ptr<NASTNode> node);
    void compileForLoop(std::unique_ptr<NASTNode> node);
    void compileWhileLoop(std::unique_ptr<NASTNode> node);

    // Utility

}