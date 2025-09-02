#include "lexer.h"
#include <cctype>
#include <iostream>
#include <iomanip>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>

// Complete Alterion Language Lexer Implementation

namespace {
    // Keywords from the Alterion Blueprint
    const std::unordered_set<std::string> KEYWORDS = {
        // Control flow
        "async", "component", "import", "extern", "for", "if", "else", "while", 
        "return", "break", "continue", "yield", "await", "throw", "try", "catch", "finally",
        
        // Components and functions
        "render", "script", "function", "fn", "method",
        
        // Type system
        "type", "interface", "struct", "enum", "union",
        
        // Module system
        "from", "export", "use", "pub", "mod", "namespace",
        
        // Memory management
        "move", "copy", "ref", "deref", "owned", "borrowed", "shared", "weak",
        
        // Literals
        "true", "false", "null", "none", "undefined",
        
        // Special
        "this", "super", "self", "Self", "match", "case", "default",
        "let", "const", "var", "new", "delete", "as", "is", "typeof",
        "print", "println"
    };

    // Multi-character operators
    const std::unordered_map<std::string, TokenType> MULTI_CHAR_OPERATORS = {
        {"=>", TokenType::Arrow},
        {"->", TokenType::Arrow}, 
        {"==", TokenType::Operator},
        {"!=", TokenType::Operator},
        {"<=", TokenType::Operator},
        {">=", TokenType::Operator},
        {"&&", TokenType::Operator},
        {"||", TokenType::Operator},
        {"**", TokenType::Operator},
        {"++", TokenType::Operator},
        {"--", TokenType::Operator},
        {"+=", TokenType::Operator},
        {"-=", TokenType::Operator},
        {"*=", TokenType::Operator},
        {"/=", TokenType::Operator},
        {"%=", TokenType::Operator}
    };
}

Lexer::Lexer(const std::string& source) 
    : input(source), position(0), line(1), column(1), state(LexerState::Normal), isUTF8Error(false) {
    stateStack.reserve(8);
}

char Lexer::peek() const {
    return eof() ? '\0' : input[position];
}

char Lexer::peekAdvance() const {
    return (position + 1 < input.size()) ? input[position + 1] : '\0';
}

char Lexer::advance() {
    if (eof()) return '\0';
    
    char current = input[position++];
    if (current == '\n') {
        ++line;
        column = 1;
    } else {
        ++column;
    }
    return current;
}

bool Lexer::match(char expected) {
    if (eof() || peek() != expected) return false;
    advance();
    return true;
}

bool Lexer::eof() const {
    return position >= input.size();
}

void Lexer::enterState(LexerState newState) {
    stateStack.push_back(state);
    state = newState;
}

void Lexer::exitState() {
    if (!stateStack.empty()) {
        state = stateStack.back();
        stateStack.pop_back();
    } else {
        state = LexerState::Normal;
    }
}

void Lexer::skipWhitespace() {
    while (!eof()) {
        char c = peek();
        
        if (std::isspace(c)) {
            advance();
        } else if (c == '/' && peekAdvance() == '/') {
            // Single-line comment
            advance(); // '/'
            advance(); // '/'
            while (!eof() && peek() != '\n') {
                advance();
            }
        } else if (c == '/' && peekAdvance() == '*') {
            // Multi-line comment
            advance(); // '/'
            advance(); // '*'
            while (!eof()) {
                if (peek() == '*' && peekAdvance() == '/') {
                    advance(); // '*'
                    advance(); // '/'
                    break;
                }
                advance();
            }
        } else {
            break;
        }
    }
}

bool Lexer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

bool Lexer::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::isAlphaNumeric(char c) const {
    return isAlpha(c) || isDigit(c);
}

bool Lexer::isOperatorStartChar(char c) const {
    return c == '=' || c == '!' || c == '<' || c == '>' || c == '&' || 
           c == '|' || c == '-' || c == '+' || c == '*' || c == '/' || 
           c == '%' || c == '^' || c == '~' || c == ':' || c == '.' || 
           c == ',' || c == ';';
}

