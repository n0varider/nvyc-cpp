#pragma once

#include "NodeType.hpp"
#include "Symbols.hpp"
#include <cstdint>
#include <string>

namespace nvyc {


    struct Value {
        NodeType type = NodeType::INVALID;
        std::string str;
        union {
            int8_t  i8;
            int32_t i32;
            int64_t i64;
            float   f32;
            double  f64;
            NodeType ty;
        };

        Value() : type(NodeType::INVALID) {}
        Value(int8_t v)      : type(NodeType::CHAR), i8(v) {}
        Value(int32_t v)     : type(NodeType::INT32), i32(v) {}
        Value(int64_t v)     : type(NodeType::INT64), i64(v) {}
        Value(float v)       : type(NodeType::FP32), f32(v) {}
        Value(double v)      : type(NodeType::FP64), f64(v) {}
        Value(NodeType v)    : type(NodeType::TYPE), ty(v) {}
        Value(const std::string& v) : type(NodeType::STR), str(v) {}

        std::string asString() const {
            switch(type) {
                case NodeType::STR: return str;
                case NodeType::CHAR: return std::to_string(i8);
                case NodeType::INT32: return std::to_string(i32);
                case NodeType::INT64: return std::to_string(i64);
                case NodeType::FP32: return std::to_string(f32);
                case NodeType::FP64: return std::to_string(f64);
                case NodeType::TYPE: return symbols::nodeTypeToString(ty);
                default: return "unknown";
            }
            return "unknown";
        }
    };

    inline const Value NULL_VALUE = Value(NodeType::VOID);

} // namespace nvyc

