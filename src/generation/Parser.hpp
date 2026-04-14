#pragma once

#include "data/NASTNode.hpp"
#include "data/NodeStream.hpp"
#include "data/NodeType.hpp"
#include <stack>
#include <vector>
#include <memory>

using nvyc::NASTNode;
using nvyc::NodeStream;
using nvyc::NodeType;

namespace nvyc {

    class Parser {

        private:
            // Module
            std::string currentModule;
            bool insideModule = false;
            std::unique_ptr<NASTNode> parseModule(NodeStream& stream);

            // Struct
            std::unique_ptr<NASTNode> parseStruct(NodeStream& stream);

            // Block statements
            std::unique_ptr<NASTNode> parseFunction(NodeStream& stream);
            std::unique_ptr<NASTNode> parseNativeFunction(NodeStream& stream);
            std::unique_ptr<NASTNode> parseFunctionCall(NodeStream& stream);
            std::unique_ptr<NASTNode> parseForLoop(NodeStream& stream);
            std::unique_ptr<NASTNode> parseWhileLoop(NodeStream& stream);
            std::unique_ptr<NASTNode> parseConditional(NodeStream& stream);

            // Expressions
            std::unique_ptr<NASTNode> parseAssign(NodeStream& stream);
            std::unique_ptr<NASTNode> parseVardef(NodeStream& stream);
            std::unique_ptr<NASTNode> parseReturn(NodeStream& stream);
            
            int getExpression(const NodeStream& stream, bool enclosed);
            std::unique_ptr<NASTNode> parseExpression(NodeStream& stream, int end);
            void processOperator(std::stack<NodeType>& operatorStack, std::stack<std::unique_ptr<NASTNode>>& valueStack);

            // Utility
            void resolveDoubleTokens(NodeStream& stream);
            std::vector<NodeStream*> parselist(NodeStream& root);
            std::vector<std::unique_ptr<NASTNode>> parseBodyNodes(NodeStream& stream);
            std::vector<int> getFunctionCallArgs(NodeStream& stream);

        public:
            std::unique_ptr<NASTNode> parse(NodeStream& stream);
            std::vector<std::unique_ptr<NASTNode>> parseStream(NodeStream& stream);

    }; // Parser

} // namespace nvyc::generation