#pragma once

#include <vector>
#include <unordered_map>
#include <regex>
#include <format>
#include "data/NodeStream.hpp"
#include "data/NodeType.hpp"
#include "error/Error.hpp"

using nvyc::NodeStream;
using nvyc::NodeType;

namespace nvyc::Processing {

    inline std::unordered_map<std::string, std::string> functionNameMap;

    void resolveFunctionCalls(NodeStream* stream);
    void mangleFunctions(std::string module, std::vector<std::string>* lines);
    void resolveSpecialSymbols(NodeStream* root);
    bool isArrayPattern(NodeStream* stream);
    
    bool startsWith(std::string str, std::string start);
    void removeInlineComments(std::vector<std::string> txt);
    int idxOfComment(std::string s);
}