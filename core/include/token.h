#pragma once
#include <string>
#include <optional>

enum class TokenType
{
    
    Identifier,
    Keyword,
    Number,
    String,
    Boolean,
    Null,
    
    
    Operator,
    Arrow,
    
    
    Plus,           
    Minus,          
    Multiply,       
    Divide,         
    Modulo,         
    Power,          
    
    
    Assign,         
    PlusAssign,     
    MinusAssign,    
    MultiplyAssign, 
    DivideAssign,   
    
    
    Equal,          
    NotEqual,       
    Less,           
    LessEqual,      
    Greater,        
    GreaterEqual,   
    
    
    LogicalAnd,     
    LogicalOr,      
    LogicalNot,     

    
    TagOpen,
    TagClose,
    TagSelfClose,
    TagEnd,
    AttributeName,
    AttributeValue,

    Text,
    Comment,

    
    ExpressionStart,    
    ExpressionEnd,      

    
    Equals,
    BraceOpen,         
    BraceClose,        
    Colon,             
    SemiColon,         
    ParenOpen,         
    ParenClose,        
    SquareBracketOpen,       
    SquareBracketClose,      
    Comma,             
    Dot,               

    
    AtModifier,        
    ValueBinding,      
    StyleProperty,
    
    
    AsyncBlockStart,   
    AsyncTryStart,     
    AsyncCatchStart,   
    AsyncFinallyStart, 
    AsyncBlockEnd,     
    
    
    IfStart,           
    ForStart,          
    WhileStart,        
    ControlBlockStart, 
    ControlBlockEnd,   
    
    
    EOFToken,
    Unknown,
    Error,
    ErrorRecovery,
    PowerAssign,      
    SlashGreater,     
    Slash,            
};

enum class LexerState
{
    Normal,
    ALTXAttribute,
    ALTXContent,
    ALTSScript,
    Expression,
    StyleValue
};

class Token
{
public:
    TokenType type;
    std::string value;
    size_t line;
    size_t column;
    std::optional<std::string> error;
    std::string errorMessage;

public:
    
    Token(TokenType t, const std::string &v, size_t l, size_t c)
        : type(t), value(v), line(l), column(c), error(std::nullopt), errorMessage("") {}

    
    Token(TokenType t, const char *v, size_t l, size_t c)
        : type(t), value(v), line(l), column(c), error(std::nullopt), errorMessage("") {}

    
    Token(TokenType t, const std::string &v, size_t l, size_t c, const std::string &errorMsg)
        : type(t), value(v), line(l), column(c), error(std::make_optional(errorMsg)), errorMessage(errorMsg) {}

    TokenType getType() const { return type; }
    const std::string &getValue() const { return value; }
    size_t getLine() const { return line; }
    size_t getColumn() const { return column; }
    const std::optional<std::string> &getError() const { return error; }
    const std::string &getErrorMessage() const { return errorMessage; }
    
    
    std::string toString() const {
        std::string result = tokenTypeToString(type) + ": \"" + value + "\"";
        if (error) {
            result += " [ERROR: " + *error + "]";
        }
        return result;
    }
    
private:
    static std::string tokenTypeToString(TokenType type) {
        switch (type) {
            case TokenType::Identifier: return "IDENTIFIER";
            case TokenType::Keyword: return "KEYWORD";
            case TokenType::Number: return "NUMBER";
            case TokenType::String: return "STRING";
            case TokenType::Boolean: return "BOOLEAN";
            case TokenType::Null: return "NULL";
            case TokenType::Plus: return "PLUS";
            case TokenType::Minus: return "MINUS";
            case TokenType::Multiply: return "MULTIPLY";
            case TokenType::Divide: return "DIVIDE";
            case TokenType::Power: return "POWER";
            case TokenType::Assign: return "ASSIGN";
            case TokenType::PlusAssign: return "PLUS_ASSIGN";
            case TokenType::Equal: return "EQUAL";
            case TokenType::NotEqual: return "NOT_EQUAL";
            case TokenType::LogicalAnd: return "LOGICAL_AND";
            case TokenType::LogicalOr: return "LOGICAL_OR";
            case TokenType::AtModifier: return "AT_MODIFIER";
            case TokenType::ValueBinding: return "VALUE_BINDING";
            case TokenType::AsyncBlockStart: return "ASYNC_BLOCK_START";
            case TokenType::BraceOpen: return "BRACE_OPEN";
            case TokenType::BraceClose: return "BRACE_CLOSE";
            case TokenType::ParenOpen: return "PAREN_OPEN";
            case TokenType::ParenClose: return "PAREN_CLOSE";
            case TokenType::SquareBracketOpen: return "SQUARE_BRACKET_OPEN";
            case TokenType::SquareBracketClose: return "BRACKET_CLOSE";
            case TokenType::EOFToken: return "EOF";
            default: return "UNKNOWN";
        }
    }
};