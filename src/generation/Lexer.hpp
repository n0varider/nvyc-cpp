#pragma once

#include "data/NodeStream.hpp"
#include "data/NodeType.hpp"
#include "data/Value.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

using nvyc::NodeStream;
using nvyc::NodeType;

namespace nvyc {

    class Lexer {
        private:
            Lexer();
            void init();
            NodeType numericNativeType(const std::string& s) const;
            static const std::unordered_set<NodeType> NUMERICS;
        
        public:
            const Value NULL_VALUE = Value(NodeType::VOID);
            std::unordered_map<std::string, NodeType> rep;
            static Lexer& getInstance();
            NodeStream* lex(const std::vector<std::string>& lines);
            NodeStream* convertNumeric(NodeType type, const std::string& value);
            
    }; // Lexer

} // namespace nvyc::generation