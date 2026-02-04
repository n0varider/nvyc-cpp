#pragma once

#include "data/NASTNode.hpp"
#include "data/NodeType.hpp"
#include "utils/EmissionBuilder.hpp"

namespace nvyc {

    static constexpr int EXPR_ARITH = 0;
    static constexpr int EXPR_LOGIC = 1;

    void compile(nvyc::EmissionBuilder* mod, std::vector<std::unique_ptr<NASTNode>>& nodes);
    void compileNode(nvyc::EmissionBuilder* mod, NASTNode* node);

    void compileFunction(nvyc::EmissionBuilder* mod, NASTNode* node);
    void compileVardef(nvyc::EmissionBuilder* mod, NASTNode* node);
    llvm::Value* getValue(nvyc::EmissionBuilder* mod, NodeType type, const Value v);
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