#include "EmissionBuilder.hpp"

namespace nvyc {

    EmissionBuilder::EmissionBuilder(const std::string& moduleName) :
        llvmContext(),
        builder(llvmContext),
        module(std::make_unique<llvm::Module>(moduleName, llvmContext)),
        name(moduleName)
    {}

    llvm::Module* EmissionBuilder::getModule() {
        return module.get();
    }


    // ----------------------------------------
    //               FUNCTIONS
    // ----------------------------------------

    llvm::Function* EmissionBuilder::makeFunction(const std::string& name, std::vector<std::string>& argNames, std::vector<llvm::Type*> args, NodeType returnType, bool isVariadic) {

        llvm::FunctionType* funcType = EmissionBuilder::buildFunction(args, returnType, isVariadic);
        
        llvm::Function* func = llvm::Function::Create(
            funcType,
            llvm::Function::ExternalLinkage,
            name,
            module.get()
        );

        int idx = 0;
        auto ArgIterator = func->arg_begin();

        for(int i = 0; i < args.size(); i++) {
            llvm::Argument* carg = &*ArgIterator++;
            carg->setName(argNames.at(idx++));
        }
        return func;
    }


    llvm::FunctionType* EmissionBuilder::buildFunction(std::vector<llvm::Type*> args, NodeType type, bool isVariadic) {
        llvm::Type* ntype = EmissionBuilder::getNativeType(type);
        return llvm::FunctionType::get(ntype, args, isVariadic);
    }


    llvm::BasicBlock* EmissionBuilder::createBlock(llvm::Function* func, const std::string name) {
        return llvm::BasicBlock::Create(llvmContext, name, func);
    }

    void EmissionBuilder::setInsertionPoint(llvm::BasicBlock* block) {
        builder.SetInsertPoint(block);
    }











    // ----------------------------------------
    //               EXPRESSIONS
    // ----------------------------------------

    void EmissionBuilder::addReturnValue(llvm::BasicBlock* block, llvm::Value* rv) {
        builder.CreateRet(rv);
    }

    void EmissionBuilder::addConstReturnValue(llvm::BasicBlock* block, int i) {
        llvm::Value* val = llvm::ConstantInt::get(
            llvm::Type::getInt32Ty(llvmContext),
            i,
            true
        );
        builder.CreateRet(val);

    }

    llvm::Value* EmissionBuilder::createVariable(const std::string name, NodeType type) {
        return builder.CreateAlloca(
            llvm::Type::getInt32Ty(llvmContext),
            nullptr,
            name
        );
    }

    void EmissionBuilder::storeToVariable(llvm::Value* variable, llvm::Value* value) {
        builder.CreateStore(value, variable);
    }

    llvm::Type* EmissionBuilder::getNativeType(NodeType type) {
        switch(type) {
            case NodeType::INT32_T:
            case NodeType::INT32:
                return builder.getInt32Ty();
            case NodeType::INT64_T:
            case NodeType::INT64:
                return builder.getInt64Ty();
            case NodeType::FP32_T:
            case NodeType::FP32:
                return builder.getFloatTy();
            case NodeType::FP64_T:
            case NodeType::FP64:
                return builder.getDoubleTy();
            case NodeType::STR:
            case NodeType::STR_T:
                return builder.getPtrTy();
        }
    }
}