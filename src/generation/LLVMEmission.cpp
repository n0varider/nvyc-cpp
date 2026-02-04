#include "LLVMEmission.hpp"
#include "utils/EmissionBuilder.hpp"
#include <unordered_map>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <cstdint>

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

    void compileNode(nvyc::EmissionBuilder* mod, NASTNode* node) {
        NodeType type = node->getType();
        switch(type) {
            case NodeType::FUNCTION:
                compileFunction(mod, std::move(node));
                break;
            case NodeType::VARDEF:
                compileVardef(mod, std::move(node));
                break;
        }
    }


    void compile(nvyc::EmissionBuilder* mod, std::vector<std::unique_ptr<NASTNode>>& nodes) {

        for(auto& node : nodes) {
            compileNode(mod, node.get());
        }

        mod->getModule()->print(llvm::outs(), nullptr);
    }



    void compileFunction(nvyc::EmissionBuilder* mod, NASTNode* node) {
        std::string funcName = node->getData().asString();
        NodeType funcRType = node->getSubnode(1)->getSubnode(0)->getType();
        int rv = node->getSubnode(2)->getSubnode(0)->getSubnode(0)->getData().i32;

        std::vector<llvm::Type*> args;
        std::vector<std::string> names;

        auto Func = mod->makeFunction(funcName, names, args, funcRType, false);
        auto block = mod->createBlock(Func, "entry");
        mod->setInsertionPoint(block);

        std::vector<std::unique_ptr<NASTNode>>& bodyNodes = node->getSubnode(2)->getSubnodes();
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
                NodeType otherType = mod->getSymbols().getType(var);
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
    void compileVardef(nvyc::EmissionBuilder* mod, NASTNode* node) {
        std::string name = node->getData().str;
        NodeType type = node->getSubnode(0)->getType();

        llvm::Value* var = mod->createVariable(name, type);
        llvm::Value* val;



        if(nvyc::symbols::LITERAL_SYMBOLS.count(type) || type == NodeType::VARIABLE) {
            val = getValue(mod, type, node->getSubnode(0)->getData());
        }
        
        else if (nvyc::symbols::ARITH_SYMBOLS.count(type)) {
            auto node1 = node->getSubnode(0)->getSubnode(0);
            auto node2 = node->getSubnode(0)->getSubnode(1);
            NodeType t1 = node1->getType();
            NodeType t2 = node2->getType();
            llvm::Value* val1 = getValue(mod, t1, node1->getData());
            llvm::Value* val2 = getValue(mod, t2, node2->getData());
            val = mod->getBuilder().CreateAdd(val1, val2, mod->getAndIncrementRegister());
        }
        mod->storeToVariable(var, val);
    }

}