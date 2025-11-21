#pragma once

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

    class ParserUtils {

        private:
            int bodyDepth = 0;
        
        public:
            // Expression constants
            static constexpr bool LOCAL_EXPRESSION = false;
            static constexpr bool ENCLOSED_EXPRESSION = true;

            // NodeStream traversal constants
            static constexpr int FUNCTION_FORWARD_NAME = 1;
            static constexpr int FUNCTION_FORWARD_FIRSTARG = 2;
            static constexpr int FUNCTION_FORWARD_NEXTARG = 2;
            static constexpr int FUNCTION_FORWARD_RETURNTYPE = 3;

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

            // Generic
            inline static NASTNode* createNode(NodeType type, void* value) {
                NASTNode* node = new NASTNode(type, value);
                return node;
            }

            void addBodyNode(NASTNode& node, NASTNode& bodyNode);

            // Functions
            NASTNode* createFunction(const std::string& name);
            void addFunctionBody(NASTNode& function, NASTNode& body);
            void addFunctionArg(NASTNode& function, NASTNode& arg);
            void setFunctionReturnType(NASTNode& function, NodeType type);
            
            NASTNode* createFunctionCall(const std::string& name);
            void addFunctionCallArg(NASTNode& function,  NASTNode& arg);

            // Conditionals
            NASTNode* createConditional();
            void setCondition(NASTNode& conditional, NASTNode& condition);
            void addConditionalIfBody(NASTNode& conditional, NASTNode& ifNode);
            void addConditionalElseBody(NASTNode& conditional,  NASTNode& elseNode);

            // Variables
            NASTNode* defineVariable(const std::string& name);
            NASTNode* createVariable(const std::string& name);
            // NASTNode* assignVariable(const std::string& name, const NASTNode& value);
            void setVariableValue(NASTNode& variable, NASTNode& value);
            void castVariable(NASTNode& variable, NodeType cast);
            void castVariableToStruct(NASTNode& variable,  std::string& structName);

            // Returns
            NASTNode* createReturn(NASTNode& value);
            
            // Structs
            NASTNode* createStruct(const std::string& name);
            void addStructNode(NASTNode& structName,  NASTNode& member);
            NASTNode* accessStructMember(const std::string& variable);

            // Loops
            NASTNode* createForLoop();
            void setLoopDefinition(NASTNode& loop, NASTNode& definition);
            void setLoopCondition(NASTNode& loop, NASTNode& condition);
            void setLoopIteration(NASTNode& loop, NASTNode& iteration);
            void addLoopBody(NASTNode& loop, NASTNode& bodyNode);

            // Arrays
            NASTNode* createArray(NodeType type, int size);
            NASTNode* accessArray(const std::string& name, std::variant<int, std::string> index);

    }; // ParserUtils

} // namespace nvyc::utils