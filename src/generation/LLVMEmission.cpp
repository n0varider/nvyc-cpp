#include "LLVMEmission.hpp"
#include "utils/GenerationUtils.hpp"

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
    void compileFunction(std::unique_ptr<NASTNode> node) {
        std::string funcName = node->getData().asString();
        NodeType funcRType = node->getSubnode(1)->getSubnode(0)->getType();
        int rv = node->getSubnode(2)->getSubnode(0)->getSubnode(0)->getData().i32;

        nvyc::GenerationUtils::EmissionBuilder fn("main_module");
        std::vector<llvm::Type*> args;
        std::vector<std::string> names;

        args.push_back(fn.getNativeType(NodeType::INT32));
        names.push_back("var1");

        auto Func = fn.makeFunction(funcName, names, args, funcRType, false);
        auto block = fn.createBlock(Func, "entry");
        fn.setInsertionPoint(block);
        fn.addConstReturnValue(block, rv);


        Func->print(llvm::outs(), nullptr);

    }


}