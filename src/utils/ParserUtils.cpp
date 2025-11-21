#include "ParserUtils.hpp"
#include "data/NodeStream.hpp"
#include "data/NASTNode.hpp"
#include "data/NodeType.hpp"
#include <string>
#include <variant>

using nvyc::data::NASTNode;
using nvyc::data::NodeType;

namespace nvyc::utils {

    /*
    
        It isn't safe to just pass references around since
        the caller and the ASTNode will both own the memory,
        which could result in a double delete.

        Move to std::unique_ptr
    
    */


    NASTNode* ParserUtils::createFunction(const std::string& name) {
        NASTNode* root = createNode(NodeType::FUNCTION, new std::string(name));
        NASTNode* functionArgs = createNode(NodeType::FUNCTIONPARAM, nullptr);
        NASTNode* functionReturn = createNode(NodeType::FUNCTIONRETURN, nullptr);
        NASTNode* functionBody = createNode(NodeType::FUNCTIONBODY, nullptr);
        
        root->addSubnode(functionArgs);
        root->addSubnode(functionReturn);
        root->addSubnode(functionBody);

        return root;
    }

    void ParserUtils::addFunctionBody(NASTNode& function, NASTNode& body) {
        function.getSubnode(ParserUtils::FUNCTION_BODY)->addSubnode(&body);
    }



} // namespace nvyc::utils