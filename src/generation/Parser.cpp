#include "Parser.hpp"
#include "utils/ParserUtils.hpp"
#include "data/Symbols.hpp"
#include <iostream>
#include <memory>

using nvyc::NASTNode;
using nvyc::NodeStream;
using nvyc::NodeType;

int advance = 0;

std::unique_ptr<NASTNode> nvyc::Parser::parse(NodeStream& stream) {

    NodeType type = stream.getType();
    std::unique_ptr<NASTNode> node;

    switch(type) {
        case NodeType::FUNCTION:
            node = parseFunction(stream);
            break;
        default:
            node = nullptr;
            break;
    }
    return node;
}

std::unique_ptr<NASTNode> nvyc::Parser::parseFunction(NodeStream& stream) {
    NodeStream* streamptr = stream.forward(nvyc::ParserUtils::FUNCTION_FORWARD_NAME); // Move to name
    std::string functionName = nvyc::getStringValue(NodeType::STR, streamptr->getData());

    streamptr = streamptr->forward(nvyc::ParserUtils::FUNCTION_FORWARD_FIRSTARG); 

    auto functionNode = nvyc::ParserUtils::createNode(NodeType::FUNCTION, new std::string(functionName));

    std::cout << functionNode->asString() << std::endl;

    auto functionParams = nvyc::ParserUtils::createNode(NodeType::FUNCTIONPARAM, nullptr);
    auto functionReturn = nvyc::ParserUtils::createNode(NodeType::FUNCTIONRETURN, nullptr);
    auto functionBody = nvyc::ParserUtils::createNode(NodeType::FUNCTIONBODY, nullptr);

    while(streamptr->getType() != NodeType::CLOSEPARENS) {
        NodeType varType = streamptr->getType();
        void* varNamePtr = streamptr->getNext()->getData();
        std::string varName = nvyc::getStringValue(NodeType::STR, varNamePtr);
        auto variableNode = nvyc::ParserUtils::createNode(NodeType::VARIABLE, new std::string(varName));

        functionParams->addSubnode(std::move(variableNode));
    
        streamptr = streamptr->forward(nvyc::ParserUtils::FUNCTION_FORWARD_NEXTARG);
        if(streamptr->getType() == NodeType::COMMADELIMIT) streamptr = streamptr->getNext();
    }

    streamptr = streamptr->forward(3); // Point at return type

    auto returnType = nvyc::ParserUtils::createNode(streamptr->getType(), nullptr);
    functionReturn->addSubnode(std::move(returnType));

    functionNode->addSubnode(std::move(functionParams));
    functionNode->addSubnode(std::move(functionReturn));
    functionNode->addSubnode(std::move(functionBody));

    return functionNode;
}