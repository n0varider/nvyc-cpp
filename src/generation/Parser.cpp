#include "Parser.hpp"
#include "utils/ParserUtils.hpp"
#include "data/Symbols.hpp"
#include "data/Value.hpp"
#include <iostream>
#include <memory>
#include <stack>

using nvyc::NASTNode;
using nvyc::NodeStream;
using nvyc::NodeType;

int forwardDepth = 0;
bool isNativeFunction = false;

std::unique_ptr<NASTNode> nvyc::Parser::parse(NodeStream& stream) {

    NodeType type = stream.getType();
    std::unique_ptr<NASTNode> node;

    switch(type) {
        case NodeType::FUNCTION:
            node = parseFunction(stream);
            break;
        case NodeType::VARDEF:
            node = parseVardef(stream);
            break;
        case NodeType::RETURN:
            node = parseReturn(stream);
            break;
        case NodeType::FORLOOP:
            node = parseForLoop(stream);
            break;
        case NodeType::VARIABLE:
            if(stream.getNext() && stream.getNext()->getType() == NodeType::ASSIGN) {
                node = parseAssign(stream);
            }
            else{ 
                // Work out Value semantics to avoid copying everywhere unless it's more efficient/safer
                // than having a pointer everywhere
                node = nvyc::ParserUtils::createNode(NodeType::VARIABLE, stream.getData());
            }
            break;
        default:
            node = nullptr;
            break;
    }
    return node;
}

// **********************************************
// *             Block Statements               *
// **********************************************

// Parses function declarations
// func add(int32 a, int32 b) -> int32 { ... }
std::unique_ptr<NASTNode> nvyc::Parser::parseFunction(NodeStream& stream) {
    NodeStream* streamptr = stream.forward(nvyc::ParserUtils::FUNCTION_FORWARD_NAME); // Move to name
    std::string functionName = streamptr->getData().asString(); //nvyc::symbols::getStringValue(NodeType::STR, streamptr->getData());

    streamptr = streamptr->forward(nvyc::ParserUtils::FUNCTION_FORWARD_FIRSTARG); 
    auto functionNode = nvyc::ParserUtils::createFunction(functionName);

    // Loop until every function parameter is parsed
    while(streamptr->getType() != NodeType::CLOSEPARENS) {
        NodeType varType = streamptr->getType();
        //void* varNamePtr = streamptr->getNext()->getData();
        //std::string varName = nvyc::symbols::getStringValue(NodeType::STR, varNamePtr);
        std::string varName = streamptr->getNext()->getData().asString();
        auto variableNode = nvyc::ParserUtils::createNode(NodeType::VARIABLE, Value(varName));

        nvyc::ParserUtils::addFunctionArg(*functionNode, std::move(variableNode));
    
        streamptr = streamptr->forward(nvyc::ParserUtils::FUNCTION_FORWARD_NEXTARG);
        if(streamptr->getType() == NodeType::COMMADELIMIT) streamptr = streamptr->getNext();
    }

    streamptr = streamptr->forward(3); // Point at return type

    auto returnType = streamptr->getType();
    nvyc::ParserUtils::setFunctionReturnType(*functionNode, returnType);

    // Walk through body and parse

    if(!isNativeFunction) {
        streamptr = streamptr->forward(nvyc::ParserUtils::FUNCTION_FORWARD_FIRSTEXPR);
        std::vector<std::unique_ptr<NASTNode>> bodyNodes = parseBodyNodes(*streamptr);
        for(auto& bodyNode : bodyNodes) {
            nvyc::ParserUtils::addFunctionBody(*functionNode, std::move(bodyNode));
        }
    }

    return functionNode;
}

std::unique_ptr<NASTNode> nvyc::Parser::parseForLoop(NodeStream& stream) {
    auto loopNode = nvyc::ParserUtils::createForLoop();
    auto copy = stream.forwardCopy();
    NodeStream* body;
    NodeStream* expression;

    // for(let x = 0; x < 10; x + 1) { ... }

    // ---- Definition    |      let x = 0
    // Moves into parens from for(let x = 0; ...)
    copy = copy->forwardType(NodeType::OPENPARENS)->getNext();
    nvyc::ParserUtils::setLoopDefinition(*loopNode, parseVardef(*copy));

    // ---- Condition     |      x < 10
    // Moves from "let x = 0;" to "x < 10"
    copy = copy->getNext()->forwardType(NodeType::ENDOFLINE)->getNext();
    expression = getExpression(*copy, nvyc::ParserUtils::LOCAL_EXPRESSION);
    nvyc::ParserUtils::setLoopCondition(*loopNode, parseExpression(*expression));

    // ---- Iteration     |     x + 1
    // Move from "x < 10;" to " x + 1"
    copy = copy->getNext()->forwardType(NodeType::ENDOFLINE)->getNext();
    copy->cutHead();
    copy = copy->forwardType(NodeType::OPENBRACE)->backward(2);
    body = copy->forwardCopy();
    copy->cutTail();
    expression = getExpression(*copy->backtrack(), nvyc::ParserUtils::ENCLOSED_EXPRESSION);
    body = body->forward(3);

    nvyc::ParserUtils::setLoopIteration(*loopNode, parseExpression(*expression));

    std::vector<std::unique_ptr<NASTNode>> bodyNodes = parseBodyNodes(*body);
    for(auto& subnode : bodyNodes) {
        nvyc::ParserUtils::addLoopBody(*loopNode, std::move(subnode));
    }

    return loopNode;
}


