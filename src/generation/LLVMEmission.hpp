#pragma once

#include "data/NASTNode.hpp"
#include "data/NodeType.hpp"
#include "utils/EmissionBuilder.hpp"
#include "error/Debug.hpp"

namespace nvyc {

    static constexpr int EXPR_ARITH = 0;
    static constexpr int EXPR_LOGIC = 1;

    void compile(EmissionBuilder* mod, const std::vector<std::unique_ptr<NASTNode>>& nodes);
    void compileNode(EmissionBuilder* mod, const NASTNode* node);

    void compileFunction(EmissionBuilder* mod, const NASTNode* node);
    void compileVardef(EmissionBuilder* mod, const NASTNode* node);
    llvm::Value* getValue(EmissionBuilder* mod, NodeType type, const Value v);
    void compileNative(std::unique_ptr<NASTNode> node);
    void compileFunctionCall(std::unique_ptr<NASTNode> node);
    void compileAssign(std::unique_ptr<NASTNode> node);
    void compileReturn(std::unique_ptr<NASTNode> node);
    void compileStruct(std::unique_ptr<NASTNode> node);
    void compileConditional(std::unique_ptr<NASTNode> node);
    void compileForLoop(std::unique_ptr<NASTNode> node);
    void compileWhileLoop(std::unique_ptr<NASTNode> node);
    
    llvm::Value* compileExpression(EmissionBuilder* mod, const NASTNode* node, int exprType);
    llvm::Value* compileReturn(EmissionBuilder* mod, const NASTNode* node);

    // Utility

}