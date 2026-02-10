#pragma once

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "data/NASTNode.hpp"
#include "data/NodeType.hpp"
#include "SymbolStorage.hpp"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

using nvyc::NASTNode;
using nvyc::NodeType;

namespace nvyc {

    class EmissionBuilder {
        private:
            llvm::LLVMContext llvmContext;
            llvm::IRBuilder<> builder;
            std::unique_ptr<llvm::Module> module;
            std::string name;
            int registerId = 0;
            nvyc::SymbolStorage symbols;

        public:
            EmissionBuilder(const std::string& moduleName);

            llvm::Module* getModule();
            llvm::IRBuilder<>& getBuilder();
            nvyc::SymbolStorage& getSymbols();

            std::string getCurrentRegister();
            std::string getAndIncrementRegister();
            std::string getPreviousRegister();
            void storeLocation(std::string);

            llvm::Function* makeFunction(
                const std::string& name, 
                std::vector<std::string>& argNames,
                std::vector<llvm::Type*> args,
                NodeType returnType,
                bool isVariadic
            );

            NodeType typePrecedence(NodeType t1, NodeType t2);
            int lrPrecedence(NodeType t1, NodeType t2);
            NodeType arithmeticPrecedence(const NASTNode* node);
            int typeToPrecedence(NodeType type);
            NodeType precedenceToType(int precedence);


            llvm::FunctionType* buildFunction(std::vector<llvm::Type*> args, NodeType type, bool isVariadic);
            void addReturnValue(llvm::BasicBlock* block, llvm::Value* rv);
            llvm::BasicBlock* createBlock(llvm::Function* func, const std::string name);
            llvm::Value* createVariable(const std::string name, NodeType type);
            void setInsertionPoint(llvm::BasicBlock* block);
            llvm::Type* getNativeType(NodeType type);
            void addConstReturnValue(llvm::BasicBlock* block, int i);
            void storeToVariable(llvm::Value* variable, llvm::Value* value);


    };
} // namespace nvyc::GenerationUtils
