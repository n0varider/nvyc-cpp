#include "ParserPasses.hpp"

#include <string>
#include <sstream>
#include <vector>
#include <unordered_set>
#include "data/NodeType.hpp"
#include "data/Symbols.hpp"
#include "data/NASTNode.hpp"
#include "data/Value.hpp"
#include "processing/StreamRebuilder.hpp"

using nvyc::NodeType;
using nvyc::NASTNode;

using nvyc::NodeType;

namespace nvyc::Passes {

    std::unordered_set<std::string> functionNames;

    std::unique_ptr<NASTNode> mangleFunctions(std::unique_ptr<NASTNode> module) {
        if(module->getType() != NodeType::MODULE) return module;
        const std::string moduleName = module->getData().asString();
        for(const auto& subnode : module->getSubnodes()) {
            NodeType ty = subnode->getType();

            if(ty == NodeType::FUNCTION) {
                std::string currentName = subnode->getData().asString();
                std::vector<NodeType> argTypes;
                std::vector<std::string> argNames;

                for(const auto& paramNode : subnode->getSubnode(0)->getSubnodes()) {
                    argTypes.push_back(paramNode->getType());
                    argNames.push_back(paramNode->getData().asString());
                }
                
                std::string newName = mangleFunction(moduleName, currentName, argTypes, argNames);
                if(functionNames.contains(newName)) {
                    std::string prototype = symbols::buildFunctionPrototype(currentName, argTypes);
                    nvyc::Error::nvyerr_failcompile(1, "Duplicate function definition found for " + prototype);
                }
                functionNames.insert(newName);
                subnode->setValue(Value(newName));
            }
        }
        return module;
    }

    std::string mangleFunction(const std::string& moduleName, const std::string& functionName, std::vector<NodeType>& args, std::vector<std::string>& names) {
        std::stringstream ss;
        size_t moduleNameLength = moduleName.length();
        size_t functionNameLength = functionName.length();

        // _nvy[moduleNameLength][moduleName][functionNameLength][functionName][arg identifiers]
         // Build function signature
        ss << "_nvy" << moduleNameLength << moduleName << functionNameLength << functionName;
        int idx = 0;
        for(NodeType ty : args) {
            ss << symbols::charTypeId(ty);
            if(ty == NodeType::STRUCT) ss << names[idx];
            idx++;
        }

        return ss.str();
    }

    /*
    Node(MODULE, main)
    -- Node(FUNCTION, main)
           -- Node(FUNCTIONPARAM, VOID)
           -- Node(FUNCTIONRETURN, VOID)
                  -- Node(INT64_T, VOID)
           -- Node(FUNCTIONBODY, VOID)
                  -- Node(VARDEF, x)
                         -- Node(INT32, 12)
                  -- Node(RETURN, VOID)
                         -- Node(INT32, 12)
    */
    std::string resolveFunctionCall(const NASTNode* node) {


        
    }

}