Token Lexer::processNumber() {
    size_t startLine = line, startColumn = column;
    std::string number;
    bool hasDecimal = false;
    bool hasExponent = false;
    
    // Handle different number formats: decimal, hex, binary
    if (peek() == '0') {
        number += advance();
        if (peek() == 'x' || peek() == 'X') {
            // Hexadecimal
            number += advance();
            while (!eof() && (isDigit(peek()) || (peek() >= 'a' && peek() <= 'f') || (peek() >= 'A' && peek() <= 'F'))) {
                number += advance();
            }
        } else if (peek() == 'b' || peek() == 'B') {
            // Binary
            number += advance();
            while (!eof() && (peek() == '0' || peek() == '1')) {
                number += advance();
            }
        } else {
            // Continue with decimal
            while (!eof() && isDigit(peek())) {
                number += advance();
            }
        }
    } else {
        // Decimal number
        while (!eof() && isDigit(peek())) {
            number += advance();
        }
    }
    
    // Handle decimal point
    if (peek() == '.' && isDigit(peekAdvance())) {
        hasDecimal = true;
        number += advance(); // '.'
        while (!eof() && isDigit(peek())) {
            number += advance();
        }
    }
    
    // Handle scientific notation
    if (peek() == 'e' || peek() == 'E') {
        hasExponent = true;
        number += advance();
        if (peek() == '+' || peek() == '-') {
            number += advance();
        }
        while (!eof() && isDigit(peek())) {
            number += advance();
        }
    }
    
    return Token(TokenType::Number, number, startLine, startColumn);
}

Token Lexer::processIdentifierOrKeyword() {
    size_t startLine = line, startColumn = column;
    std::string text;
    
    while (!eof() && (isAlphaNumeric(peek()) || peek() == '-')) {
        text += advance();
    }
    
    // Check if it's a keyword
    if (KEYWORDS.count(text)) {
        return Token(TokenType::Keyword, text, startLine, startColumn);
    }
    
    return Token(TokenType::Identifier, text, startLine, startColumn);
}

Token Lexer::processString() {
    size_t startLine = line, startColumn = column;
    char quote = peek(); // Can be ' or "
    advance(); // Skip opening quote
    
    std::string value;
    
    while (!eof() && peek() != quote) {
        if (peek() == '\\') {
            advance(); // Skip backslash
            if (eof()) break;
            
            char escaped = advance();
            switch (escaped) {
                case 'n': value += '\n'; break;
                case 't': value += '\t'; break;
                case 'r': value += '\r'; break;
                case '\\': value += '\\'; break;
                case '\'': value += '\''; break;
                case '"': value += '"'; break;
                case '0': value += '\0'; break;
                default: value += escaped; break;
            }
        } else {
            value += advance();
        }
    }
    
    if (!eof()) {
        advance(); // Skip closing quote
    }
    
    return Token(TokenType::String, value, startLine, startColumn);
}

