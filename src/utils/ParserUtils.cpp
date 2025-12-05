#include "ParserUtils.hpp"
#include "data/NodeStream.hpp"
#include "data/NASTNode.hpp"
#include "data/NodeType.hpp"
#include "data/Symbols.hpp"
#include <string>
#include <variant>
#include <memory>
#include <stack>

using nvyc::NASTNode;
using nvyc::NodeType;

namespace nvyc::ParserUtils {

    void addBodyNode(NASTNode& node, std::unique_ptr<NASTNode> bodyNode) {
        NodeType type = node.getType();


        if(type == NodeType::VARDEF || type == NodeType::STRUCT) {
            node.addSubnode(std::move(bodyNode));
            return;
        }

        int bodyIndex = 0;

        switch(type) {
            case NodeType::FUNCTION: 
                bodyIndex = FUNCTION_BODY;
                break;
            case NodeType::IF: 
                bodyIndex = CONDITIONAL_BODY;
                break;
            case NodeType::FORLOOP: 
                bodyIndex = FORLOOP_BODY;
                break;
            default: 
                // Runtime errors on windows just show the "Not responding" box so print to console
                std::cout << "Unknown branch: " << nvyc::symbols::nodeTypeToString(type) << std::endl;
                throw std::runtime_error("Unknown branch: " + nvyc::symbols::nodeTypeToString(type));
                break;
        }

        node.getSubnode(bodyIndex)->addSubnode(std::move(bodyNode));
    }


    // ----------------------------------------------
    // -                FUNCTIONS                   -
    // ----------------------------------------------
    std::unique_ptr<NASTNode> createFunction(const std::string& name) {
        auto root = createNode(NodeType::FUNCTION, Value(name));
        auto functionArgs = createNode(NodeType::FUNCTIONPARAM, NULL_VALUE);
        auto functionReturn = createNode(NodeType::FUNCTIONRETURN, NULL_VALUE);
        auto functionBody = createNode(NodeType::FUNCTIONBODY, NULL_VALUE);
        
        root->addSubnode(std::move(functionArgs));
        root->addSubnode(std::move(functionReturn));
        root->addSubnode(std::move(functionBody));

        return root;
    }

    void addFunctionBody(NASTNode& function, std::unique_ptr<NASTNode> body) {
        addBodyNode(function, std::move(body));
    }

    void addFunctionArg(NASTNode& function, std::unique_ptr<NASTNode> arg) {
        function.getSubnode(FUNCTION_ARGS)->addSubnode(std::move(arg));
    }

    void setFunctionReturnType(NASTNode& function, NodeType type) {
        auto returnNode = createNode(type, NULL_VALUE);
        function.getSubnode(FUNCTION_RETURN)->addSubnode(std::move(returnNode));
    }



    // ----------------------------------------------
    // -                CONDITIONALS                -
    // ----------------------------------------------

    std::unique_ptr<NASTNode> createConditional() {
        auto conditionalHead = createNode(NodeType::IF, NULL_VALUE);
        auto conditionalCondition = createNode(NodeType::CONDITION, NULL_VALUE);
        auto conditionalBody = createNode(NodeType::FUNCTIONBODY, NULL_VALUE);
        auto conditionalElse = createNode(NodeType::ELSE, NULL_VALUE);

        conditionalHead->addSubnode(std::move(conditionalCondition));
        conditionalHead->addSubnode(std::move(conditionalBody));
        conditionalHead->addSubnode(std::move(conditionalElse));

        return conditionalHead;
    }

    void setCondition(NASTNode& conditional, std::unique_ptr<NASTNode> condition) {
        conditional.getSubnode(CONDITIONAL_COND)->addSubnode(std::move(condition));
    }

    void addConditionalIfBody(NASTNode& conditional, std::unique_ptr<NASTNode> ifNode) {
        addBodyNode(conditional, std::move(ifNode));
    }

    void addConditionalElseBody(NASTNode& conditional, std::unique_ptr<NASTNode> elseNode) {
        conditional.getSubnode(CONDITIONAL_ELSE)->addSubnode(std::move(elseNode));
    }


    // ----------------------------------------------
    // -                VARIABLES                   -
    // ----------------------------------------------

    std::unique_ptr<NASTNode> defineVariable(const std::string& name) {
        return createNode(NodeType::VARDEF, Value(name));
    }

    std::unique_ptr<NASTNode> createVariable(const std::string& name) {
        return createNode(NodeType::VARIABLE, Value(name));
    }

    void setVariableValue(NASTNode& variable, std::unique_ptr<NASTNode> value) {
        addBodyNode(variable, std::move(value));
    }
    void castVariable(NASTNode& variable, NodeType cast) {
        auto castNode = createNode(NodeType::CAST, Value(cast));
        addBodyNode(variable, std::move(castNode));
    }

