#include "LLVMEmission.hpp"
#include "utils/EmissionBuilder.hpp"
#include <unordered_map>
#include <cstdint>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>

namespace nvyc {


    /*
    Head is at Node(...)
    Node(FUNCTION, _nvylang_test_4test_4)
    -- Node(FUNCTIONPARAM, VOID)
    -- Node(FUNCTIONRETURN, VOID)
           -- Node(INT32_T, VOID)
    -- Node(FUNCTIONBODY, VOID)
           -- Node(RETURN, VOID)
                  -- Node(INT32, 12)

    */

    void compileNode(nvyc::EmissionBuilder* mod, const NASTNode* node) {
        NodeType type = node->getType();
        switch(type) {
            case NodeType::FUNCTION:
                compileFunction(mod, node);
                break;
            case NodeType::VARDEF:
                compileVardef(mod, node);
                break;
            case NodeType::ADD:
                compileExpression(mod, node, 0);
                break;
        }
    }


    void compile(nvyc::EmissionBuilder* mod, const std::vector<std::unique_ptr<NASTNode>>& nodes) {

        for(auto& node : nodes) {
            compileNode(mod, node.get());
        }

        mod->getModule()->print(llvm::outs(), nullptr);
    }



    void compileFunction(nvyc::EmissionBuilder* mod, const NASTNode* node) {
        std::string funcName = node->getData().asString();
        NodeType funcRType = node->getSubnode(1)->getSubnode(0)->getType();
        int rv = node->getSubnode(2)->getSubnode(0)->getSubnode(0)->getData().i32;

        std::vector<llvm::Type*> args;
        std::vector<std::string> names;

        auto Func = mod->makeFunction(funcName, names, args, funcRType, false);
        auto block = mod->createBlock(Func, "entry");
        mod->setInsertionPoint(block);

        const std::vector<std::unique_ptr<NASTNode>>& bodyNodes = node->getSubnode(2)->getSubnodes();
        for(auto& bodyNode : bodyNodes) {
            compileNode(mod, bodyNode.get());
        }
    }


    llvm::Value* getValue(nvyc::EmissionBuilder* mod, NodeType type, const Value v) {
        llvm::Value* val;
        switch(type) {
            case NodeType::INT32: {
                int32_t value_i32 = v.i32;
                val = llvm::ConstantInt::get(mod->getNativeType(type), value_i32);
                break;
            }
            case NodeType::INT64: {
                int64_t value_i64 = v.i64;
                val = llvm::ConstantInt::get(mod->getNativeType(type), value_i64);
                break;
            }
            case NodeType::FP32: {
                float_t value_f32 = v.f32;
                val = llvm::ConstantFP::get(mod->getNativeType(type), value_f32);
                break;
            }
            case NodeType::FP64: {
                double_t value_f64 = v.f64;
                val = llvm::ConstantFP::get(mod->getNativeType(type), value_f64);
                break;
            }
            case NodeType::VARIABLE: {
                std::string var = v.str;
                NodeType otherType = mod->getSymbols().getVarType(var);
                val = mod->getBuilder().CreateLoad(mod->getNativeType(otherType), mod->getSymbols().getAlloca(var), var + "_val");
                break;
            }
        }

        return val;
    }

    /*
    -- Node(VARDEF, x)
            -- Node(INT32, 12)
    -- Node(VARDEF, y)
            -- Node(ADD, VOID)
                    -- Node(INT32, 1)
                    -- Node(INT32, 2)
    -- Node(VARDEF, z)
            -- Node(ADD, VOID)
                    -- Node(VARIABLE, x)
                    -- Node(VARIABLE, y)
    */
    void compileVardef(nvyc::EmissionBuilder* mod, const NASTNode* node) {
        std::string name = node->getData().str;
        NodeType type;
        
        llvm::Value* var;
        llvm::Value* val;

        if(nvyc::symbols::LITERAL_SYMBOLS.count(type) || type == NodeType::VARIABLE) {
            type = node->getSubnode(0)->getType();
            var = mod->createVariable(name, type);
            val = getValue(mod, type, node->getSubnode(0)->getData());
        }
        
        else if (nvyc::symbols::ARITH_SYMBOLS.count(type)) {
            type = mod->arithmeticPrecedence(node);
            var = mod->createVariable(name, type);
            val = compileExpression(mod, node->getSubnode(0), 0);
            
        }
        mod->storeToVariable(var, val);
    }

    llvm::Value* compileExpression(nvyc::EmissionBuilder* mod, const NASTNode* node, int exprType) {
        NodeType op = node->getType();
        llvm::Value* lhs_llvm;
        llvm::Value* rhs_llvm;

        const NASTNode* lhs = node->getSubnode(0);
        const NASTNode* rhs = node->getSubnode(1);
        NodeType lhsType = lhs->getType();
        NodeType rhsType = rhs->getType();

        std::string lhsVariable = lhs->getData().str;
        std::string rhsVariable = rhs->getData().str;

        if(lhsType == NodeType::VARIABLE) {
            lhs_llvm = mod->getSymbols().getAlloca(lhsVariable);
            lhsType = mod->getSymbols().getVarType(lhsVariable);
            lhs_llvm = mod->getBuilder().CreateLoad(mod->getNativeType(lhsType), lhs_llvm, lhsVariable + "_val");

        }else{
            std::cout << "LITERAL1" << std::endl;
            std::cout << lhs->asString() << std::endl;
            lhs_llvm = getValue(mod, lhsType, lhs->getData());
            if(lhs_llvm) std::cout << "VALID" << std::endl;
        }


        if(rhsType == NodeType::VARIABLE) {
            rhs_llvm = mod->getSymbols().getAlloca(rhsVariable);
            rhsType = mod->getSymbols().getVarType(rhsVariable);
            rhs_llvm = mod->getBuilder().CreateLoad(mod->getNativeType(rhsType), rhs_llvm, rhsVariable + "_val");
        }else{
            std::cout << "LITERAL2" << std::endl;
            std::cout << rhs->asString() << std::endl;
            rhs_llvm = getValue(mod, rhsType, rhs->getData());
            if(rhs_llvm) std::cout << "VALID" << std::endl;
        }

        return mod->getBuilder().CreateAdd(lhs_llvm, rhs_llvm, mod->getAndIncrementRegister());
    
    
    }

}