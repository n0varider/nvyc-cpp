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

    SymbolStorage& EmissionBuilder::getSymbols() {
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
            if(symbols::LITERAL_SYMBOLS.count(type)) return type;
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

    llvm::Value* EmissionBuilder::createArithmeticOperation(NodeType type, NumericType mode, llvm::Value* lhs, llvm::Value* rhs) {
        llvm::Instruction::BinaryOps op;
        
        switch(type) {
            case NodeType::ADD: {
                if(mode == NumericType::FLOAT)              op = llvm::Instruction::FAdd;
                else                                        op = llvm::Instruction::Add;
                break;
            }
            case NodeType::SUB: {
                if(mode == NumericType::FLOAT)              op = llvm::Instruction::FSub;
                else                                        op = llvm::Instruction::Sub;
                break;
            }
            case NodeType::MUL: {
                if(mode == NumericType::FLOAT)              op = llvm::Instruction::FMul;
                else                                        op = llvm::Instruction::Mul;
                break;
            }
            case NodeType::DIV: {
                if(mode == NumericType::FLOAT)              op = llvm::Instruction::FDiv;
                else if(mode == NumericType::UNSIGNEDINT)   op = llvm::Instruction::UDiv;
                else                                        op = llvm::Instruction::SDiv;
                break;
            }
            default: {
                op = llvm::Instruction::Add;
                break;
            }
        }

        return builder.CreateBinOp(op, lhs, rhs);
    }

    llvm::Value* EmissionBuilder::createLogicalOperation(NodeType type, NumericType mode, llvm::Value* lhs, llvm::Value* rhs) {
        switch(type) {
            case NodeType::LT: {
                if (mode == NumericType::FLOAT)         return builder.CreateFCmpOLT(lhs, rhs);
                if (mode == NumericType::UNSIGNEDINT)   return builder.CreateICmpULT(lhs, rhs);
                                                        return builder.CreateICmpSLT(lhs, rhs);
            }

            case NodeType::LTE: {
                if (mode == NumericType::FLOAT)         return builder.CreateFCmpOLE(lhs, rhs);
                if (mode == NumericType::UNSIGNEDINT)   return builder.CreateICmpULE(lhs, rhs);
                                                        return builder.CreateICmpSLE(lhs, rhs);
            }

            case NodeType::GT: {
                if (mode == NumericType::FLOAT)         return builder.CreateFCmpOGT(lhs, rhs);
                if (mode == NumericType::UNSIGNEDINT)   return builder.CreateICmpUGT(lhs, rhs);
                                                        return builder.CreateICmpSGT(lhs, rhs);
            }

            case NodeType::GTE: {
                if (mode == NumericType::FLOAT)         return builder.CreateFCmpOGE(lhs, rhs);
                if (mode == NumericType::UNSIGNEDINT)   return builder.CreateICmpUGE(lhs, rhs);
                                                        return builder.CreateICmpSGE(lhs, rhs);
            }

        }
    }

    // Does not handle unsigned values yet
    llvm::Value* EmissionBuilder::castNumeric(CastType castType, llvm::Value* value) {
        switch(castType) {
            case CastType::I32_I64: return builder.CreateSExt(value, builder.getInt64Ty());
            case CastType::INT_F32: return builder.CreateSIToFP(value, builder.getFloatTy());
            case CastType::INT_F64: return builder.CreateSIToFP(value, builder.getDoubleTy());
            case CastType::FLOAT_I32: return builder.CreateFPToSI(value, builder.getInt32Ty());
            case CastType::FLOAT_I64: return builder.CreateFPToSI(value, builder.getInt64Ty());
            default: return nullptr; // temp fallback
        }
    }

    EmissionBuilder::CastType EmissionBuilder::getCastType(NodeType from, NodeType to) {
        if(from == NodeType::INT32 && to == NodeType::INT64) return CastType::I32_I64;
        if(from == NodeType::INT32 || from == NodeType::INT64) {
            if(to == NodeType::FP32) return CastType::INT_F32;
            else return CastType::INT_F64;
        }
        if(from == NodeType::FP32 || from == NodeType::FP64) {
            if(to == NodeType::INT32) return CastType::FLOAT_I32;
            else return CastType::FLOAT_I64;
        }
    }

    EmissionBuilder::NumericType EmissionBuilder::getMode(NodeType type) {
        switch(type) {
            case NodeType::INT32:
            case NodeType::INT64:
                return NumericType::SIGNEDINT;
            case NodeType::FP32:
            case NodeType::FP64:
                return NumericType::FLOAT;
            default: return NumericType::SIGNEDINT;
        }
    }
}