std::vector<std::unique_ptr<NASTNode>> nvyc::Parser::parseBodyNodes(NodeStream& stream) {
    NodeType type;
    std::vector<std::unique_ptr<NASTNode>> bodyNodes;
    std::stack<int> braces;
    braces.push(1);
    auto streamptr = &stream;

    while(!braces.empty()) {
        type = streamptr->getType();

        switch(type) {
            case NodeType::OPENBRACE:
                braces.push(1);
                break;
            case NodeType::CLOSEBRACE:
                braces.pop();
                break;
            case NodeType::ENDOFLINE:
                streamptr = streamptr->getNext();
                break;
            default:
                type = streamptr->getType();
                auto node = parse(*streamptr);
                bodyNodes.push_back(std::move(node));

                if(type == NodeType::FORLOOP) {
                    streamptr = streamptr->forwardType(NodeType::OPENBRACE)->getNext();
                    forwardDepth = nvyc::ParserUtils::getDepth(*streamptr, NodeType::OPENBRACE, NodeType::CLOSEBRACE) + 1;
                    streamptr = streamptr->forward(forwardDepth);
                }
                else if(type != NodeType::IF) streamptr = streamptr->forwardType(NodeType::ENDOFLINE);
                else braces.pop();
                break;
        }
    }

    return bodyNodes;
}




// **********************************************
// *                Expressions                 *
// **********************************************

std::unique_ptr<NASTNode> nvyc::Parser::parseVardef(NodeStream& stream) {
    NodeStream* streamptr = &stream;
    std::string name = streamptr->getNext()->getData().asString(); //nvyc::symbols::getStringValue(NodeType::VARIABLE, streamptr->getNext()->getData());
    auto variableNode = nvyc::ParserUtils::defineVariable(name);

    streamptr = streamptr->forward(nvyc::ParserUtils::VARDEF_FORWARD_EXPR);
    auto expression = parseExpression(*getExpression(*streamptr, nvyc::ParserUtils::LOCAL_EXPRESSION));

    nvyc::ParserUtils::setVariableValue(*variableNode, std::move(expression));

    return variableNode;
}


// Get expression from a line
// let x = 12 + f(); -> "12 + f()"
NodeStream* nvyc::Parser::getExpression(const NodeStream& stream, bool enclosed) {
    NodeStream* copy = stream.forwardCopy();
    NodeType delimiter = enclosed ? NodeType::OPENPARENS : NodeType::ENDOFLINE;

    if(copy->length() == 1) return copy->backtrack();

    if(!enclosed) {

        // While there is a next node and it isn't a start symbol
        while(copy->getNext() && !nvyc::symbols::START_SYMBOLS.count(copy->getType())) {
            copy = copy->getNext();
        }
        copy = copy->getPrev();

        copy->cutTail();
    }

    return copy->backtrack();
}


