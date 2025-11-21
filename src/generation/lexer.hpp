#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "../data/NodeStream.hpp"
#include "../data/NodeType.hpp"

using nvyc::data::NodeStream;
using nvyc::data::NodeType;

namespace nvyc::generation {

    class Lexer {
        private:
            std::unordered_map<std::string, NodeType> rep;
        
            Lexer();
            void init();
            NodeType numericNativeType(const std::string& s) const;
            static const std::unordered_set<nvyc::data::NodeType> NUMERICS;
        
        public:
            static Lexer& getInstance();
            NodeStream* lex(const std::vector<std::string>& lines);
            
    }; // Lexer

} // namespace nvyc::generation