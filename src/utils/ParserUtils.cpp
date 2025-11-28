#include "ParserUtils.hpp"
#include "data/NodeStream.hpp"
#include "data/NASTNode.hpp"
#include "data/NodeType.hpp"
#include "data/Symbols.hpp"
#include <string>
#include <variant>
#include <memory>

using nvyc::data::NASTNode;
using nvyc::data::NodeType;

namespace nvyc::utils {

    void ParserUtils::addBodyNode(NASTNode& node, std::unique_ptr<NASTNode> bodyNode) {
        NodeType type = node.getType();


        if(type == NodeType::VARDEF || type == NodeType::STRUCT) {
            node.addSubnode(std::move(bodyNode));
            return;
        }

        int bodyIndex = 0;

        switch(type) {
            case NodeType::FUNCTION: bodyIndex = ParserUtils::FUNCTION_BODY;
            case NodeType::IF: bodyIndex = ParserUtils::CONDITIONAL_BODY;
            case NodeType::FORLOOP: bodyIndex = ParserUtils::FORLOOP_BODY;
            default: 
                // Runtime errors on windows just show the "Not responding" box so print to console
                std::cout << "Unknown branch: " << nodeTypeToString(type) << std::endl;
                throw std::runtime_error("Unknown branch: " + nodeTypeToString(type));
        }

        node.getSubnode(bodyIndex)->addSubnode(std::move(bodyNode));
    }


