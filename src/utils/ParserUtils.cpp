#include "ParserUtils.hpp"
#include "data/NodeStream.hpp"
#include "data/NASTNode.hpp"
#include "data/NodeType.hpp"
#include <string>
#include <variant>
#include <memory>

using nvyc::data::NASTNode;
using nvyc::data::NodeType;

namespace nvyc::utils {

    /*
    
        It isn't safe to just pass references around since
        the caller and the ASTNode will both own the memory,
        which could result in a double delete.

        Move to std::unique_ptr
    
    */


    std::unique_ptr<NASTNode> ParserUtils::createFunction(const std::string& name) {
        auto root = createNode(NodeType::FUNCTION, new std::string(name));
        auto functionArgs = createNode(NodeType::FUNCTIONPARAM, nullptr);
        auto functionReturn = createNode(NodeType::FUNCTIONRETURN, nullptr);
        auto functionBody = createNode(NodeType::FUNCTIONBODY, nullptr);
        
        root->addSubnode(std::move(functionArgs));
        root->addSubnode(std::move(functionReturn));
        root->addSubnode(std::move(functionBody));

        return root;
    }

    void ParserUtils::addFunctionBody(NASTNode& function, std::unique_ptr<NASTNode> body) {
        function.getSubnode(ParserUtils::FUNCTION_BODY)->addSubnode(std::move(body));
    }

    void ParserUtils::addFunctionArg(NASTNode& function, std::unique_ptr<NASTNode> body) {

    }



} // namespace nvyc::utils