#pragma once

#include "data/NodeType.hpp"
#include "data/Symbols.hpp"
#include "data/Value.hpp"
#include "error/Error.hpp"
#include "data/Symbols.hpp"
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <string>
#include <sstream>
#include <iostream>

namespace nvyc {

    class NodeStream {
        private:
            struct Token {
                Value val;
                NodeType type;
                int line;

                Token(Value v, NodeType ty, int l) : type(ty), val(v), line(l) {}

                Value getValue() {
                    return val;
                }

                NodeType getType() {
                    return type;
                }

                int getLine() {
                    return line;
                }
            };

            std::vector<Token> tokens;
            size_t idx = 0;

            struct StreamCursor {
                const std::vector<Token>& tok_ref;
                size_t idx_it = 0;
                const size_t idx_origin = 0; // Cursor spawned here
                size_t intermediate = 0;

                StreamCursor(const std::vector<Token>& tok, size_t it) 
                : tok_ref(tok), idx_it(it), idx_origin(it) {}

                StreamCursor& next() {
                    idx_it++;
                    return *this;
                }

                size_t distMoved() {
                    return idx_it - idx_origin;
                }

                bool validNext() {
                    return (idx_it + 1) < tok_ref.size(); 
                }

                bool validPrev() {
                    return idx_it != 0;
                }

                StreamCursor& prev() {
                    idx_it--;
                    return *this;
                }

                void forward(int i) {
                    idx_it += i;
                }

                void backward(int i) {
                    idx_it -= i;
                }

                StreamCursor& transient(int i) {
                    intermediate = idx_it + i;
                    return *this;
                }

                Token getTransient() {
                    Token t = tok_ref[intermediate];
                    intermediate = 0;
                    return t;
                }

                Token peek(size_t dist) {
                    if(dist + idx_it > tok_ref.size()) {
                        nvyc::Error::nvyerr_failcompile(1, "Attempted to peek at a token out of bounds");
                    }
                    Token t = tok_ref[dist + idx_it];
                    return t;
                }

                Token behind(size_t dist) {
                    if(dist > idx_it) dist = idx_it;
                    Token t = tok_ref[idx_it - dist];
                    return t;
                }

                Token get() {
                    if(idx_it < 0 || idx_it > tok_ref.size()) {
                        nvyc::Error::nvyerr_failcompile(1, "Attempted to access token out of bounds");
                    }
                    return tok_ref[idx_it];
                }
                
                StreamCursor spawn() {
                    return StreamCursor(tok_ref, idx_it);
                }
            };

        public:
            NodeStream() {}

            void addNode(NodeType type, Value val, int line) {
                tokens.push_back(Token(val, type, line));
            }

            Value getValue(int i = -1) const {
                if(i < 0) i = idx;
                return tokens[i].val;
            }

            NodeType getType(int i = -1) const {
                if(i < 0) i = idx;
                return tokens[i].type;
            }

            Token getToken(int i = -1) const {
                if(i < 0) i = idx;
                return tokens[i];
            }

            Token getNext() {
                return tokens[idx + 1];
            }

            Token getPrev() {
                return tokens[idx - 1];
            } 

            Token getForward(int dist) {
                if((idx + dist) > tokens.size()) nvyc::Error::nvyerr_out("Out of bounds access for NodeStream::getForward");
                return tokens[idx + dist];
            }

            Token getBackward(int dist) {
                int traverse = idx - dist;
                if(traverse < 0 || traverse > tokens.size()) nvyc::Error::nvyerr_out("Out of bounds access for NodeStream::getBackward");
                return tokens[idx - dist];
            }

            Token createToken(NodeType ty, Value v, int line) {
                return Token(v, ty, line);
            }

            int size() const {
                return tokens.size();
            }

            void setToken(Token tok, int idx) {
                tokens[idx] = tok;
            }

            void delTokens(int idx, int idy) {
                int i = idx;
                while(i < idy) {
                    tokens.erase(tokens.begin() + idx);
                    i++;
                }
            }

            void insertToken(Token tok, int idx) {
                tokens.insert(tokens.begin() + idx, tok);
            }

            void backward(int limit = -1) {
                if(limit < 0 || limit > tokens.size()) idx = 0;
                else idx -= limit;
            }

            void forward(int dist) {
                idx += dist;
            }

            void moveTo(int i) {
                idx = i;
            }

            void forwardType(NodeType type) {
                int size = tokens.size();
                while(idx < size && tokens[idx].type != type) {
                    idx++;
                }
            }

            std::string currentNodeAsString() const {
                return "Node(" + symbols::nodeTypeToString(getType()) + " " + getValue().asString() + ")";
            }

            StreamCursor iterator() const {
                StreamCursor it(tokens, idx);
                return it;
            }

    }; // NodeStream

    inline std::ostream& operator<<(std::ostream& os, const NodeStream& stream) {
        for(int i = 0; i < stream.size(); i++) {
            os << "NodeStream(" << symbols::nodeTypeToString(stream.getType(i)) << ", " << stream.getValue(i).asString() << ")\n";
        }
        return os;
    }

} // namespace nvyc