Token Lexer::processOperator() {
    size_t startLine = line, startColumn = column;
    char first = peek();
    char second = peekAdvance();
    
    // Try multi-character operators first
    std::string twoChar;
    twoChar += first;
    twoChar += second;
    
    if (MULTI_CHAR_OPERATORS.count(twoChar)) {
        advance(); // first char
        advance(); // second char
        return Token(MULTI_CHAR_OPERATORS.at(twoChar), twoChar, startLine, startColumn);
    }
    
    // Single character operators
    char op = advance();
    switch (op) {
        case '=': return Token(TokenType::Equals, "=", startLine, startColumn);
        case '+': case '-': case '*': case '/': case '%':
        case '<': case '>': case '!': case '&': case '|':
        case '^': case '~':
            return Token(TokenType::Operator, std::string(1, op), startLine, startColumn);
        case '(': return Token(TokenType::ParenOpen, "(", startLine, startColumn);
        case ')': return Token(TokenType::ParenClose, ")", startLine, startColumn);
        case '{': return Token(TokenType::BraceOpen, "{", startLine, startColumn);
        case '}': return Token(TokenType::BraceClose, "}", startLine, startColumn);
        case '[': return Token(TokenType::BracketOpen, "[", startLine, startColumn);
        case ']': return Token(TokenType::BracketClose, "]", startLine, startColumn);
        case ':': return Token(TokenType::Colon, ":", startLine, startColumn);
        case ';': return Token(TokenType::SemiColon, ";", startLine, startColumn);
        case ',': return Token(TokenType::Comma, ",", startLine, startColumn);
        case '.': return Token(TokenType::Dot, ".", startLine, startColumn);
        default: return Token(TokenType::Unknown, std::string(1, op), startLine, startColumn);
    }
}

Token Lexer::processTag() {
    size_t startLine = line, startColumn = column;
    advance(); // Skip '<'
    
    // Check for closing tag
    if (peek() == '/') {
        return processTagEnd();
    }
    
    std::string tagName;
    while (!eof() && (isAlphaNumeric(peek()) || peek() == '-')) {
        tagName += advance();
    }
    
    if (tagName.empty()) {
        return Token(TokenType::Error, "<", startLine, startColumn, "Invalid tag: expected tag name");
    }
    
    enterState(LexerState::ALTXAttribute);
    return Token(TokenType::TagOpen, tagName, startLine, startColumn);
}

Token Lexer::processTagEnd() {
    size_t startLine = line, startColumn = column;
    advance(); // Skip '/'
    
    std::string tagName;
    while (!eof() && (isAlphaNumeric(peek()) || peek() == '-')) {
        tagName += advance();
    }
    
    // Expect '>'
    skipWhitespace();
    if (peek() == '>') {
        advance();
    }
    
    if (state == LexerState::ALTXContent) {
        exitState();
    }
    
    return Token(TokenType::TagClose, tagName, startLine, startColumn);
}

Token Lexer::processTextContent() {
    size_t startLine = line, startColumn = column;
    std::string text;
    
    while (!eof()) {
        char c = peek();
        if (c == '<' || c == '{' || std::isspace(c)) {
            break;
        }
        text += advance();
    }
    
    return Token(TokenType::Text, text, startLine, startColumn);
}

Token Lexer::processValueBinding() {
    size_t startLine = line, startColumn = column;
    advance(); // Skip '!'
    
    std::string identifier;
    while (!eof() && (isAlphaNumeric(peek()) || peek() == '_')) {
        identifier += advance();
    }
    
    if (identifier.empty()) {
        return Token(TokenType::Error, "!", startLine, startColumn, "Expected identifier after '!'");
    }
    
    return Token(TokenType::ValueBinding, "!" + identifier, startLine, startColumn);
}

Token Lexer::processStyleProperty() {
    size_t startLine = line, startColumn = column;
    std::string property;
    
    // Read until we hit a delimiter
    while (!eof() && peek() != ':' && peek() != ';' && peek() != '}' && !std::isspace(peek())) {
        property += advance();
    }
    
    return Token(TokenType::StyleProperty, property, startLine, startColumn);
}

