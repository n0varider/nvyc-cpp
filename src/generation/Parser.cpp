#include "Parser.hpp"
#include "utils/ParserUtils.hpp"
#include "data/Symbols.hpp"
#include "data/Value.hpp"
#include "error/Debug.hpp"
#include <iostream>
#include <memory>
#include <stack>

#define SWITCHNODE(fun, nodes) node = fun(nodes); break;

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
            SWITCHNODE(parseFunction, stream);
        case NodeType::VARDEF:
            SWITCHNODE(parseVardef, stream);
        case NodeType::RETURN:
            SWITCHNODE(parseReturn, stream);
        case NodeType::FORLOOP:
            SWITCHNODE(parseForLoop, stream);
        case NodeType::IF:
            SWITCHNODE(parseConditional, stream);
        case NodeType::FUNCTIONCALL:
            SWITCHNODE(parseFunctionCall, stream);
        case NodeType::VARIABLE:
            if(stream.getNext().type == NodeType::ASSIGN) {
                node = parseAssign(stream);
            }
            else{ 
                // Work out Value semantics to avoid copying everywhere unless it's more efficient/safer
                // than having a pointer everywhere
                node = nvyc::ParserUtils::createNode(NodeType::VARIABLE, stream.getValue());
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
/*std::unique_ptr<NASTNode> nvyc::Parser::parseFunction(NodeStream& stream) {
    NodeStream* streamptr = stream.forward(nvyc::ParserUtils::FUNCTION_FORWARD_NAME); // Move to name
    std::string functionName = streamptr->getData().asString(); //nvyc::symbols::getStringValue(NodeType::STR, streamptr->getData());

    streamptr = streamptr->forward(nvyc::ParserUtils::FUNCTION_FORWARD_FIRSTARG); 
    auto functionNode = nvyc::ParserUtils::createFunction(functionName);

    std::cout << "Fname " << functionName << std::endl;

    // Loop until every function parameter is parsed
    while(streamptr->getType() != NodeType::CLOSEPARENS) {
        NodeType varType = streamptr->getType();
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
}*/

std::unique_ptr<NASTNode> nvyc::Parser::parseFunction(NodeStream& stream) {
    stream.forward(nvyc::ParserUtils::FUNCTION_FORWARD_NAME);
    std::string functionName = stream.getValue().asString();

    stream.forward(nvyc::ParserUtils::FUNCTION_FORWARD_FIRSTARG); 
    auto functionNode = nvyc::ParserUtils::createFunction(functionName);

    while(stream.getType() != NodeType::CLOSEPARENS) {
        NodeType varType = stream.getType();
        std::string varName = stream.getNext().val.asString();
        auto variableNode = nvyc::ParserUtils::createNode(NodeType::VARIABLE, Value(varName));

        nvyc::ParserUtils::addFunctionArg(*functionNode, std::move(variableNode));
    
        stream.forward(nvyc::ParserUtils::FUNCTION_FORWARD_NEXTARG);
        if(stream.getType() == NodeType::COMMADELIMIT) stream.forward(1);
    }

    stream.forward(2); // Point at return type

    auto returnType = stream.getType();
    nvyc::ParserUtils::setFunctionReturnType(*functionNode, returnType);

    // Walk through body and parse

    if(!isNativeFunction) {
        stream.forward(nvyc::ParserUtils::FUNCTION_FORWARD_FIRSTEXPR);
        std::vector<std::unique_ptr<NASTNode>> bodyNodes = parseBodyNodes(stream);
        for(auto& bodyNode : bodyNodes) {
            nvyc::ParserUtils::addFunctionBody(*functionNode, std::move(bodyNode));
        }
    }

    return functionNode;

}

std::unique_ptr<NASTNode> nvyc::Parser::parseConditional(NodeStream& stream) {
    auto conditionalNode = nvyc::ParserUtils::createConditional();
    
    // From if ( cond ) { ... }, move to "cond" starting at "if"
    stream.forward(2);


    // Move to "{", then move back to "cond" to isolate the inner expression
    /*auto copy = streamptr->forwardCopy();
    copy = copy->forwardType(NodeType::OPENBRACE)->getPrev()->getPrev();
    copy->cutTail();
    auto expression = parseExpression(*getExpression(*copy->backtrack(), nvyc::ParserUtils::ENCLOSED_EXPRESSION));
    nvyc::ParserUtils::setCondition(*conditionalNode, std::move(expression));

    // Move body into "if" part of condition
    streamptr = streamptr->forwardType(NodeType::OPENBRACE)->getNext();
    auto bodyNodes = parseBodyNodes(*streamptr);

    for(auto& bodyNode : bodyNodes) {
        nvyc::ParserUtils::addConditionalIfBody(*conditionalNode, std::move(bodyNode));
    }

    // TODO for now, everything else is assumed to be an "else" block by default.
    streamptr = nvyc::ParserUtils::moveToMatchingDelimiter(*streamptr, NodeType::OPENBRACE, NodeType::CLOSEBRACE);
    bodyNodes = parseBodyNodes(*streamptr);
    for(auto& bodyNode : bodyNodes) {
        nvyc::ParserUtils::addConditionalElseBody(*conditionalNode, std::move(bodyNode));
    }*/

    // Move to "{", then move back to "cond" to isolate the inner expression
    stream.forwardType(NodeType::OPENBRACE);
    stream.backward(2);
    auto expression = parseExpression(stream, getExpression(stream, nvyc::ParserUtils::ENCLOSED_EXPRESSION));
    nvyc::ParserUtils::setCondition(*conditionalNode, std::move(expression));

    // Move body into "if" part of condition
    stream.forwardType(NodeType::OPENBRACE);
    auto bodyNodes = parseBodyNodes(stream);
    
    for(auto& bodyNode : bodyNodes) {
        nvyc::ParserUtils::addConditionalIfBody(*conditionalNode, std::move(bodyNode));
    }

    // TODO for now, everything else is assumed to be an "else" block by default.
    stream.forward(nvyc::ParserUtils::moveToMatchingDelimiter(stream, NodeType::OPENBRACE, NodeType::CLOSEBRACE));
    bodyNodes = parseBodyNodes(stream);
        for(auto& bodyNode : bodyNodes) {
        nvyc::ParserUtils::addConditionalElseBody(*conditionalNode, std::move(bodyNode));
    }


    return conditionalNode;
}   

std::unique_ptr<NASTNode> nvyc::Parser::parseForLoop(NodeStream& stream) {
    auto loopNode = nvyc::ParserUtils::createForLoop();
    NodeStream* body;
    int len;

    // for(let x = 0; x < 10; x + 1) { ... }

    // ---- Definition    |      let x = 0
    // Moves into parens from for(let x = 0; ...)
    stream.forwardType(NodeType::OPENPARENS);
    stream.forward(1);
    nvyc::ParserUtils::setLoopDefinition(*loopNode, parseVardef(stream));

    // ---- Condition     |      x < 10
    // Moves from "let x = 0;" to "x < 10"
    stream.forwardType(NodeType::ENDOFLINE);
    stream.forward(1);
    len = getExpression(stream, nvyc::ParserUtils::LOCAL_EXPRESSION);
    nvyc::ParserUtils::setLoopCondition(*loopNode, parseExpression(stream, len));

    // ---- Iteration     |     x + 1
    // Move from "x < 10;" to " x + 1"
    stream.forwardType(NodeType::ENDOFLINE);
    stream.forward(1);
    stream.forwardType(NodeType::OPENBRACE);
    stream.backward(2);
    len = getExpression(stream, nvyc::ParserUtils::ENCLOSED_EXPRESSION);

    nvyc::ParserUtils::setLoopIteration(*loopNode, parseExpression(stream, len));

    stream.forward(3);
    std::vector<std::unique_ptr<NASTNode>> bodyNodes = parseBodyNodes(stream);
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

    while(!braces.empty()) {
        type = stream.getType();

        switch(type) {
            case NodeType::ASSIGN:
                braces.pop();
                break;
            case NodeType::OPENBRACE:
                braces.push(1);
                break;
            case NodeType::CLOSEBRACE:
                braces.pop();
                break;
            case NodeType::ENDOFLINE:
                stream.forward(1);
                break;
            default:
                type = stream.getType();
                auto node = parse(stream);
                bodyNodes.push_back(std::move(node));

                /*if(type == NodeType::FORLOOP) {
                    stream.forwardType(NodeType::OPENBRACE);
                    forwardDepth = nvyc::ParserUtils::getDepth(stream, NodeType::OPENBRACE, NodeType::CLOSEBRACE) + 1;
                    stream.forward(forwardDepth);
                }
                else if(type != NodeType::IF) stream.forwardType(NodeType::ENDOFLINE);
                else braces.pop();*/
                break;
        }
    }

    return bodyNodes;
}

/*std::vector<std::unique_ptr<NASTNode>> nvyc::Parser::parseBodyNodes(NodeStream& stream) {
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
}*/




// **********************************************
// *                Expressions                 *
// **********************************************

std::unique_ptr<NASTNode> nvyc::Parser::parseVardef(NodeStream& stream) {

    std::string name = stream.getNext().getValue().asString(); //nvyc::symbols::getStringValue(NodeType::VARIABLE, streamptr->getNext()->getData());
    auto variableNode = nvyc::ParserUtils::defineVariable(name);

    stream.forward(nvyc::ParserUtils::VARDEF_FORWARD_EXPR);

    int len = getExpression(stream, nvyc::ParserUtils::LOCAL_EXPRESSION);
    auto expression = parseExpression(stream, len);

    nvyc::ParserUtils::setVariableValue(*variableNode, std::move(expression));

    return variableNode;
}

std::unique_ptr<NASTNode> nvyc::Parser::parseFunctionCall(NodeStream& stream) {
    std::string funName = stream.getValue().asString();
    auto callNode = nvyc::ParserUtils::createFunctionCall(funName);

    auto args = getFunctionCallArgs(stream);
    for(int arg : args) {
        if(stream.getToken(arg).getType() == NodeType::COMMADELIMIT) arg++;
        stream.moveTo(arg);

        auto expression = parseExpression(stream, getExpression(stream, nvyc::ParserUtils::ENCLOSED_EXPRESSION));
        nvyc::ParserUtils::addFunctionCallArg(*callNode, std::move(expression));
    }
    return callNode;

}

std::vector<int> nvyc::Parser::getFunctionCallArgs(NodeStream& stream) {
    NodeType type;
    std::vector<int> nodes;

    auto it = stream.iterator();

    // Empty call case, func ( )  
    it.forward(2);

    if(it.get().getType() == NodeType::CLOSEPARENS) {
        return nodes;
    }
    
    std::stack<int> args;
    args.push(1);

    while(!args.empty()) {
        type = it.get().getType();

        if(type == NodeType::OPENPARENS) {
            args.push(1);
        }

        else if(type == NodeType::CLOSEPARENS) {
            args.pop();
            if(args.empty()) nodes.push_back(it.idx_it);
        }

        else if(args.size() == 1 && type == NodeType::COMMADELIMIT) {
            nodes.push_back(it.idx_it);
        }

        if(!args.empty() && it.validNext()) {
            it.next();
        }
    }

    return nodes;
}


// Get expression from a line
// let x = 12 + f(); -> "12 + f()"
int nvyc::Parser::getExpression(const NodeStream& stream, bool enclosed) {
    /*NodeStream* copy = stream.forwardCopy();
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

    return copy->backtrack();*/
    auto it = stream.iterator();
    size_t len = 0;


    // If the stream is at the end, return immediately
    if(!it.validNext()) return 1;

    if(!enclosed) {
        

        while(it.validNext() && !nvyc::symbols::START_SYMBOLS.count(it.get().getType())) {
            len++;
            it.next();
        }
    }

    return len - 1;


}


// Walk through expression and parse each part
std::unique_ptr<NASTNode> nvyc::Parser::parseExpression(NodeStream& stream, int end) {
    std::stack<std::unique_ptr<NASTNode>> valueStack;
    std::stack<NodeType> operatorStack;
    bool expectUnary = true;
    auto it = stream.iterator();
    NodeType tokenType = stream.getType();
    int dist = 0;

    while(it.validNext() && (it.next().get().getType() != NodeType::ENDOFLINE || it.next().get().getType() != NodeType::CLOSEPARENS)) {
        tokenType = stream.getType();
        // Member access, checks if type is variable and if it has a member (x.member)
        if(
            tokenType == NodeType::VARIABLE 
            //&& nvyc::symbols::getStringValue(NodeType::VARIABLE, streamptr->getData()).find('.') != std::string::npos
            && stream.getValue().asString().find('.') != std::string::npos
        ) {
            valueStack.push(nvyc::ParserUtils::accessStructMember(stream.getValue().asString()));//nvyc::symbols::getStringValue(NodeType::VARIABLE, streamptr->getData())));
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
            valueStack.push(nvyc::ParserUtils::createNode(stream.getType(), stream.getValue()));
            expectUnary = false;
        }

        else if(nvyc::symbols::LITERAL_SYMBOLS.count(tokenType) || tokenType == NodeType::VARIABLE) {
            valueStack.push(nvyc::ParserUtils::createNode(stream.getType(), stream.getValue()));
            expectUnary = false;
        }

        else if(tokenType == NodeType::FUNCTIONCALL) {
            int depth = nvyc::ParserUtils::getDepth(stream, NodeType::OPENPARENS, NodeType::CLOSEPARENS);
            valueStack.push(parseFunctionCall(stream));
            stream.forward(depth);
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

        stream.forward(1);
        it.next();
        dist++;
        if(dist > end) break;
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
    stream.forward(1);
    auto returnValue = parseExpression(stream, getExpression(stream, nvyc::ParserUtils::LOCAL_EXPRESSION));
    return nvyc::ParserUtils::createReturn(std::move(returnValue));
}

std::unique_ptr<NASTNode> nvyc::Parser::parseAssign(NodeStream& stream) {
    std::string name = stream.getValue().asString();
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

    //auto streamptr = &stream;
    //streamptr = streamptr->forward(2);
    auto value = parseExpression(stream, getExpression(stream, nvyc::ParserUtils::LOCAL_EXPRESSION));

    return nvyc::ParserUtils::assignVariable(std::move(head), std::move(value));
}

