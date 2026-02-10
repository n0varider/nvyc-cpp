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

    NodeType SymbolStorage::getVarType(const std::string& variable) {
        auto it = variableTypes.find(variable);
        if(it != variableTypes.end()) {
            return it->second;
        }
        nvyc::Error::nvyerr_out("Invalid type request for variable " + variable);
        return NodeType::INVALID;
    }
    void SymbolStorage::storeVarType(const std::string variable, NodeType type) {
        variableTypes[variable] = type;
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

}
