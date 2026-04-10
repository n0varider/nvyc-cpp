#include "Lexer.hpp"
#include "error/Error.hpp"
#include "error/Debug.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>

using nvyc::NodeStream;

// There is probably a better way to format all of this

const std::unordered_set<NodeType> nvyc::Lexer::NUMERICS = {
    NodeType::INT32, NodeType::INT64, NodeType::FP32, NodeType::FP64
};

// Delimiters are any non-operator, single character symbol
const std::unordered_set<char> DELIMITERS = {
    '(', ')', 
    '[', ']',
    '{', '}',
    ',', ';', '\'', '\"', '\\',

};

const std::unordered_set<std::string> IDENTIFIERS = {
    "let", "if", "else", "switch", "case", "return",
    "true", "false", "for", "while", 

    "final", "static", "public", "private",
    "impl", "const", "native", "struct", "ref",

    "int32", "int64", "fp32", "fp64", "bool",
    "short", "char", "string", "void", "function",
    "func", "type", "unified", "ptr_t"
};

const std::unordered_set<std::string> OPERATORS = {
    "+", "-", "/", "*", ".", "?",
    "&", "&&",
    "|", "||",
    "^", "^^",
    "<", "<=",
    ">", ">=",
    "=", "=="
    "++", "--",
    "!", "~",
    "->"
};

const std::unordered_set<char> NUMERIC_QUALIFIERS = {
    'f', 'F', 'd', 'D', 'L', 'U'
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
    rep["&&"] = NodeType::LOGICAND;
    rep["|"] = NodeType::BITOR;
    rep["||"] = NodeType::LOGICOR;
    rep["^"] = NodeType::BITXOR;
    rep["^^"] = NodeType::LOGICXOR;
    rep[">"] = NodeType::GT;
    rep["<"] = NodeType::LT;
    rep["func"] = NodeType::FUNCTION;
    rep["!"] = NodeType::NOT;
    rep["?"] = NodeType::TERNARY;
    rep["."] = NodeType::ATTRIB;
    rep["->"] = NodeType::RETTYPE;

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
    rep["ptr_t"] = NodeType::PTR_TYPE;

    // Modifiers
    rep["final"] = NodeType::FINAL;
    rep["static"] = NodeType::STATIC;
    rep["public"] = NodeType::PUBLIC;
    rep["private"] = NodeType::PRIVATE;
    rep["impl"] = NodeType::IMPLICIT;
    rep["const"] = NodeType::CONSTANT;
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
    NodeStream* head = new NodeStream();


    // For debugging
    int lineNumber = 1;

    for(const std::string line: lines) {
        int i = 0;
        while(i < line.length()) {
            char ch = line[i];
            std::string currentToken;

            /*
            
            
            Use longest valid subtoken
            
            let x12 = 152;
            - let (SPACE)
            - x12 (SPACE)
            - = (SPACE)
            - 152 (SWITCH TOKEN TYPE)
            - ; (EOL)



            PROCESS:

            do token identification and consumption
            let x12 = 152;

            isalpha(let) { consume let } loop
            isalpha(x) build until next valid token (=)) loop
            isOperator(=) loop
            isdigit(152) loop


            */

            if(isspace(ch)) {
                i++;
            }

            // Identifier / Keyword
            else if(isalpha(ch)) {

                int j = i;
                char currentChar = line[j];
                NodeType type;
                std::string longestToken = "";

                // Make sure token is not a delimiter or operator, and keep building
                while(
                    j < line.length() && 
                    !isspace(currentChar) && 
                    !DELIMITERS.count(currentChar) && 
                    !OPERATORS.count(std::string(1, currentChar))
                ) {
                    currentToken += currentChar;
                    j++;
                    currentChar = line[j];
                    
                    if(IDENTIFIERS.count(currentToken)) {
                        longestToken = currentToken;
                    }

                }
                if(longestToken == "") {
                    type = NodeType::VARIABLE;
                    longestToken = currentToken;
                }else{
                    type = rep[longestToken];
                }

                head->addNode(type, Value(longestToken), lineNumber);
                i += longestToken.length();
            }


            // Delimiters are single character, so consume immediately
            else if(DELIMITERS.count(ch)) {
                head->addNode(rep[std::string(1, ch)], Value(std::string(1,ch)), lineNumber);
                i++;
            }


            // Operators can be multiple characters, so loop until longest token is found
            else if(OPERATORS.count(std::string(1, ch))) {
                int j = i;
                char currentChar = line[j];
                NodeType type;
                std::string longestToken = "";

                // Only need to see if next token is an operator, since anything else will immediately consume
                while(!isspace(currentChar) && OPERATORS.count(std::string(1, currentChar))) {
                    longestToken += currentChar;
                    j++;
                    currentChar = line[j];
                }

                head->addNode(rep[longestToken], Value(longestToken), lineNumber);
                i += longestToken.length();
            }
            

            // Numbers
            else if(isdigit(ch)) {
                int j = i;
                char currentChar = line[j];
                std::string number;

                while(isdigit(currentChar) || currentChar == '.' || NUMERIC_QUALIFIERS.count(currentChar)) {
                    number += currentChar;
                    j++;
                    currentChar = line[j];
                }

                NodeType type = numericNativeType(number);
                head->addNode(type, convertNumeric(type, number), lineNumber);
                i += number.length();
            }
            
            else{
                nvyc::Error::nvyerr_out("Encountered unknown token type: ");
                nvyc::Error::nvyerr_out(std::string(1, ch));
                i++;
            }

            // Reset token
            currentToken = "";
        }
        lineNumber++;
    }
    return head;
}

nvyc::Value nvyc::Lexer::convertNumeric(NodeType type, const std::string& value) {
    switch(type) {
        case NodeType::INT32:
            return Value(std::stoi(value));
        case NodeType::INT64:
            return Value(std::stoll(value));
        case NodeType::FP32:
            return Value(std::stof(value));
        case NodeType::FP64:
            return Value(std::stod(value));
        default:
            return Value(value);
    }
}