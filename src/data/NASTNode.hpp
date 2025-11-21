#pragma once

#include "NodeType.hpp"
#include "NodeStream.hpp"
#include "Symbols.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

namespace nvyc::data {

    class NASTNode {
        private:
            std::vector<NASTNode*> subnodes;
            NodeType type;
            void* dptr;
            bool owned;

        public:
            NASTNode(void* p, NodeType t, bool owned) : dptr(p), type(t), owned(owned) {}
            ~NASTNode() {
                for(auto subnode : subnodes) {
                    subnode->free();
                    delete subnode;
                }
                free();
            }

            void free() {
                if(!owned || !dptr) return;
            }

            void* getData() const {
                return dptr;
            }

            NodeType getType() const {
                    return type;
            }

            void setType(NodeType t) {
                    type = t;
            }

            bool isOwned() const {
                    return owned;
            }

            std::vector<NASTNode*> getSubnodes() const {
                return subnodes;
            }   

            void addSubnode(NASTNode* node) {
                    subnodes.push_back(node);
            }

            NASTNode* getSubnode(int node) {
                    return subnodes.at(node); // Automatically throws error if OOB
            }

            std::string asString() {
                    return asStringHelper("", "");
            }

            std::string asStringHelper(std::string prefix, std::string child) {
                    std::ostringstream oss;
                    oss << prefix;
                    oss << "Node(" << nodeTypeToString(type) << ", " << getStringValue(type, dptr) << ")\n";
                    if(!subnodes.empty()) {
                            for(auto subnode: subnodes) {
                                    oss << subnode->asStringHelper(child + "        -- ", child + "         ");
                            }
                    }
                    return oss.str();
                }
        }; // NASTNode
} // namespace nvyc::data