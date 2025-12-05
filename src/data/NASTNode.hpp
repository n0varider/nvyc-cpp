#pragma once

#include "NodeType.hpp"
#include "NodeStream.hpp"
#include "Symbols.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

namespace nvyc {

    class NASTNode {
        private:
            std::vector<std::unique_ptr<NASTNode>> subnodes;
            NodeType type;
            //void* dptr; // Move to unique_ptr with custom ReleaseStream deletion function
            Value dptr;
            bool owned;

        public:
            NASTNode(NodeType t, Value p, bool owned = true) : dptr(p), type(t), owned(owned) {}
            ~NASTNode() = default;

            /*void free() {
                if(!owned || !dptr) return;
            }*/

            Value getData() const {
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

            std::vector<std::unique_ptr<NASTNode>>& getSubnodes() {
                return subnodes;
            }   

            void addSubnode(std::unique_ptr<NASTNode> node) {
                    subnodes.push_back(std::move(node));
            }

            // Cannot move ownership, so returning raw ptr is fine
            NASTNode* getSubnode(int node) {
                    return subnodes.at(node).get(); // Automatically throws error if OOB
            }

            std::string asString() {
                    return asStringHelper("", "");
            }

            std::string asStringHelper(std::string prefix, std::string child) {
                    std::ostringstream oss;
                    oss << prefix;
                    //oss << "Node(" << nvyc::symbols::nodeTypeToString(type) << ", " << nvyc::symbols::getStringValue(type, dptr) << ")\n";
                    oss << "Node(" << nvyc::symbols::nodeTypeToString(type) << ", " << dptr.asString() << ")\n";
                    if(!subnodes.empty()) {
                            for(const auto& subnode: subnodes) {
                                if(subnode) 
                                    oss << subnode->asStringHelper(child + "        -- ", child + "         ");
                                else
                                    oss << "Null node\n";
                            }
                    }
                    return oss.str();
                }
        }; // NASTNode
} // namespace nvyc