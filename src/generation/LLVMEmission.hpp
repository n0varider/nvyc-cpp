#pragma once

#include "data/NASTNode.hpp"
#include "data/NodeType.hpp"
#include "utils/EmissionBuilder.hpp"
#include "error/Debug.hpp"

namespace nvyc {

    static constexpr int EXPR_ARITH = 0;
    static constexpr int EXPR_LOGIC = 1;

    void compile(nvyc::EmissionBuilder* mod, const std::vector<std::unique_ptr<NASTNode>>& nodes);
    void compileNode(nvyc::EmissionBuilder* mod, const NASTNode* node);

    void compileFunction(nvyc::EmissionBuilder* mod, const NASTNode* node);
    void compileVardef(nvyc::EmissionBuilder* mod, const NASTNode* node);
    llvm::Value* getValue(nvyc::EmissionBuilder* mod, NodeType type, const Value v);
    void compileNative(std::unique_ptr<NASTNode> node);
    void compileFunctionCall(std::unique_ptr<NASTNode> node);
    void compileAssign(std::unique_ptr<NASTNode> node);
    void compileReturn(std::unique_ptr<NASTNode> node);
    void compileStruct(std::unique_ptr<NASTNode> node);
    void compileConditional(std::unique_ptr<NASTNode> node);
    void compileForLoop(std::unique_ptr<NASTNode> node);
    void compileWhileLoop(std::unique_ptr<NASTNode> node);
    
    llvm::Value* compileExpression(nvyc::EmissionBuilder* mod, const NASTNode* node, int exprType);
    llvm::Value* compileReturn(nvyc::EmissionBuilder* mod, const NASTNode* node);

    // Utility

}