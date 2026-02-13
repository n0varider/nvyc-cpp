#pragma once

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Constant.h"
#include "data/NASTNode.hpp"
#include "data/NodeType.hpp"
#include "SymbolStorage.hpp"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstddef>

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
            static constexpr int EXPR_NOTFLOAT =    0;
            static constexpr int EXPR_ISFLOAT =     1;
            static constexpr int EXPR_SIGNEDINT =   0;
            static constexpr int EXPR_UNSIGNEDINT = 1;

            static constexpr int CAST_I32_I64 =     0;
            static constexpr int CAST_INT_F32 =     1;
            static constexpr int CAST_INT_F64 =     2;
            static constexpr int CAST_FLOAT_I32 =   3;
            static constexpr int CAST_FLOAT_I64 =   4;


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
            llvm::Value* createArithmeticOperation(NodeType type, bool isFloat, bool isSigned, llvm::Value* lhs, llvm::Value* rhs);
            llvm::Value* createLogicalOperation(NodeType type, bool isFloat, bool isUnsigned, llvm::Value* lhs, llvm::Value* rhs);
            llvm::Value* castNumeric(int castType, llvm::Value* value);
            int getCastType(NodeType from, NodeType to);

            llvm::FunctionType* buildFunction(std::vector<llvm::Type*> args, NodeType type, bool isVariadic);
            void addReturnValue(llvm::BasicBlock* block, llvm::Value* rv);
            llvm::BasicBlock* createBlock(llvm::Function* func, const std::string name);
            llvm::Value* createVariable(const std::string name, NodeType type);
            void setInsertionPoint(llvm::BasicBlock* block);
            llvm::Type* getNativeType(NodeType type);
            void addConstReturnValue(llvm::BasicBlock* block, int i);
            void storeToVariable(llvm::Value* variable, llvm::Value* value);

            // Replacement for getValue() in LLVMEmission
            /*
            template <typename T>
            llvm::Constant* makeConstant(T value) {

                // Check int32/int64
                if constexpr (std::is_integral_v<T>) {
                    size_t size = sizeof(T) * 8;

                    // Signed by default for now
                    return llvm::ConstantInt::get(llvm::IntegerType::get(context, size), value, true);
                } 

                else if constexpr (std::is_floating_point_v<T>) {
                    llvm::Type* type;
                    if constexpr (std::is_same_v<T, float>)            type = llvm::Type::getFloatTy(context);
                    else if constexpr (std::is_same_v<T, double>)      type = llvm::Type::getDoubleTy(context);
                    else if constexpr (std::is_same_v<T, long double>) type = llvm::Type::getFP128Ty(context);
                    return llvm::ConstantFP::get(type, value);
                } 
            }
            */

    };
} // namespace nvyc::GenerationUtils
