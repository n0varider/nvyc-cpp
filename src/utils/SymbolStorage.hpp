#pragma once

#include "data/NodeType.hpp"
#include <unordered_map>
#include <string>
#include <utility>
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>

using nvyc::NodeType;

namespace nvyc {

    class SymbolStorage {
    private:
        std::unordered_map<std::string, llvm::Value*> variableAlloca;
        std::unordered_map<std::string, llvm::Function*> functionData;
        std::unordered_map<std::string, std::pair<NodeType, llvm::Type*>> variableTypes;
        std::unordered_map<std::string, NodeType> functionTypes;

        std::pair<NodeType, llvm::Type*> makePair(NodeType type, llvm::Type* ty);
        
    public:
        SymbolStorage() {}

        llvm::Value* getAlloca(const std::string& variable);
        void storeAlloca(const std::string variable, llvm::Value* value);

        NodeType getVarNvyType(const std::string& variable);
        llvm::Type* getVarNativeType(const std::string& variable);
        void storeVarType(const std::string variable, NodeType type, llvm::Type* ty);

        NodeType getFunType(const std::string& func);
        void storeFunType(const std::string func, NodeType type);

        //llvm::Function* getFunction(const std::string& func);
        //void storeFunction(const std::string& func, llvm::Function* fptr);
    };

}
