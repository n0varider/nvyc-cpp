#pragma once

#include <string>
#include <cstdint>
#include "NodeType.hpp"

namespace nvyc::data {

// Not sure this is optimal
inline std::string nodeTypeToString(NodeType t) {
    switch (t) {
        case NodeType::VARDEF: return "VARDEF";
        case NodeType::VARIABLE: return "VARIABLE";
        case NodeType::REDEF: return "REDEF";
        case NodeType::ASSIGN: return "ASSIGN";
        case NodeType::GLOBALVARDEF: return "GLOBALVARDEF";
        case NodeType::ADD: return "ADD";
        case NodeType::SUB: return "SUB";
        case NodeType::MUL: return "MUL";
        case NodeType::DIV: return "DIV";
        case NodeType::LT: return "LT";
        case NodeType::GT: return "GT";
        case NodeType::GTE: return "GTE";
        case NodeType::LTE: return "LTE";
        case NodeType::EQ: return "EQ";
        case NodeType::NEQ: return "NEQ";
        case NodeType::MODULO: return "MODULO";
        case NodeType::LOOPDEF: return "LOOPDEF";
        case NodeType::LOOPCOND: return "LOOPCOND";
        case NodeType::FORLOOP: return "FORLOOP";
        case NodeType::WHILELOOP: return "WHILELOOP";
        case NodeType::LOOPITERATION: return "LOOPITERATION";
        case NodeType::INT32: return "INT32";
        case NodeType::INT64: return "INT64";
        case NodeType::FP32: return "FP32";
        case NodeType::FP64: return "FP64";
        case NodeType::STR: return "STR";
        case NodeType::CHAR: return "CHAR";
        case NodeType::BOOL_TR: return "BOOL_TR";
        case NodeType::UNIFIED: return "UNIFIED";
        case NodeType::SINGULAR: return "SINGULAR";
        case NodeType::TYPE: return "TYPE";
        case NodeType::VOID: return "VOID";
        case NodeType::BOOL_FA: return "BOOL_FA";
        case NodeType::NUM32: return "NUM32";
        case NodeType::NUM64: return "NUM64";
        case NodeType::NUMBER: return "NUMBER";
        case NodeType::ARRAY_TYPE: return "ARRAY_TYPE";
        case NodeType::MAP: return "MAP";
        case NodeType::FUNCTIONCHAIN: return "FUNCTIONCHAIN";
        case NodeType::UNSIGNED: return "UNSIGNED";
        case NodeType::STRUCT: return "STRUCT";
        case NodeType::BOOL: return "BOOL";
        case NodeType::SHORT: return "SHORT";
        case NodeType::RAWBIN: return "RAWBIN";
        case NodeType::RAWHEX: return "RAWHEX";
        case NodeType::ARRAY_SIZE: return "ARRAY_SIZE";
        case NodeType::ARRAY: return "ARRAY";
        case NodeType::ARRAY_ACCESS: return "ARRAY_ACCESS";
        case NodeType::ARRAY_INDEX: return "ARRAY_INDEX";
        case NodeType::FINAL: return "FINAL";
        case NodeType::STATIC: return "STATIC";
        case NodeType::PUBLIC: return "PUBLIC";
        case NodeType::PRIVATE: return "PRIVATE";
        case NodeType::IMPLICIT: return "IMPLICIT";
        case NodeType::CONSTANT: return "CONSTANT";
        case NodeType::NATIVE: return "NATIVE";
        case NodeType::FUNCTION_T: return "FUNCTION_T";
        case NodeType::INT32_T: return "INT32_T";
        case NodeType::INT64_T: return "INT64_T";
        case NodeType::FP32_T: return "FP32_T";
        case NodeType::FP64_T: return "FP64_T";
        case NodeType::STRING_T: return "STRING_T";
        case NodeType::CHAR_T: return "CHAR_T";
        case NodeType::BOOL_T: return "BOOL_T";
        case NodeType::STR_T: return "STR_T";
        case NodeType::TYPE_T: return "TYPE_T";
        case NodeType::USERTYPE_T: return "USERTYPE_T";
        case NodeType::CAST: return "CAST";
        case NodeType::VOID_T: return "VOID_T";
        case NodeType::STAR: return "STAR";
        case NodeType::FUNCTION_STAR: return "FUNCTION_STAR";
        case NodeType::INT32_STAR: return "INT32_STAR";
        case NodeType::INT64_STAR: return "INT64_STAR";
        case NodeType::FP32_STAR: return "FP32_STAR";
        case NodeType::FP64_STAR: return "FP64_STAR";
        case NodeType::UNIFIED_STAR: return "UNIFIED_STAR";
        case NodeType::STRING_STAR: return "STRING_STAR";
        case NodeType::CHAR_STAR: return "CHAR_STAR";
        case NodeType::BOOL_STAR: return "BOOL_STAR";
        case NodeType::STR_STAR: return "STR_STAR";
        case NodeType::TYPE_STAR: return "TYPE_STAR";
        case NodeType::USERTYPE_STAR: return "USERTYPE_STAR";
        case NodeType::CAST_STAR: return "CAST_STAR";
        case NodeType::VOID_STAR: return "VOID_STAR";
        case NodeType::STRUCT_STAR: return "STRUCT_STAR";
        case NodeType::OPENPARENS: return "OPENPARENS";
        case NodeType::CLOSEPARENS: return "CLOSEPARENS";
        case NodeType::OPENBRKT: return "OPENBRKT";
        case NodeType::CLOSEBRKT: return "CLOSEBRKT";
        case NodeType::ENDOFLINE: return "ENDOFLINE";
        case NodeType::COMMADELIMIT: return "COMMADELIMIT";
        case NodeType::SQUOTE: return "SQUOTE";
        case NodeType::DQUOTE: return "DQUOTE";
        case NodeType::COMMENT: return "COMMENT";
        case NodeType::MLCOMMENTSTART: return "MLCOMMENTSTART";
        case NodeType::MLCOMMENTEND: return "MLCOMMENTEND";
        case NodeType::OPENBRACE: return "OPENBRACE";
        case NodeType::CLOSEBRACE: return "CLOSEBRACE";
        case NodeType::ASSUME: return "ASSUME";
        case NodeType::USING: return "USING";
        case NodeType::ATTRIB: return "ATTRIB";
        case NodeType::ENDOFSTREAM: return "ENDOFSTREAM";
        case NodeType::CONDITION: return "CONDITION";
        case NodeType::FSLASH: return "FSLASH";
        case NodeType::BSLASH: return "BSLASH";
        case NodeType::RETURN: return "RETURN";
        case NodeType::RETFUNC: return "RETFUNC";
        case NodeType::RETNAT: return "RETNAT";
        case NodeType::RETTYPE: return "RETTYPE";
        case NodeType::IF: return "IF";
        case NodeType::ELSE: return "ELSE";
        case NodeType::IFRESULTS: return "IFRESULTS";
        case NodeType::TERNARY: return "TERNARY";
        case NodeType::SWITCH: return "SWITCH";
        case NodeType::CASE: return "CASE";
        case NodeType::BITAND: return "BITAND";
        case NodeType::BITOR: return "BITOR";
        case NodeType::BITXOR: return "BITXOR";
        case NodeType::BITNEGATE: return "BITNEGATE";
        case NodeType::NOT: return "NOT";
        case NodeType::ARITHRIGHTSHIFT: return "ARITHRIGHTSHIFT";
        case NodeType::ARITHLEFTSHIFT: return "ARITHLEFTSHIFT";
        case NodeType::LOGICRIGHTSHIFT: return "LOGICRIGHTSHIFT";
        case NodeType::LOGICAND: return "LOGICAND";
        case NodeType::LOGICOR: return "LOGICOR";
        case NodeType::LOGICXOR: return "LOGICXOR";
        case NodeType::LOGICNEGATE: return "LOGICNEGATE";
        case NodeType::SWITCHSIGN: return "SWITCHSIGN";
        case NodeType::NODE: return "NODE";
        case NodeType::PRINT: return "PRINT";
        case NodeType::SYSCALL: return "SYSCALL";
        case NodeType::PTRDEREF: return "PTRDEREF";
        case NodeType::FINDADDRESS: return "FINDADDRESS";
        case NodeType::INC: return "INC";
        case NodeType::DEC: return "DEC";
        case NodeType::PROGRAM: return "PROGRAM";
        case NodeType::FORWARD: return "FORWARD";
        case NodeType::INVALID: return "INVALID";
        case NodeType::MEMBER: return "MEMBER";
        case NodeType::FUNCTION: return "FUNCTION";
        case NodeType::FUNCTIONAPP: return "FUNCTIONAPP";
        case NodeType::ARGUMENT: return "ARGUMENT";
        case NodeType::FUNCTIONRETURN: return "FUNCTIONRETURN";
        case NodeType::BLOCKSTART: return "BLOCKSTART";
        case NodeType::BLOCKEND: return "BLOCKEND";
        case NodeType::FUNCTIONNAME: return "FUNCTIONNAME";
        case NodeType::FUNCTIONPARAM: return "FUNCTIONPARAM";
        case NodeType::FUNCTIONLINE: return "FUNCTIONLINE";
        case NodeType::FUNCTIONCALL: return "FUNCTIONCALL";
        case NodeType::FUNCTIONBODY: return "FUNCTIONBODY";
        case NodeType::DIRECTIVE: return "DIRECTIVE";
        case NodeType::DIRTYPE: return "DIRTYPE";
        case NodeType::DIRVALUE: return "DIRVALUE";
        case NodeType::DIRIMPORT: return "DIRIMPORT";
        case NodeType::DIRLIBDEF: return "DIRLIBDEF";
        case NodeType::DIRPRIVATE: return "DIRPRIVATE";
        case NodeType::DIRALIAS: return "DIRALIAS";
        case NodeType::DIRUSERTYPE: return "DIRUSERTYPE";
        default: return "UNKNOWN_NODETYPE";
    }
}

    /*
        Convert raw ptr to string based on dtype
        Should be fine since internal types remain static
        once set or are changed to something that maps
        to the same string
    */
    inline std::string getStringValue(NodeType type, void* data) {
        if(!data) return "VOID";
        
        switch(type) {
            case NodeType::INT32: {
                int32_t value = *static_cast<int32_t*>(data);
                return std::to_string(value);
            }
            case NodeType::INT64: {
                int64_t value = *static_cast<int64_t*>(data);
                return std::to_string(value);
            }
            case NodeType::FP32: {
                float value = *static_cast<float*>(data);
                return std::to_string(value);
            }
            case NodeType::FP64: {
                double value = *static_cast<double*>(data);
                return std::to_string(value);
            }
            case NodeType::VARIABLE: 
            case NodeType::STR:
            case NodeType::FUNCTION: {
                return *static_cast<std::string*>(data);
            }
            case NodeType::CAST:
                return nodeTypeToString(*static_cast<NodeType*>(data));
            default: {
                return "VOID";
            }
        }
    }
} // namespace nvyc::data