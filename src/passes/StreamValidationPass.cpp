#include "StreamValidationPass.hpp"
#include "data/NodeStream.hpp"
#include "data/Symbols.hpp"
#include "data/NodeType.hpp"
#include <sstream>

using nvyc::NodeType;
using nvyc::NodeStream;

namespace nvyc::Passes {

    // Need a computeDist() to find where to place the ^

    // Syntax check
    bool StreamValidationPass::validTokens(NodeStream& stream) {
        auto it = stream.iterator();
        std::stringstream ss;

        ss << "Invalid syntax on line ";
        std::streampos start = ss.tellp();
        int line = it.get().getLine();
        size_t index = it.get().getValue().asString().length() + 1;
        size_t lookbehind = 0;

        while(it.validNext()) {
            NodeType ty = it.get().getType();
            NodeType nextTy = it.peek(1).getType();

            if(it.get().getLine() > line) {
                lookbehind = index;
                index = it.get().getValue().asString().length() + 1;
                
            }
            
            
            // '->' must always be followed by a type
            if(ty == NodeType::RETTYPE && !nvyc::symbols::TYPE_SYMBOLS.count(nextTy)) {
                std::cout << symbols::nodeTypeToString(ty) << " " << symbols::nodeTypeToString(nextTy) << std::endl;
                ss << it.peek(1).getLine() << ".\n" << "Missing return type after '->'\n";
                ss << rebuilder.getErrorLocation(it.peek(1).getLine()-1, index);
                nvyc::Error::nvyerr_failcompile(1, ss.str());
            }


            // ';' followed by anything that doesn't start a statement
            else if(
                ty == NodeType::ENDOFLINE && 
                (
                    !nvyc::symbols::START_SYMBOLS.count(nextTy) &&
                    nextTy != NodeType::CLOSEBRACE
                )) {
                ss << it.peek(1).getLine() << ".\n" << "Token after ';' is not the start of a statement\n";
                ss << rebuilder.getErrorLocation(it.peek(1).getLine()-1, index);
            }

            // 'let' must be followed by a variable candidate token. No "let 12apple"
            else if(ty == NodeType::VARDEF && nextTy != NodeType::VARIABLE) {
                ss << it.peek(1).getLine() << ".\n" << "Symbol following variable definition is not a variable name\n";
                ss << rebuilder.getErrorLocation(it.peek(1).getLine()-1, index);
            }

            // Anything that requires looking behind
            else if(it.idx_it != 0) {

                // If we have a start symbols (let, return, etc) and the previous symbol was not EOL/blocking
                if(
                    symbols::START_SYMBOLS.count(ty) && 
                    (
                        (
                            it.behind(1).getType() != NodeType::ENDOFLINE &&
                            !symbols::BRACES.count(it.behind(1).getType())
                        ) &&
                        !symbols::START_SYMBOLS.count(it.behind(1).getType())
                    )
                ) {
                    ss << it.behind(1).getLine() << ".\n" << "Missing semicolon\n";
                    ss << rebuilder.getErrorLocation(it.behind(1).getLine()-1, lookbehind);
                }
            }

            // Check if any branch succeeded
            if(ss.tellp() > start) {
                nvyc::Error::nvyerr_failcompile(1, ss.str());
                return false;
            }

            // Advance to next token
            it.next();
            index += it.get().getValue().asString().length();

        }

        return true;

    }

}