#include "LexerCleaner.hpp"
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

    void resolveFunctionCalls(NodeStream* stream) {
        while(stream->getType() != NodeType::ENDOFLINE) {

            /*
            
            fun()
            By default this becomes VARIABLE OPENPARENS ... CLOSEPARENS
            It needs to transform into FUNCTIONCALL ...

            
            */
            if(
                stream->getType() == NodeType::VARIABLE &&
                stream->getNext() &&
                stream->getNext()->getType() == NodeType::OPENPARENS
            ) {
                NodeStream* functionCall = new NodeStream(NodeType::FUNCTIONCALL, Value(stream->getData().asString()));
                functionCall->setPrev(stream->getPrev());
                functionCall->setNext(stream->getNext());
                stream->remove();
                stream = functionCall;
                stream->getPrev()->setNext(functionCall);
                stream->getNext()->setPrev(functionCall);
            
            } else {
                stream = stream->getNext();
            }

        }
    }


    // Turn BITAND BITAND into LOGICAND, Arrays, etc.
    void resolveSpecialSymbols(NodeStream* root) {
        while(root && root->getType() != NodeType::ENDOFSTREAM) {

            if(root->getNext()) {
                NodeType currentType = root->getType();
                NodeType nextType = root->getNext()->getType();

                // Check for pointer types
                if(nvyc::symbols::TYPE_SYMBOLS.count(currentType) && nextType == NodeType::MUL) {
                    std::string pointerType = nvyc::symbols::nodeTypeToString(currentType);
                    NodeStream* foot = root->getNext();

                    while(foot->getType() == NodeType::MUL) {
                        pointerType += "*";
                        foot = foot->getNext();
                    }

                    foot->setPrev(root);
                    root->setType(NodeType::STAR);
                    root->setData(Value(pointerType));

                    // This leaks memory by skipping all nodes between root and foot. 
                    // It won't be an issue once Boxes are implemented
                    root->setNext(foot);
                }

                // Arrays
                else if(
                    nvyc::symbols::TYPE_SYMBOLS.contains(currentType) &&
                    root->getNext()->getType() == NodeType::OPENBRKT &&
                    root->forward(2)->getType() == NodeType::CLOSEBRKT
                ) {
                    NodeStream* array = new NodeStream(NodeType::ARRAY_TYPE, Value(currentType));
                    
                    /*
                        Normally the stream looks like this
                        ... TYPE OPENBRKT CLOSEBRKT ...
                        P   Root N        N         N

                        We move to the 3rd N to get the next node
                    */
                    array->setPrev(root->getPrev());
                    array->setNext(root->forward(3));
                    root = array->getNext();
                }

                else if(isArrayPattern(root)) {
                    // Root initially at VARIABLE
                    // Input:   ... VARIABLE OPENBRKT [INT/VARIABLE] CLOSEBRKT ...
                    // Output:  ... ARRAY_ACCESS VARIABLE [INT/VARIABLE] ...
                    NodeStream* size = root->forward(2);
                    Value index = size->getData();
                    NodeStream* accessNode;
                    NodeStream* countNode;

                    // If the "variable" is a type, it's array creation
                    std::cout << nvyc::symbols::nodeTypeToString(root->getType());
                    if(nvyc::symbols::TYPE_SYMBOLS.count(root->getType())) {
                        NodeType type = root->getType();
                        accessNode = new NodeStream(NodeType::ARRAY, nvyc::NULL_VALUE);
                    }

                    // Otherwise, it is an array access
                    else {
                        std::string arrayVariable = root->getData().asString();
                        accessNode = new NodeStream(NodeType::ARRAY_ACCESS, nvyc::NULL_VALUE);
                    }

                    countNode = new NodeStream(size->getType(), Value(index));
                    
                    // This leaks memory
                    countNode->setNext(root->forward(4));
                    root->forward(4)->setPrev(countNode);

                    countNode->setPrev(root);
                    root->setNext(countNode);
                    
                    accessNode->setPrev(root->getPrev());
                    root->setPrev(accessNode);
                    accessNode->setNext(root);
                    root = accessNode->forward(2);
                }

                // builtin types
                // ...

                // 
            }
            root = root->getNext();

        }
    }


    // Utility

    bool startsWith(std::string str, std::string start) {
        return str.compare(0, start.length(), start) == 0;
    }

    bool isArrayPattern(NodeStream* stream) {
        // Needs to match [VARIABLE/TYPE] OPENBRKT [INT/VARIABLE] CLOSEBRKT
        return
            (
                stream->getType() == NodeType::VARIABLE ||
                nvyc::symbols::TYPE_SYMBOLS.count(stream->getType())
            ) &&
            stream->getNext()->getType() == NodeType::OPENBRKT &&
            (
                stream->forward(2)->getType() == NodeType::INT32 ||
                stream->forward(2)->getType() == NodeType::VARIABLE
            ) &&
            stream->forward(3)->getType() == NodeType::CLOSEBRKT;
    }


    // FUNCTIONS

    // Ideally, this should be done at the AST level since operating on the whole line is expensive
    void mangleFunctions(std::string module, std::vector<std::string>* lines) {
        for(int i = 0; i < lines->size(); i++) {
            std::string line = lines->at(i);
            std::string fname;
            std::string replacement;

            if(startsWith(line, "func")) {
                fname = line.substr(5, line.find("(") - 5);
                
                // Check for main, which shouldn't be mangled
                if(fname != "main") {
            
                    // _nvylang_module_(fname length)fname_(module name length)
                    // _nvylang_nvio_6printf_4
                    std::string moduleId = std::regex_replace(module, std::regex("[^a-zA-Z0-9]"), "");
                    std::string name = std::format("_nvylang_{}_{}{}_{}", moduleId, fname.length(), fname, module.length());
                    lines->at(i) = line.replace(5, fname.length(), name);

                    // Check for name collisions
                    if(functionNameMap.contains(fname)) {
                        std::string secondModule = functionNameMap.at(fname);
                        secondModule = secondModule.substr(9);
                        secondModule = secondModule.substr(0, secondModule.find("_"));
                        
                        // TODO temp
                        nvyc::Error::nvyerr_out("Name collision found");
                    }

                    // Map old name to new name
                    functionNameMap[fname] = name;
                    functionNameMap[module + "_" + fname] = name;
                }
            }
        }
    }



    // COMMENTS

    void removeInlineComments(std::vector<std::string> txt) {
        for(int i = 0; i < txt.size(); i++) {
            std::string line = txt[i];
            int idx = idxOfComment(line);
            if(idx != -1) txt[i] = line.substr(0, idx);
        }
    }

    int idxOfComment(std::string s) {
        bool inString = false;
        char stringChar = '\0'; // Null byte initializer, tracks single vs double quotes

        for(int i = 0; i < s.length(); i++) {
            char c = s[i];

            if(inString) {
                // If c is a quote but the previous character wasn't an escape, it isn't a quoted string
                if(c == stringChar && s[i-1] != '\\') {
                    inString = false;
                }
            } else {
                if(c == '"' || c == '\'') {
                    inString = true;
                    stringChar = c;
                } else if (c == '/' && s[i+1] == '/') {
                    return i;
                }
            }
        }
        return -1; // No comment found
    }
}