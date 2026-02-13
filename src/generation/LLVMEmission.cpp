#include "LLVMEmission.hpp"
#include "utils/EmissionBuilder.hpp"
#include <unordered_map>
#include <cstdint>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>

using CastType = nvyc::EmissionBuilder::CastType;
using NumericType = nvyc::EmissionBuilder::NumericType;

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

    void compileNode(EmissionBuilder* mod, const NASTNode* node) {
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
            case NodeType::RETURN:
                compileReturn(mod, node);
                break;
        }
    }


    void compile(EmissionBuilder* mod, const std::vector<std::unique_ptr<NASTNode>>& nodes) {

        for(auto& node : nodes) {
            compileNode(mod, node.get());
        }

        //mod->getModule()->print(llvm::outs(), nullptr);
    }



    void compileFunction(EmissionBuilder* mod, const NASTNode* node) {
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


    llvm::Value* getValue(EmissionBuilder* mod, NodeType type, const Value v) {
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
    void compileVardef(EmissionBuilder* mod, const NASTNode* node) {
        std::string name = node->getData().str;
        NodeType type = node->getSubnode(0)->getType();

        llvm::Value* var;
        llvm::Value* val;

        if(symbols::LITERAL_SYMBOLS.count(type) || type == NodeType::VARIABLE) {
            var = mod->createVariable(name, type);
            val = getValue(mod, type, node->getSubnode(0)->getData());
        }
        
        else if (symbols::ARITH_SYMBOLS.count(type)) {
            type = mod->arithmeticPrecedence(node);
            var = mod->createVariable(name, type);
            val = compileExpression(mod, node->getSubnode(0), 0);
            
        }
        mod->storeToVariable(var, val);
    }

    llvm::Value* compileExpression(EmissionBuilder* mod, const NASTNode* node, int exprType) {
        NodeType nodeType = node->getType();

        bool isArith = symbols::ARITH_SYMBOLS.count(nodeType);
        bool isLogic = symbols::LOGIC_SYMBOLS.count(nodeType);

        if(symbols::LITERAL_SYMBOLS.count(nodeType)) {
            return getValue(mod, nodeType, node->getData());
        }

        // Load variable
        else if(nodeType == NodeType::VARIABLE) {
            const std::string varName = node->getData().str;
            NodeType varType = mod->getSymbols().getVarType(varName);
            return mod->getBuilder().CreateLoad(mod->getNativeType(varType), mod->getSymbols().getAlloca(varName));
        }

        // Arithmetic & Logical ops
        else if(isArith || isLogic) {
            NodeType op = nodeType;
            NodeType resultType = mod->arithmeticPrecedence(node);

            // In order of LHS, RHS
            llvm::Value* values[2];
            const NASTNode* operands[2] = {node->getSubnode(0), node->getSubnode(1)};
            NodeType types[2] = {operands[0]->getType(), operands[1]->getType()};
            std::string variableNames[2] = {operands[0]->getData().str, operands[1]->getData().str};

            for(int i = 0; i < 2; i++) {
                types[i] = operands[i]->getType();
                NodeType sideType = types[i];
                const std::string sideVariable = variableNames[i];
                llvm::Value* sideValue;

                if(sideType == NodeType::VARIABLE) {
                    sideValue = mod->getSymbols().getAlloca(sideVariable);
                    sideType = mod->getSymbols().getVarType(sideVariable);
                    types[i] = sideType;
                    values[i] = mod->getBuilder().CreateLoad(mod->getNativeType(sideType), sideValue);
                }
                
                else if(symbols::LITERAL_SYMBOLS.count(sideType)) {
                    values[i] = getValue(mod, sideType, operands[i]->getData());
                }

                else {
                    values[i] = compileExpression(mod, operands[i], exprType);
                }

                // Logic for promotion/demotion
                if(types[i] != resultType) {
                    CastType castType = mod->getCastType(types[i], resultType);
                    values[i] = mod->castNumeric(castType, values[i]);
                }

            }

            NumericType mode = mod->getMode(resultType);

            if(isArith)      return mod->createArithmeticOperation(op, mode, values[0], values[1]);
            else if(isLogic) return mod->createLogicalOperation(op, mode, values[0], values[1]);
        }
    
    }

    llvm::Value* compileReturn(EmissionBuilder* mod, const NASTNode* node) {
        const NASTNode* returnValue = node->getSubnode(0);
        NodeType type = returnValue->getType();

        llvm::Value* value = compileExpression(mod, returnValue, 0);
        return mod->getBuilder().CreateRet(value);
    }

}