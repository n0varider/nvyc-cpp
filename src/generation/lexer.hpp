#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "data/NodeStream.hpp"
#include "data/NodeType.hpp"

using nvyc::data::NodeStream;
using nvyc::data::NodeType;

namespace nvyc::generation {

    class Lexer {
        private:
            Lexer();
            void init();
            NodeType numericNativeType(const std::string& s) const;
            static const std::unordered_set<nvyc::data::NodeType> NUMERICS;
        
        public:
            std::unordered_map<std::string, NodeType> rep;
            static Lexer& getInstance();
            NodeStream* lex(const std::vector<std::string>& lines);
            NodeStream* convertNumeric(NodeType type, const std::string& value);
            
    }; // Lexer

} // namespace nvyc::generation