// Walk through expression and parse each part
std::unique_ptr<NASTNode> nvyc::Parser::parseExpression(NodeStream& stream) {
    std::stack<std::unique_ptr<NASTNode>> valueStack;
    std::stack<NodeType> operatorStack;

    NodeStream* streamptr = &stream;

    bool expectUnary = true;

    while(streamptr) {
        NodeType tokenType = streamptr->getType();

        // Member access, checks if type is variable and if it has a member (x.member)
        if(
            tokenType == NodeType::VARIABLE 
            //&& nvyc::symbols::getStringValue(NodeType::VARIABLE, streamptr->getData()).find('.') != std::string::npos
            && streamptr->getData().asString().find('.') != std::string::npos
        ) {
            valueStack.push(nvyc::ParserUtils::accessStructMember(streamptr->getData().asString()));//nvyc::symbols::getStringValue(NodeType::VARIABLE, streamptr->getData())));
            expectUnary = false;
        }

        else if(tokenType == NodeType::ARRAY_TYPE) {
            // valueStack.push(parseArray(...));
            expectUnary = false;
        }

        else if(tokenType == NodeType::ARRAY_ACCESS) {
            // valueStack.push(parseArrayAccess(...));
            expectUnary = false;
        }

        // Can fold together with literals and variables
        else if(nvyc::symbols::MEMORY_SYMBOLS.count(tokenType)) {
            valueStack.push(nvyc::ParserUtils::createNode(streamptr->getType(), streamptr->getData()));
            expectUnary = false;
        }

        else if(nvyc::symbols::LITERAL_SYMBOLS.count(tokenType) || tokenType == NodeType::VARIABLE) {
            valueStack.push(nvyc::ParserUtils::createNode(streamptr->getType(), streamptr->getData()));
            expectUnary = false;
        }

        else if(tokenType == NodeType::FUNCTIONCALL) {
            int depth; // = nvyc::ParserUtils::getDepth(streamptr, OPENPARENS, CLOSEPARENS)
            // valueStack.push(parseFunctionCall(...));
            // streamptr = streamptr->forward(depth);
            expectUnary = false;
        }

        else if(nvyc::symbols::isOperator(tokenType)) {
            NodeType actualOp = tokenType;

            if(expectUnary && nvyc::symbols::isPrefixOperator(tokenType)) {
                actualOp = nvyc::symbols::mapUnaryOperator(tokenType);
            }

            while(
                !operatorStack.empty()
                && operatorStack.top() != NodeType::OPENPARENS
                && nvyc::symbols::operatorPrecedence(operatorStack.top()) >= nvyc::symbols::operatorPrecedence(actualOp)
            ) {
                processOperator(operatorStack, valueStack);
            }
            

            operatorStack.push(actualOp);
            expectUnary = true;
        }

        else if(tokenType == NodeType::OPENPARENS) {
            operatorStack.push(tokenType);
            expectUnary = true;
        }

        else if(tokenType == NodeType::CLOSEPARENS) {
            while(!operatorStack.empty() && operatorStack.top() != NodeType::OPENPARENS) {
                processOperator(operatorStack, valueStack);
            }
            operatorStack.pop(); // Discard '('
            expectUnary = false;
        }

        streamptr = streamptr->getNext();
    }

    while(!operatorStack.empty()) {
        processOperator(operatorStack, valueStack);
    }

    auto top = std::move(valueStack.top());
    valueStack.pop();
    return std::move(top);
}

/*

    Walk through operator and value stack and combine into a single tree

    [+], [1, 2] folds into 
    NODE(ADD, VOID)
    -- NODE(INT32, 1)
    -- NODE(INT32, 2) 

    which gets pushed onto the value stack as another operand

*/
void nvyc::Parser::processOperator(std::stack<NodeType>& operatorStack, std::stack<std::unique_ptr<NASTNode>>& valueStack) {
    NodeType operation = operatorStack.top();
    operatorStack.pop();

    // If the current operator is unary
    if(nvyc::symbols::UNARY_SYMBOLS.count(operation)) {
        auto rhs = std::move(valueStack.top()); valueStack.pop();
        auto node = nvyc::ParserUtils::createNode(operation, Value("VOID"));
        node->addSubnode(std::move(rhs));
        valueStack.push(std::move(node));
    }

    // Non-unary operator
    else {
        // Temporary, debug/testing
        if(valueStack.size() < 2) {
            int siz = valueStack.size();
            for(int i = 0; i < siz; i++) {
                std::cout << valueStack.top()->asString() << std::endl;
                valueStack.pop();
            }
            std::string op = nvyc::symbols::nodeTypeToString(operation);
            std::cout << "Not enough values in stack for operation " << op << std::endl;
            std::exit(1);
        }

        auto rhs = std::move(valueStack.top()); valueStack.pop();
        auto lhs = std::move(valueStack.top()); valueStack.pop();
        auto node = nvyc::ParserUtils::createNode(operation, Value("VOID"));
        node->addSubnode(std::move(lhs));
        node->addSubnode(std::move(rhs));
        valueStack.push(std::move(node));
    }
}


std::unique_ptr<NASTNode> nvyc::Parser::parseReturn(NodeStream& stream) {
    auto returnValue = parseExpression(*getExpression(*stream.getNext(), nvyc::ParserUtils::LOCAL_EXPRESSION));
    return nvyc::ParserUtils::createReturn(std::move(returnValue));
}

std::unique_ptr<NASTNode> nvyc::Parser::parseAssign(NodeStream& stream) {
    std::string name = stream.getData().asString();
    bool ptrderef = stream.getType() == NodeType::PTRDEREF;
    bool arrayAccess = stream.getType() == NodeType::ARRAY_ACCESS;
    std::unique_ptr<NASTNode> head;

    // Requires tuple, may need to reformat
    /*
    if(arrayAccess) {
        
    }
    */

    if(ptrderef) {
        head = nvyc::ParserUtils::createNode(NodeType::PTRDEREF, Value(name));
    }

    // Split member access
    /*
    else if(name.find('.') != std::string::npos) {

    }
    */

    // Fallback
    else {
        head = nvyc::ParserUtils::createNode(NodeType::VARIABLE, Value(name));
    }

    auto streamptr = &stream;
    streamptr = streamptr->forward(2);
    auto value = parseExpression(*getExpression(*streamptr, nvyc::ParserUtils::LOCAL_EXPRESSION));

    return nvyc::ParserUtils::assignVariable(std::move(head), std::move(value));
}

