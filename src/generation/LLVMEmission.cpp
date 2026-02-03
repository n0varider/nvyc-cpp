#include "LLVMEmission.hpp"
#include "utils/EmissionBuilder.hpp"
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

    void compile(nvyc::EmissionBuilder* mod, std::unique_ptr<NASTNode> node) {
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
            compile(mod, std::move(node));
        }

        mod->getModule()->print(llvm::outs(), nullptr);
    }



    void compileFunction(nvyc::EmissionBuilder* mod, std::unique_ptr<NASTNode> node) {
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
            compile(mod, std::move(bodyNode));
        }
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
    void compileVardef(nvyc::EmissionBuilder* mod, std::unique_ptr<NASTNode> node) {
        std::string name = node->getData().str;
        NodeType type = node->getSubnode(0)->getType();
        switch(type) {
            case NodeType::INT32:
                llvm::Value* var = mod->createVariable(name, type);
                int value = node->getSubnode(0)->getData().i32;
                llvm::Value* val = llvm::ConstantInt::get(llvm::Type::getInt32Ty(mod->getModule()->getContext()), value);
                mod->storeToVariable(var, val);
                break;
        }
    }

}