Token Lexer::nextToken() {
    skipWhitespace();
    
    if (eof()) {
        return Token(TokenType::EOFToken, "", line, column);
    }
    
    char c = peek();
    size_t startLine = line, startColumn = column;
    
    // Handle different lexer states
    switch (state) {
        case LexerState::ALTXAttribute:
            return processALTXAttribute();
            
        case LexerState::ALTXContent:
            return processALTXContent();
            
        case LexerState::Expression:
            return processExpression();
            
        case LexerState::StyleValue:
            return processStyleValue();
            
        default:
            break;
    }
    
    // Normal state processing
    if (isDigit(c)) {
        return processNumber();
    }
    
    if (isAlpha(c)) {
        return processIdentifierOrKeyword();
    }
    
    if (c == '"' || c == '\'') {
        return processString();
    }
    
    if (c == '<') {
        return processTag();
    }
    
    if (c == '@') {
        advance();
        std::string modifier;
        while (!eof() && isAlphaNumeric(peek())) {
            modifier += advance();
        }
        return Token(TokenType::AtModifier, "@" + modifier, startLine, startColumn);
    }
    
    if (c == '!') {
        return processValueBinding();
    }
    
    if (c == '{') {
        advance();
        enterState(LexerState::Expression);
        return Token(TokenType::ExpressionStart, "{", startLine, startColumn);
    }
    
    if (isOperatorStartChar(c)) {
        return processOperator();
    }
    
    // Unknown character
    return Token(TokenType::Unknown, std::string(1, advance()), startLine, startColumn);
}

Token Lexer::processALTXAttribute() {
    skipWhitespace();
    
    if (eof()) {
        exitState();
        return Token(TokenType::EOFToken, "", line, column);
    }
    
    char c = peek();
    size_t startLine = line, startColumn = column;
    
    // Handle tag endings
    if (c == '>') {
        advance();
        state = LexerState::ALTXContent; // Switch to content parsing
        return Token(TokenType::TagEnd, ">", startLine, startColumn);
    }
    
    if (c == '/' && peekAdvance() == '>') {
        advance(); // '/'
        advance(); // '>'
        exitState();
        return Token(TokenType::TagSelfClose, "/>", startLine, startColumn);
    }
    
    // Handle expressions
    if (c == '{') {
        advance();
        enterState(LexerState::Expression);
        return Token(TokenType::ExpressionStart, "{", startLine, startColumn);
    }
    
    // Handle attribute names
    if (isAlpha(c)) {
        std::string attrName;
        while (!eof() && (isAlphaNumeric(peek()) || peek() == '-' || peek() == '_')) {
            attrName += advance();
        }
        return Token(TokenType::AttributeName, attrName, startLine, startColumn);
    }
    
    // Handle other tokens
    if (c == '=') {
        advance();
        return Token(TokenType::Equals, "=", startLine, startColumn);
    }
    
    if (c == '"' || c == '\'') {
        return processString();
    }
    
    // Skip unknown characters and try again
    advance();
    return nextToken();
}

Token Lexer::processALTXContent() {
    skipWhitespace();
    
    if (eof()) {
        exitState();
        return Token(TokenType::EOFToken, "", line, column);
    }
    
    char c = peek();
    
    // Handle nested tags
    if (c == '<') {
        if (peekAdvance() == '/') {
            // Closing tag - exit content state
            exitState();
            return processTagEnd();
        } else {
            // Opening tag - stay in content but process the tag
            return processTag();
        }
    }
    
    // Handle expressions
    if (c == '{') {
        advance();
        enterState(LexerState::Expression);
        return Token(TokenType::ExpressionStart, "{", line, column);
    }
    
    // Handle text content
    if (isAlpha(c) || isDigit(c)) {
        return processTextContent();
    }
    
    // Skip unknown characters
    advance();
    return nextToken();
}

Token Lexer::processExpression() {
    skipWhitespace();
    
    if (eof()) {
        exitState();
        return Token(TokenType::EOFToken, "", line, column);
    }
    
    char c = peek();
    size_t startLine = line, startColumn = column;
    
    // End of expression
    if (c == '}') {
        advance();
        exitState();
        return Token(TokenType::ExpressionEnd, "}", startLine, startColumn);
    }
    
    // Regular token processing within expressions
    if (isDigit(c)) {
        return processNumber();
    }
    
    if (isAlpha(c)) {
        return processIdentifierOrKeyword();
    }
    
    if (c == '"' || c == '\'') {
        return processString();
    }
    
    if (c == '!') {
        return processValueBinding();
    }
    
    if (isOperatorStartChar(c)) {
        return processOperator();
    }
    
    // Unknown character in expression
    return Token(TokenType::Unknown, std::string(1, advance()), startLine, startColumn);
}

