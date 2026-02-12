#include "Lexer.hpp"
#include "error/Error.hpp"
#include "error/Debug.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>

// There is probably a better way to format all of this

const std::unordered_set<NodeType> nvyc::Lexer::NUMERICS = {
    NodeType::INT32, NodeType::INT64, NodeType::FP32, NodeType::FP64
};

nvyc::Lexer& nvyc::Lexer::getInstance() {
    static Lexer instance;
    return instance;
}

nvyc::Lexer::Lexer() {
    init();
}

void nvyc::Lexer::init() {
    // Keywords
    rep["let"] = NodeType::VARDEF;
    rep["true"] = NodeType::BOOL_T;
    rep["false"] = NodeType::BOOL_FA;

    // Symbols
    rep["+"] = NodeType::ADD;
    rep["-"] = NodeType::SUB;
    rep["/"] = NodeType::DIV;
    rep["*"] = NodeType::MUL;
    rep["~"] = NodeType::BITNEGATE;
    rep["&"] = NodeType::BITAND;
    rep["|"] = NodeType::BITOR;
    rep["^"] = NodeType::BITXOR;
    rep[">"] = NodeType::GT;
    rep["<"] = NodeType::LT;
    rep["func"] = NodeType::FUNCTION;
    rep["!"] = NodeType::NOT;
    rep["?"] = NodeType::TERNARY;
    rep["."] = NodeType::ATTRIB;

    // Conditionals
    rep["if"] = NodeType::IF;
    rep["else"] = NodeType::ELSE;
    rep["switch"] = NodeType::SWITCH;
    rep["case"] = NodeType::CASE;
    rep["return"] = NodeType::RETURN;
    rep["for"] = NodeType::FORLOOP;
    rep["while"] = NodeType::WHILELOOP;

    // Types
    rep["int32"] = NodeType::INT32_T;
    rep["int64"] = NodeType::INT64_T;
    rep["unsigned"] = NodeType::UNSIGNED;
    rep["fp32"] = NodeType::FP32_T;
    rep["fp64"] = NodeType::FP64_T;
    rep["string"] = NodeType::STR_T;
    rep["char"] = NodeType::CHAR_T;
    rep["bool"] = NodeType::BOOL_T;
    rep["type"] = NodeType::TYPE_T;
    rep["short"] = NodeType::SHORT;
    rep["numeric32"] = NodeType::NUM32;
    rep["numeric64"] = NodeType::NUM64;
    rep["unified"] = NodeType::UNIFIED;
    rep["function"] = NodeType::FUNCTION_T;
    rep["void"] = NodeType::VOID;

    // Modifiers
    rep["final"] = NodeType::FINAL;
    rep["static"] = NodeType::STATIC;
    rep["public"] = NodeType::PUBLIC;
    rep["private"] = NodeType::PRIVATE;
    rep["impl"] = NodeType::IMPLICIT;
    rep["constant"] = NodeType::CONSTANT;
    rep["native"] = NodeType::NATIVE;
    rep["ref"] = NodeType::FINDADDRESS;
    rep["struct"] = NodeType::STRUCT;

    // Delimiters
    rep["("] = NodeType::OPENPARENS;
    rep[")"] = NodeType::CLOSEPARENS;
    rep["="] = NodeType::ASSIGN;
    rep[";"] = NodeType::ENDOFLINE;
    rep[","] = NodeType::COMMADELIMIT;
    rep["["] = NodeType::OPENBRKT;
    rep["]"] = NodeType::CLOSEBRKT;
    rep["\""] = NodeType::DQUOTE;
    rep["'"] = NodeType::SQUOTE;
    rep["{"] = NodeType::OPENBRACE;
    rep["}"] = NodeType::CLOSEBRACE;
    rep["\\"] = NodeType::BSLASH;
}

bool nvyc::Lexer::isNumericLiteral(const std::string& s) {
    if (s.empty()) return false;

    bool hasDigit = false;

    for (char c : s) {
        if (isdigit(c)) {
            hasDigit = true;
            continue;
        }
        if (c == '.' || c == 'e' || c == 'E' ||
            c == 'F' || c == 'D' || c == '_' ||
            c == '+' || c == '-') {
            continue;
        }
        return false; 
    }

    return hasDigit;
}