    void castVariableToStruct(NASTNode& variable,  std::string& structName) {
        auto castNode = createNode(NodeType::CAST, Value(NodeType::STRUCT));
        auto structType = createNode(NodeType::STRUCT, Value(structName));
        
        castNode->addSubnode(std::move(structType));
        addBodyNode(variable, std::move(castNode));
    }



    // ----------------------------------------------
    // -                RETURNS                     -
    // ----------------------------------------------

    std::unique_ptr<NASTNode> createReturn(std::unique_ptr<NASTNode> value) {
        auto returnNode = createNode(NodeType::RETURN, NULL_VALUE);
        
        returnNode->addSubnode(std::move(value));

        return returnNode;
    }


    // ----------------------------------------------
    // -                STRUCTS                     -
    // ----------------------------------------------

    std::unique_ptr<NASTNode> createStruct(const std::string& name) {
        return createNode(NodeType::STRUCT, Value(name));
    }

    void addStructNode(NASTNode& structNode,  std::unique_ptr<NASTNode> member) {
        addBodyNode(structNode, std::move(member));
    }

    std::unique_ptr<NASTNode> accessStructMember(const std::string& variable) {
        
        // Split variable at '.' such as x.y being var x -> access member y
        std::vector<std::string> elems; 
        std::stringstream ss(variable);
        std::string item;

        while(std::getline(ss, item, '.')) {
            elems.push_back(item);
        }

        if(elems.empty()) return nullptr; // Should ideally error if this ever happens

        // Get full member depth (x.y.z)
        auto root = createNode(NodeType::VARIABLE, Value(elems[0]));
        NASTNode* current = root.get();

        for(int i = 1; i < elems.size(); i++) {
            auto memberNode = createNode(NodeType::MEMBER, Value(elems[i]));
            current->addSubnode(std::move(memberNode));
            current = current->getSubnode(0);
        }

        return root;
    }

    // ----------------------------------------------
    // -                  LOOPS                     -
    // ----------------------------------------------

    std::unique_ptr<NASTNode> createForLoop() {
        auto loopHead = createNode(NodeType::FORLOOP, Value("VOID"));
        auto loopDefinition = createNode(NodeType::LOOPDEF, NULL_VALUE);
        auto loopCondition = createNode(NodeType::LOOPCOND, NULL_VALUE);
        auto loopIteration = createNode(NodeType::LOOPITERATION, NULL_VALUE);
        auto loopBody = createNode(NodeType::FUNCTIONBODY, NULL_VALUE);

        loopHead->addSubnode(std::move(loopDefinition));
        loopHead->addSubnode(std::move(loopCondition));
        loopHead->addSubnode(std::move(loopIteration));
        loopHead->addSubnode(std::move(loopBody));

        return loopHead;
    }

    void setLoopDefinition(NASTNode& loop, std::unique_ptr<NASTNode> definition) {
        loop.getSubnode(FORLOOP_DEFINITION)->addSubnode(std::move(definition));
    }

    void setLoopCondition(NASTNode& loop, std::unique_ptr<NASTNode> condition) {
        loop.getSubnode(FORLOOP_CONDITION)->addSubnode(std::move(condition));
    }

    void setLoopIteration(NASTNode& loop, std::unique_ptr<NASTNode> iteration) {
        loop.getSubnode(FORLOOP_ITERATION)->addSubnode(std::move(iteration));
    }

    void addLoopBody(NASTNode& loop, std::unique_ptr<NASTNode> bodyNode) {
        addBodyNode(loop, std::move(bodyNode));
    }



    // ----------------------------------------------
    // -                  ARRAYS                    -
    // ----------------------------------------------

    std::unique_ptr<NASTNode> createArray(NodeType type, int size) {
        auto arrayNode = createNode(NodeType::ARRAY, Value(type));
        auto arraySize = createNode(NodeType::ARRAY_SIZE, Value(size));

        arrayNode->addSubnode(std::move(arraySize));

        return arrayNode;

    }

    std::unique_ptr<NASTNode> accessArray(const std::string& name, std::variant<int, std::string> index) {
        auto accessNode = createNode(NodeType::ARRAY_ACCESS, NULL_VALUE);
        auto arrayName = createNode(NodeType::ARRAY, Value(name));
        //auto arrayIndex = createNode(NodeType::ARRAY_INDEX, new std::variant<int, std::string>(index));
    
        accessNode->addSubnode(std::move(arrayName));
        //accessNode->addSubnode(std::move(arrayIndex));

        return accessNode;
    }


    int getDepth(NodeStream& stream, NodeType open, NodeType close) {
        std::stack<int> braces;
        NodeType type;
        NodeStream* streamptr = &stream;
        streamptr = streamptr->getNext()->getNext();
        int depth = 2;

        braces.push(1);

        while(!braces.empty()) {
            type = streamptr->getType();
            
            if(type == open) braces.push(1);
            else if(type == close) braces.pop();

            if(streamptr->getNext() && !braces.empty()) {
                streamptr = streamptr->getNext();
                depth++;
            }
        }

        return depth;
    }

} // namespace nvyc