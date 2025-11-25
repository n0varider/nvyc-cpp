#pragma once

#include "data/NASTNode.hpp"
#include "data/NodeStream.hpp"
#include "data/NodeType.hpp"
#include <stack>
#include <vector>

using nvyc::data::NASTNode;
using nvyc::data::NodeStream;
using nvyc::data::NodeType;

namespace nvyc::generation {

    class Parser {

        private:
            // Struct
            NASTNode* parseStruct(NodeStream& stream);

            // Block statements
            NASTNode* parseFunction(NodeStream& stream);
            NASTNode* parseForLoop(NodeStream& stream);
            NASTNode* parseWhileLoop(NodeStream& stream);
            NASTNode* parseConditional(NodeStream& stream);

            // Expressions
            NASTNode* parseAssign(NodeStream& stream);
            NASTNode* parseVardef(NodeStream& stream);
            NASTNode* parseReturn(NodeStream& stream);
            NodeStream* getExpression(const NodeStream& stream);
            NASTNode* parseExpression(NodeStream& stream);
            void processOperator(std::stack<NodeType>& operatorStack, std::stack<NASTNode*>& valueStack);

            // Utility
            void resolveDoubleTokens(NodeStream& stream);
            std::vector<NodeStream*> parselist(NodeStream& root);

        public:
            NASTNode* parse(NodeStream& stream);

    }; // Parser

} // namespace nvyc::generation