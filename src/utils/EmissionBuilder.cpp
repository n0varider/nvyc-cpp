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

    llvm::IRBuilder<>& EmissionBuilder::getBuilder() {
        return builder;
    }

    std::string EmissionBuilder::getCurrentRegister() {
        return std::to_string(registerId);
    }

    std::string EmissionBuilder::getAndIncrementRegister() {
        return std::to_string(registerId++);
    }

    std::string EmissionBuilder::getPreviousRegister() {
        return std::to_string(registerId - 1);
    }

    nvyc::SymbolStorage& EmissionBuilder::getSymbols() {
        return symbols;
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

        getSymbols().storeFunType(name, returnType);

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
        auto alloca = builder.CreateAlloca(
            getNativeType(type),
            nullptr,
            name
        );
        getSymbols().storeAlloca(name, alloca);
        getSymbols().storeVarType(name, type);
        return alloca;
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

    NodeType EmissionBuilder::arithmeticPrecedence(const NASTNode* node) {
        NodeType type = node->getType();

        // Either a literal, function call, or variable
        if(node->getSubnodes().empty()) {
            if(nvyc::symbols::LITERAL_SYMBOLS.count(type)) return type;
            if(type == NodeType::FUNCTIONCALL) return getSymbols().getFunType(node->getData().str);
            return getSymbols().getVarType(node->getData().str);
        }

        // Struct access
        // ...

        NodeType precedence = NodeType::INT32;
        for(const auto& subnode : node->getSubnodes()) {
            precedence = typePrecedence(precedence, arithmeticPrecedence(subnode.get()));
        }

        return precedence;
    }

    NodeType EmissionBuilder::typePrecedence(NodeType t1, NodeType t2) {
        int t1_p = typeToPrecedence(t1);
        int t2_p = typeToPrecedence(t2);

        if(t1_p > t2_p) return t1;
        else return t2;

    }

    int EmissionBuilder::typeToPrecedence(NodeType type) {
        switch(type) {
            case NodeType::CHAR:    return -1;
            case NodeType::SHORT:   return 0;
            case NodeType::INT32:   return 1;
            case NodeType::INT64:   return 2;
            case NodeType::FP32:    return 3;
            case NodeType::FP64:    return 4;
            default:                return -10;
        }
    }

    NodeType EmissionBuilder::precedenceToType(int precedence) {
        switch(precedence) {
            case -1:    return NodeType::CHAR;
            case 0:     return NodeType::SHORT;
            case 1:     return NodeType::INT32;
            case 2:     return NodeType::INT64;
            case 3:     return NodeType::FP32;
            case 4:     return NodeType::FP64;
            default:    return NodeType::INVALID;
        }
    }

    llvm::Instruction::BinaryOps EmissionBuilder::getInstruction(NodeType type) {
        switch(type) {
            case NodeType::ADD: return llvm::Instruction::Add;
            case NodeType::SUB: return llvm::Instruction::Sub;
            case NodeType::MUL: return llvm::Instruction::Mul;
            case NodeType::DIV: return llvm::Instruction::SDiv;
            default: return llvm::Instruction::Add; // Temporary fallback
        }
    }
}