    // ----------------------------------------------
    // -                FUNCTIONS                   -
    // ----------------------------------------------
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
        addBodyNode(function, std::move(body));
    }

    void ParserUtils::addFunctionArg(NASTNode& function, std::unique_ptr<NASTNode> arg) {
        function.getSubnode(ParserUtils::FUNCTION_ARGS)->addSubnode(std::move(arg));
    }

    void ParserUtils::setFunctionReturnType(NASTNode& function, NodeType type) {
        function.setType(type);
    }



    // ----------------------------------------------
    // -                CONDITIONALS                -
    // ----------------------------------------------

    std::unique_ptr<NASTNode> ParserUtils::createConditional() {
        auto conditionalHead = createNode(NodeType::IF, nullptr);
        auto conditionalCondition = createNode(NodeType::CONDITION, nullptr);
        auto conditionalBody = createNode(NodeType::FUNCTIONBODY, nullptr);
        auto conditionalElse = createNode(NodeType::ELSE, nullptr);

        conditionalHead->addSubnode(std::move(conditionalCondition));
        conditionalHead->addSubnode(std::move(conditionalBody));
        conditionalHead->addSubnode(std::move(conditionalElse));

        return conditionalHead;
    }

    void ParserUtils::setCondition(NASTNode& conditional, std::unique_ptr<NASTNode> condition) {
        conditional.getSubnode(ParserUtils::CONDITIONAL_COND)->addSubnode(std::move(condition));
    }

    void ParserUtils::addConditionalIfBody(NASTNode& conditional, std::unique_ptr<NASTNode> ifNode) {
        addBodyNode(conditional, std::move(ifNode));
    }

    void ParserUtils::addConditionalElseBody(NASTNode& conditional, std::unique_ptr<NASTNode> elseNode) {
        conditional.getSubnode(ParserUtils::CONDITIONAL_ELSE)->addSubnode(std::move(elseNode));
    }


    // ----------------------------------------------
    // -                VARIABLES                   -
    // ----------------------------------------------

    std::unique_ptr<NASTNode> ParserUtils::defineVariable(const std::string& name) {
        return createNode(NodeType::VARDEF, new std::string(name));
    }

    std::unique_ptr<NASTNode> ParserUtils::createVariable(const std::string& name) {
        return createNode(NodeType::VARIABLE, new std::string(name));
    }

    void ParserUtils::setVariableValue(NASTNode& variable, std::unique_ptr<NASTNode> value) {
        addBodyNode(variable, std::move(value));
    }
    void ParserUtils::castVariable(NASTNode& variable, NodeType cast) {
        auto castNode = createNode(NodeType::CAST, new NodeType(cast));
        addBodyNode(variable, std::move(castNode));
    }

    void ParserUtils::castVariableToStruct(NASTNode& variable,  std::string& structName) {
        auto castNode = createNode(NodeType::CAST, new NodeType(NodeType::STRUCT));
        auto structType = createNode(NodeType::STRUCT, new std::string(structName));
        
        castNode->addSubnode(std::move(structType));
        addBodyNode(variable, std::move(castNode));
    }


    // ----------------------------------------------
    // -                STRUCTS                     -
    // ----------------------------------------------

    std::unique_ptr<NASTNode> ParserUtils::createStruct(const std::string& name) {
        return createNode(NodeType::STRUCT, new std::string(name));
    }

    void ParserUtils::addStructNode(NASTNode& structNode,  std::unique_ptr<NASTNode> member) {
        addBodyNode(structNode, std::move(member));
    }

    std::unique_ptr<NASTNode> ParserUtils::accessStructMember(const std::string& variable) {
        
        // Split variable at '.' such as x.y being var x -> access member y
        std::vector<std::string> elems; 
        std::stringstream ss(variable);
        std::string item;

        while(std::getline(ss, item, '.')) {
            elems.push_back(item);
        }

        if(elems.empty()) return nullptr; // Should ideally error if this ever happens

        // Get full member depth (x.y.z)
        auto root = createNode(NodeType::VARIABLE, new std::string(elems[0]));
        NASTNode* current = root.get();

        for(int i = 1; i < elems.size(); i++) {
            auto memberNode = createNode(NodeType::MEMBER, new std::string(elems[i]));
            current->addSubnode(std::move(memberNode));
            current = current->getSubnode(0);
        }

        return root;
    }

    // ----------------------------------------------
    // -                  LOOPS                     -
    // ----------------------------------------------

    std::unique_ptr<NASTNode> ParserUtils::createForLoop() {
        auto loopHead = createNode(NodeType::FORLOOP, nullptr);
        auto loopDefinition = createNode(NodeType::LOOPDEF, nullptr);
        auto loopCondition = createNode(NodeType::LOOPCOND, nullptr);
        auto loopIteration = createNode(NodeType::LOOPITERATION, nullptr);
        auto loopBody = createNode(NodeType::FUNCTIONBODY, nullptr);

        loopHead->addSubnode(std::move(loopDefinition));
        loopHead->addSubnode(std::move(loopCondition));
        loopHead->addSubnode(std::move(loopIteration));
        loopHead->addSubnode(std::move(loopBody));
    }

    void ParserUtils::setLoopDefinition(NASTNode& loop, std::unique_ptr<NASTNode> definition) {
        loop.getSubnode(ParserUtils::FORLOOP_DEFINITION)->addSubnode(std::move(definition));
    }

    void ParserUtils::setLoopCondition(NASTNode& loop, std::unique_ptr<NASTNode> condition) {
        loop.getSubnode(ParserUtils::FORLOOP_CONDITION)->addSubnode(std::move(condition));
    }

    void ParserUtils::setLoopIteration(NASTNode& loop, std::unique_ptr<NASTNode> iteration) {
        loop.getSubnode(ParserUtils::FORLOOP_ITERATION)->addSubnode(std::move(iteration));
    }

    void ParserUtils::addLoopBody(NASTNode& loop, std::unique_ptr<NASTNode> bodyNode) {
        addBodyNode(loop, std::move(bodyNode));
    }



    // ----------------------------------------------
    // -                  ARRAYS                    -
    // ----------------------------------------------

    std::unique_ptr<NASTNode> ParserUtils::createArray(NodeType type, int size) {
        auto arrayNode = createNode(NodeType::ARRAY, new NodeType(type));
        auto arraySize = createNode(NodeType::ARRAY_SIZE, new int(size));

        arrayNode->addSubnode(std::move(arraySize));

        return arrayNode;

    }

    std::unique_ptr<NASTNode> ParserUtils::accessArray(const std::string& name, std::variant<int, std::string> index) {
        auto accessNode = createNode(NodeType::ARRAY_ACCESS, nullptr);
        auto arrayName = createNode(NodeType::ARRAY, new std::string(name));
        auto arrayIndex = createNode(NodeType::ARRAY_INDEX, new std::variant<int, std::string>(index));
    
        accessNode->addSubnode(std::move(arrayName));
        accessNode->addSubnode(std::move(arrayIndex));

        return accessNode;
    }



} // namespace nvyc::utils