#pragma once

#include "data/NodeStream.hpp"
#include "data/NASTNode.hpp"
#include "data/NodeType.hpp"
#include "data/Value.hpp"
#include <string>
#include <variant>
#include <memory>

using nvyc::NASTNode;
using nvyc::NodeType;

namespace nvyc::ParserUtils {

    inline const Value NULL_VALUE = Value(NodeType::VOID);

    // Expression constants
    static constexpr bool LOCAL_EXPRESSION = false;
    static constexpr bool ENCLOSED_EXPRESSION = true;

    // NodeStream traversal constants
    static constexpr int FUNCTION_FORWARD_NAME = 1;
    static constexpr int FUNCTION_FORWARD_FIRSTARG = 2;
    static constexpr int FUNCTION_FORWARD_NEXTARG = 2;
    static constexpr int FUNCTION_FORWARD_RETURNTYPE = 3;
    static constexpr int FUNCTION_FORWARD_FIRSTEXPR = 2;
    static constexpr int VARDEF_FORWARD_EXPR = 3;
    static constexpr int STRUCT_FORWARD_NEXTARG = 3;

    // Body node locations
    static constexpr int FUNCTION_ARGS = 0;
    static constexpr int FUNCTION_RETURN = 1;
    static constexpr int FUNCTION_BODY = 2;

    static constexpr int CONDITIONAL_COND = 0;
    static constexpr int CONDITIONAL_BODY = 1;
    static constexpr int CONDITIONAL_ELSE = 2;

    static constexpr int FORLOOP_DEFINITION = 0;
    static constexpr int FORLOOP_CONDITION = 1;
    static constexpr int FORLOOP_ITERATION = 2;
    static constexpr int FORLOOP_BODY = 3;

    // Generic / Utility
    inline static std::unique_ptr<NASTNode> createNode(NodeType type, Value value) {
        return std::make_unique<NASTNode>(type, value);
    }

    void addBodyNode(NASTNode& node, std::unique_ptr<NASTNode> bodyNode);
    int getDepth(NodeStream&, NodeType open, NodeType close);
    NodeStream* moveToMatchingDelimiter(NodeStream& stream, NodeType open, NodeType close);
    std::vector<NodeStream*> getParseList(NodeStream& root);

    // Functions
    std::unique_ptr<NASTNode> createFunction(const std::string& name);
    void addFunctionBody(NASTNode& function, std::unique_ptr<NASTNode> body);
    void addFunctionArg(NASTNode& function, std::unique_ptr<NASTNode> arg);
    void setFunctionReturnType(NASTNode& function, NodeType type);
    
    std::unique_ptr<NASTNode> createFunctionCall(const std::string& name);
    void addFunctionCallArg(NASTNode& function,  std::unique_ptr<NASTNode> arg);

    // Conditionals
    std::unique_ptr<NASTNode> createConditional();
    void setCondition(NASTNode& conditional, std::unique_ptr<NASTNode> condition);
    void addConditionalIfBody(NASTNode& conditional, std::unique_ptr<NASTNode> ifNode);
    void addConditionalElseBody(NASTNode& conditional,  std::unique_ptr<NASTNode> elseNode);

    // Variables
    std::unique_ptr<NASTNode> defineVariable(const std::string& name);
    std::unique_ptr<NASTNode> createVariable(const std::string& name);
    std::unique_ptr<NASTNode> assignVariable(std::unique_ptr<NASTNode> variable, std::unique_ptr<NASTNode> value);
    void setVariableValue(NASTNode& variable, std::unique_ptr<NASTNode> value);
    void castVariable(NASTNode& variable, NodeType cast);
    void castVariableToStruct(NASTNode& variable,  std::string& structName);

    // Returns
    std::unique_ptr<NASTNode> createReturn(std::unique_ptr<NASTNode> value);
    
    // Structs
    std::unique_ptr<NASTNode> createStruct(const std::string& name);
    void addStructNode(NASTNode& structNode,  std::unique_ptr<NASTNode> member);
    std::unique_ptr<NASTNode> accessStructMember(const std::string& variable);

    // Loops
    std::unique_ptr<NASTNode> createForLoop();
    void setLoopDefinition(NASTNode& loop, std::unique_ptr<NASTNode> definition);
    void setLoopCondition(NASTNode& loop, std::unique_ptr<NASTNode> condition);
    void setLoopIteration(NASTNode& loop, std::unique_ptr<NASTNode> iteration);
    void addLoopBody(NASTNode& loop, std::unique_ptr<NASTNode> bodyNode);

    // Arrays
    std::unique_ptr<NASTNode> createArray(NodeType type, int size);
    std::unique_ptr<NASTNode> accessArray(const std::string& name, std::variant<int, std::string> index);


} // namespace nvyc::utils