Token Lexer::processStyleValue() {
    // TODO: Implement style value processing
    exitState();
    return nextToken();
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (true) {
        Token token = nextToken();
        tokens.push_back(token);
        
        if (token.type == TokenType::EOFToken) {
            break;
        }
    }
    
    return tokens;
}

std::string Lexer::getTokenTypeName(TokenType type) const {
    switch (type) {
        case TokenType::Identifier: return "Identifier";
        case TokenType::Keyword: return "Keyword";
        case TokenType::Number: return "Number";
        case TokenType::String: return "String";
        case TokenType::Operator: return "Operator";
        case TokenType::Arrow: return "Arrow";
        case TokenType::TagOpen: return "TagOpen";
        case TokenType::TagClose: return "TagClose";
        case TokenType::TagSelfClose: return "TagSelfClose";
        case TokenType::TagEnd: return "TagEnd";
        case TokenType::AttributeName: return "AttributeName";
        case TokenType::AttributeValue: return "AttributeValue";
        case TokenType::Text: return "Text";
        case TokenType::ExpressionStart: return "ExpressionStart";
        case TokenType::ExpressionEnd: return "ExpressionEnd";
        case TokenType::Equals: return "Equals";
        case TokenType::BraceOpen: return "BraceOpen";
        case TokenType::BraceClose: return "BraceClose";
        case TokenType::Colon: return "Colon";
        case TokenType::SemiColon: return "SemiColon";
        case TokenType::ParenOpen: return "ParenOpen";
        case TokenType::ParenClose: return "ParenClose";
        case TokenType::BracketOpen: return "BracketOpen";
        case TokenType::BracketClose: return "BracketClose";
        case TokenType::Comma: return "Comma";
        case TokenType::Dot: return "Dot";
        case TokenType::AtModifier: return "AtModifier";
        case TokenType::ValueBinding: return "ValueBinding";
        case TokenType::StyleProperty: return "StyleProperty";
        case TokenType::EOFToken: return "EOFToken";
        case TokenType::Unknown: return "Unknown";
        case TokenType::Error: return "Error";
        case TokenType::ErrorRecovery: return "ErrorRecovery";
        default: return "Unknown";
    }
}

void Lexer::debugPrintTokens(const std::vector<Token>& tokens) const {
    std::cout << "=== TOKEN STREAM (" << tokens.size() << " tokens) ===" << std::endl;
    for (size_t i = 0; i < tokens.size(); ++i) {
        const Token& token = tokens[i];
        std::cout << std::setw(3) << i << ": " 
                  << std::setw(15) << getTokenTypeName(token.type) 
                  << " '" << token.value << "'";
        
        if (token.error.has_value()) {
            std::cout << " [ERROR: " << *token.error << "]";
        }
        
        std::cout << " (" << token.line << ":" << token.column << ")" << std::endl;
    }
    std::cout << "=== END TOKEN STREAM ===" << std::endl;
}

Token Lexer::createErrorToken(const std::string& lexeme, const std::string& message) {
    return Token(TokenType::Error, lexeme, line, column, message);
}

void Lexer::recoverFromError() {
    // Skip characters until we find a safe recovery point
    while (!eof()) {
        char c = peek();
        if (c == '\n' || c == ';' || c == '}' || c == ')' || c == ']') {
            break;
        }
        advance();
    }
}

Token Lexer::safeNextToken() {
    try {
        return nextToken();
    } catch (const std::exception& e) {
        recoverFromError();
        return Token(TokenType::ErrorRecovery, "", line, column, e.what());
    }
}