NodeType nvyc::Lexer::numericNativeType(const std::string& s) const {
    char back = s.back();
    std::string literal = s;
    nvyc::debug("Numeric: " + s);
    // Float
    if (
        s.find('.') != std::string::npos ||
        back == 'F' || back == 'D'
    ) {

        if(back == 'F' || back == 'D') literal.pop_back();
        double value_float;
        
        try {
            value_float = std::stod(literal);
        } catch (...) {
            nvyc::Error::nvyerr_failcompile(-1, "Invalid number: " + s);
            return NodeType::INVALID;
        }

        if(back == 'F') return NodeType::FP32;
        else if(back == 'D') return NodeType::FP64;
        
        else if(
            value_float >= std::numeric_limits<double>::lowest() &&
            value_float <= std::numeric_limits<double>::max()
        ) {
            return NodeType::FP64;
        }
    } 
    
    else {
        if(back == 'L') literal.pop_back();
        int64_t value_int;

        try {
            value_int = std::stoll(literal);
        } catch (...) {
            nvyc::Error::nvyerr_failcompile(-1, "Invalid number: " + s);
            return NodeType::INVALID;
        }

        if(back == 'L') return NodeType::INT64;

        if(
            value_int >= std::numeric_limits<int32_t>::lowest() &&
            value_int <= std::numeric_limits<int32_t>::max()    
        ) {
            return NodeType::INT32;
        } 
        
        return NodeType::INT64;
    }
}

NodeStream* nvyc::Lexer::lex(const std::vector<std::string>& lines) {
    NodeStream* head = new NodeStream(NodeType::PROGRAM, NULL_VALUE);
    NodeStream* sentinel = head;
    NodeStream* current;

    int lineNumber = 0;

    for(const std::string line : lines) {
        int i = 0;
        while (i < line.length()) {
            char ch = line[i];

            // Skip whitespace
            if(isspace(ch)) {
                i++;
                continue;
            }

            std::string symbol(1, ch);
            if(rep.count(symbol)) {
                NodeType type = rep[symbol];

                // Quoted string ("Hello World")
                if(type == NodeType::DQUOTE) {

                    // TODO does not account for escaped quotes
                    std::string value(1, ch);
                    i++;
                    while(i < line.length() && line[i] != '"') {
                        value += line[i];
                        i++;
                    }
                    if (i < line.length() && line[i] == '"') value += '"';
                    i++;
                    current = new NodeStream(NodeType::STR, Value(value));
                }

                // Single chars ('c')
                else if(type == NodeType::SQUOTE) {
                    std::string value(1, ch);
                    i++;
                    while(i < line.length() && line[i] != '\'') {
                        value += line[i];
                        i++;
                    }
                    if(i < line.length()) {
                        value += '\'';
                    }
                    i++;
                    current = new NodeStream(NodeType::CHAR, Value(value));
                }

                // Otherwise, assign type
                else {
                    current = new NodeStream(type, Value(symbol));
                    i++;
                }

                head->setNext(current);
                current->setPrev(head);
                head = head->getNext();
                continue;
            }

            if(isalnum(ch) || ch == '_') {
                std::string value;
                while(i < line.length() && (isalnum(line[i]) || line[i] == '_' || line[i] == '.')) {
                    value += line[i];
                    i++;
                }

                NodeType type;
                auto it = rep.find(value);

                if(it != rep.end()) type = it->second;
                else if(isNumericLiteral(value)) type = numericNativeType(value);
                else type = NodeType::VARIABLE;

                if(NUMERICS.count(type)) current = convertNumeric(type, value);
                else current = new NodeStream(type, Value(value));
            
                head->setNext(current);
                current->setPrev(head);
                head = head->getNext();
                continue;
            }
            
            // Unknown fallback
            current = new NodeStream(NodeType::VARIABLE, Value(std::string(1, ch)));
            head->setNext(current);
            current->setPrev(head);
            head = head->getNext();
            continue;
        }
        lineNumber++;
    }

    current = new NodeStream(NodeType::ENDOFSTREAM, NULL_VALUE);
    current->setPrev(head);
    head->setNext(current);

    return sentinel;
}

NodeStream* nvyc::Lexer::convertNumeric(NodeType type, const std::string& value) {
    switch(type) {
        case NodeType::INT32:
            return new NodeStream(type, Value(std::stoi(value)));
        case NodeType::INT64:
            return new NodeStream(type, Value(std::stoll(value)));
        case NodeType::FP32:
            return new NodeStream(type, Value(std::stof(value)));
        case NodeType::FP64:
            return new NodeStream(type, Value(std::stod(value)));
        default:
            return new NodeStream(NodeType::VARIABLE, Value(value));
    }
}