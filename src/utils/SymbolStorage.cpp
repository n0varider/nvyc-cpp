#include "SymbolStorage.hpp"
#include "error/Error.hpp"

namespace nvyc {

    llvm::Value* SymbolStorage::getAlloca(const std::string& variable) {
        auto it = variableAlloca.find(variable);
        if(it != variableAlloca.end()) {
            return it->second;
        }
        nvyc::Error::nvyerr_out("Invalid map request for variable " + variable);
        return nullptr;
    }

    void SymbolStorage::storeAlloca(const std::string variable, llvm::Value* value) {
        variableAlloca[variable] = value;
    }

    NodeType SymbolStorage::getVarNvyType(const std::string& variable) {
        auto it = variableTypes.find(variable);
        if(it != variableTypes.end()) {
            return it->second.first;
        }
        nvyc::Error::nvyerr_out("Invalid type request for variable " + variable);
        return NodeType::INVALID;
    }

    llvm::Type* SymbolStorage::getVarNativeType(const std::string& variable) {
        auto it = variableTypes.find(variable);
        if(it != variableTypes.end()) {
            return it->second.second;
        }
        nvyc::Error::nvyerr_out("Invalid type request for variable " + variable);
        return nullptr;
    }

    void SymbolStorage::storeVarType(const std::string variable, NodeType type, llvm::Type* ty) {
        variableTypes[variable] = makePair(type, ty);
    }

    NodeType SymbolStorage::getFunType(const std::string& func) {
        auto it = functionTypes.find(func);
        if(it != functionTypes.end()) {
            return it->second;
        }
        nvyc::Error::nvyerr_out("Invalid map request for variable " + func);
        return NodeType::INVALID;
    }
    void SymbolStorage::storeFunType(const std::string func, NodeType type) {
        functionTypes[func] = type;
    }


    std::pair<NodeType, llvm::Type*> SymbolStorage::makePair(NodeType type, llvm::Type* ty) {
        return std::pair<NodeType, llvm::Type*>(type, ty);
    }
}
