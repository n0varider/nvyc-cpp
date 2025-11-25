#include "Parser.hpp"
#include "utils/ParserUtils.hpp"
#include "data/Symbols.hpp"
#include <iostream>
#include <memory>

int advance = 0;
using nvyc::utils::ParserUtils;

NASTNode* nvyc::generation::Parser::parse(NodeStream& stream) {

    NodeType type = stream.getType();
    NASTNode* node;

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

NASTNode* nvyc::generation::Parser::parseFunction(NodeStream& stream) {
    NodeStream* streamptr = stream.forward(ParserUtils::FUNCTION_FORWARD_NAME); // Move to name
    std::string functionName = nvyc::data::getStringValue(NodeType::STR, streamptr->getData());

    streamptr = streamptr->forward(ParserUtils::FUNCTION_FORWARD_FIRSTARG); 

    NASTNode* functionNode = new NASTNode(NodeType::FUNCTION, new std::string(functionName));

    std::cout << functionNode->asString() << std::endl;

    auto functionParams = ParserUtils::createNode(NodeType::FUNCTIONPARAM, nullptr);
    auto functionReturn = ParserUtils::createNode(NodeType::FUNCTIONRETURN, nullptr);
    auto functionBody = ParserUtils::createNode(NodeType::FUNCTIONBODY, nullptr);

    while(streamptr->getType() != NodeType::CLOSEPARENS) {
        NodeType varType = streamptr->getType();
        void* varNamePtr = streamptr->getNext()->getData();
        std::string varName = nvyc::data::getStringValue(NodeType::STR, varNamePtr);
        auto variableNode = ParserUtils::createNode(NodeType::VARIABLE, new std::string(varName));

        functionParams->addSubnode(std::move(variableNode));
    
        streamptr = streamptr->forward(ParserUtils::FUNCTION_FORWARD_NEXTARG);
        if(streamptr->getType() == NodeType::COMMADELIMIT) streamptr = streamptr->getNext();
    }

    streamptr = streamptr->forward(3); // Point at return type

    auto returnType = ParserUtils::createNode(streamptr->getType(), nullptr);
    functionReturn->addSubnode(std::move(returnType));

    functionNode->addSubnode(std::move(functionParams));
    functionNode->addSubnode(std::move(functionReturn));
    functionNode->addSubnode(std::move(functionBody));

    return functionNode;
}