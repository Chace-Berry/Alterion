#include "../include/lexer.h"
#include <cctype>
#include <iostream>
#include <iomanip>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <cstdint>


namespace {
    // Decodes a UTF-8 sequence starting at 'pos' in 'input'.
    // Returns the Unicode codepoint and number of bytes consumed.
    // If the sequence is malformed, returns U+FFFD and consumes one byte.
    std::pair<uint32_t, size_t> decodeUTF8(const std::string& input, size_t pos) {
        if (pos >= input.size()) return {0, 0};
        unsigned char c = input[pos];
        if (c < 0x80) return {c, 1};
        if ((c & 0xE0) == 0xC0 && pos + 1 < input.size()) {
            unsigned char c2 = input[pos + 1];
            if ((c2 & 0xC0) == 0x80) {
                uint32_t cp = ((c & 0x1F) << 6) | (c2 & 0x3F);
                if (cp >= 0x80) return {cp, 2};
            }
        } else if ((c & 0xF0) == 0xE0 && pos + 2 < input.size()) {
            unsigned char c2 = input[pos + 1];
            unsigned char c3 = input[pos + 2];
            if ((c2 & 0xC0) == 0x80 && (c3 & 0xC0) == 0x80) {
                uint32_t cp = ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
                if (cp >= 0x800) return {cp, 3};
            }
        } else if ((c & 0xF8) == 0xF0 && pos + 3 < input.size()) {
            unsigned char c2 = input[pos + 1];
            unsigned char c3 = input[pos + 2];
            unsigned char c4 = input[pos + 3];
            if ((c2 & 0xC0) == 0x80 && (c3 & 0xC0) == 0x80 && (c4 & 0xC0) == 0x80) {
                uint32_t cp = ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
                if (cp >= 0x10000 && cp <= 0x10FFFF) return {cp, 4};
            }
        }
        
        return {0xFFFD, 1};
    }
}


namespace {
    
    const std::unordered_set<std::string> KEYWORDS = {
        
        "async", "component", "import", "extern", "for", "if", "else", "while", 
        "return", "break", "continue", "yield", "await", "throw", "try", "catch", "finally",
        
        
        "render", "script", "function", "fn", "method",
        
        
        "type", "interface", "struct", "enum", "union",
        
        
        "from", "export", "use", "pub", "mod", "namespace",
        
        
        "move", "copy", "ref", "deref", "owned", "borrowed", "shared", "weak",
        
        
        "true", "false", "null", "none", "undefined",
        
        
        "this", "super", "self", "Self", "match", "case", "default",
        "let", "const", "var", "new", "delete", "as", "is", "typeof",
        "print", "println"
    };

    
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



// Returns the next Unicode codepoint without advancing the position.
uint32_t Lexer::peekCodepoint() const {
    auto [cp, len] = decodeUTF8(input, position);
    return cp;
}


// Returns the codepoint after the current one, without advancing.
uint32_t Lexer::peekAdvanceCodepoint() const {
    auto [cp1, len1] = decodeUTF8(input, position);
    auto [cp2, len2] = decodeUTF8(input, position + len1);
    return cp2;
}


// Advances the position by one Unicode codepoint and returns it.
// Sets isUTF8Error if a malformed sequence is found.
uint32_t Lexer::advanceCodepoint() {
    auto [cp, len] = decodeUTF8(input, position);
    position += len;
    if (cp == 0xFFFD) isUTF8Error = true;
    if (cp == '\n') {
        ++line;
        column = 1;
    } else {
        ++column;
    }
    return cp;
}


char Lexer::peek() const {
    return static_cast<char>(peekCodepoint());
}

char Lexer::peekAdvance() const {
    return static_cast<char>(peekAdvanceCodepoint());
    }

char Lexer::advance() {
    return static_cast<char>(advanceCodepoint());
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

// Skips whitespace and comments (single-line and multi-line).
// Advances the lexer position until a non-whitespace, non-comment character is found.
void Lexer::skipWhitespace() {
    while (!eof()) {
        char c = peek();
        unsigned char uc = static_cast<unsigned char>(c);
        if (uc <= 127 && std::isspace(uc)) {
            advance();
        } else if (c == '/' && peekAdvance() == '/') {
            // Single-line comment: skip until end of line
            advance(); 
            advance(); 
            while (!eof() && peek() != '\n') {
                advance();
            }
        } else if (c == '/' && peekAdvance() == '*') {
            // Multi-line comment: skip until closing '*/'
            advance(); 
            advance(); 
            while (!eof()) {
                if (peek() == '*' && peekAdvance() == '/') {
                    advance(); 
                    advance(); 
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
    unsigned char uc = static_cast<unsigned char>(c);
    return (uc >= 'a' && uc <= 'z') || (uc >= 'A' && uc <= 'Z') || uc == '_' || (uc >= 0x80);
    
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

// Lexes a number literal (decimal, hex, binary, float, scientific notation).
Token Lexer::processNumber() {
    size_t startLine = line, startColumn = column;
    std::string number;
    bool hasDecimal = false;
    bool hasExponent = false;
    
    
    if (peek() == '0') {
        number += advance();
        if (peek() == 'x' || peek() == 'X') {
            
            number += advance();
            while (!eof() && (isDigit(peek()) || (peek() >= 'a' && peek() <= 'f') || (peek() >= 'A' && peek() <= 'F'))) {
                number += advance();
            }
        } else if (peek() == 'b' || peek() == 'B') {
            
            number += advance();
            while (!eof() && (peek() == '0' || peek() == '1')) {
                number += advance();
            }
        } else {
            
            while (!eof() && isDigit(peek())) {
                number += advance();
            }
        }
    } else {
        
        while (!eof() && isDigit(peek())) {
            number += advance();
        }
    }
    
    
    if (peek() == '.' && isDigit(peekAdvance())) {
        hasDecimal = true;
        number += advance(); 
        while (!eof() && isDigit(peek())) {
            number += advance();
        }
    }
    
    
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

// Lexes an identifier or keyword. Identifiers may contain Unicode letters and '-'.
Token Lexer::processIdentifierOrKeyword() {
    size_t startLine = line, startColumn = column;
    std::string text;
    
    while (!eof() && (isAlphaNumeric(peek()) || peek() == '-')) {
        text += advance();
    }
    
    
    if (KEYWORDS.count(text)) {
        return Token(TokenType::Keyword, text, startLine, startColumn);
    }
    
    return Token(TokenType::Identifier, text, startLine, startColumn);
}

// Lexes a string literal, handling escape sequences and Unicode.
Token Lexer::processString() {
    size_t startLine = line, startColumn = column;
    char quote = peek(); 
    advance(); 
    
    std::string value;
    
    while (!eof() && peek() != quote) {
        if (peek() == '\\') {
            advance(); 
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
        advance(); 
    }
    
    return Token(TokenType::String, value, startLine, startColumn);
}

// Lexes operators (single and multi-character).
Token Lexer::processOperator() {
    size_t startLine = line, startColumn = column;
    char first = peek();
    char second = peekAdvance();
    
    
    std::string twoChar;
    twoChar += first;
    twoChar += second;
    
    if (MULTI_CHAR_OPERATORS.count(twoChar)) {
        advance(); 
        advance(); 
        return Token(MULTI_CHAR_OPERATORS.at(twoChar), twoChar, startLine, startColumn);
    }
    
    
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

// Lexes an opening tag (e.g., <div>). Handles tag name and enters attribute state.
Token Lexer::processTag() {
    size_t startLine = line, startColumn = column;
    advance(); 
    
    
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

// Lexes a closing tag (e.g., </div>). Handles tag name and exits content state if needed.
Token Lexer::processTagEnd() {
    size_t startLine = line, startColumn = column;
    advance(); 
    
    std::string tagName;
    while (!eof() && (isAlphaNumeric(peek()) || peek() == '-')) {
        tagName += advance();
    }
    
    
    skipWhitespace();
    if (peek() == '>') {
        advance();
    }
    
    if (state == LexerState::ALTXContent) {
        exitState();
    }
    
    return Token(TokenType::TagClose, tagName, startLine, startColumn);
}

// Lexes raw text content between tags or expressions.
Token Lexer::processTextContent() {
    size_t startLine = line, startColumn = column;
    std::string text;
    
    while (!eof()) {
        char c = peek();
        unsigned char uc = static_cast<unsigned char>(c);
        // Only treat ASCII whitespace as delimiter
        if (c == '<' || c == '{' || (uc <= 127 && std::isspace(uc))) {
            break;
        }
        text += advance();
    }
    
    return Token(TokenType::Text, text, startLine, startColumn);
}

// Lexes a value binding (e.g., !identifier).
Token Lexer::processValueBinding() {
    size_t startLine = line, startColumn = column;
    advance(); 
    
    std::string identifier;
    while (!eof() && (isAlphaNumeric(peek()) || peek() == '_')) {
        identifier += advance();
    }
    
    if (identifier.empty()) {
        return Token(TokenType::Error, "!", startLine, startColumn, "Expected identifier after '!'");
    }
    
    return Token(TokenType::ValueBinding, "!" + identifier, startLine, startColumn);
}

// Lexes a style property name inside a style block.
Token Lexer::processStyleProperty() {
    size_t startLine = line, startColumn = column;
    std::string property;
    
    
    while (!eof() && peek() != ':' && peek() != ';' && peek() != '}' && !std::isspace(peek())) {
        property += advance();
    }
    
    return Token(TokenType::StyleProperty, property, startLine, startColumn);
}

// Main tokenization function. Dispatches based on current state and character.
Token Lexer::nextToken() {
    skipWhitespace();
    
    if (eof()) {
        return Token(TokenType::EOFToken, "", line, column);
    }
    
    char c = peek();
    size_t startLine = line, startColumn = column;
    
    
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
    
    
    return Token(TokenType::Unknown, std::string(1, advance()), startLine, startColumn);
}

// Lexes attributes inside a tag (e.g., <div attr="value">).
Token Lexer::processALTXAttribute() {
    skipWhitespace();
    
    if (eof()) {
        exitState();
        return Token(TokenType::EOFToken, "", line, column);
    }
    
    char c = peek();
    size_t startLine = line, startColumn = column;
    
    
    if (c == '>') {
        advance();
        state = LexerState::ALTXContent; 
        return Token(TokenType::TagEnd, ">", startLine, startColumn);
    }
    
    if (c == '/' && peekAdvance() == '>') {
        advance(); 
        advance(); 
        exitState();
        return Token(TokenType::TagSelfClose, "/>", startLine, startColumn);
    }
    
    
    if (c == '{') {
        advance();
        enterState(LexerState::Expression);
        return Token(TokenType::ExpressionStart, "{", startLine, startColumn);
    }
    
    
    if (isAlpha(c)) {
        std::string attrName;
        while (!eof() && (isAlphaNumeric(peek()) || peek() == '-' || peek() == '_')) {
            attrName += advance();
        }
        if (KEYWORDS.count(attrName)) {
            return Token(TokenType::Keyword, attrName, startLine, startColumn);
        }
        return Token(TokenType::AttributeName, attrName, startLine, startColumn);
    }
    
    
    if (c == '=') {
        advance();
        return Token(TokenType::Equals, "=", startLine, startColumn);
    }
    
    if (c == '"' || c == '\'') {
        return processString();
    }
    
    
    advance();
    return nextToken();
}

// Lexes content inside a tag (e.g., <div> ... </div>).
Token Lexer::processALTXContent() {
    skipWhitespace();
    
    if (eof()) {
        exitState();
        return Token(TokenType::EOFToken, "", line, column);
    }
    
    char c = peek();
    
    
    if (c == '<') {
        if (peekAdvance() == '/') {
            
            exitState();
            return processTagEnd();
        } else {
            
            return processTag();
        }
    }
    
    
    if (c == '{') {
        advance();
        enterState(LexerState::Expression);
        return Token(TokenType::ExpressionStart, "{", line, column);
    }
    
    
    if (isAlpha(c) || isDigit(c)) {
        return processTextContent();
    }
    
    
    advance();
    return nextToken();
}

// Lexes an embedded expression (e.g., { ... }). Handles nested tokens until '}'.
Token Lexer::processExpression() {
    skipWhitespace();
    
    if (eof()) {
        exitState();
        return Token(TokenType::EOFToken, "", line, column);
    }
    
    char c = peek();
    size_t startLine = line, startColumn = column;
    
    
    if (c == '}') {
        advance();
        exitState();
        return Token(TokenType::ExpressionEnd, "}", startLine, startColumn);
    }
    
    
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
    
    
    return Token(TokenType::Unknown, std::string(1, advance()), startLine, startColumn);
}

// Lexes a style value (not yet implemented).
Token Lexer::processStyleValue() {
    
    exitState();
    return nextToken();
}

// Tokenizes the entire input and returns a vector of tokens.
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

// Prints the token stream for debugging purposes.
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

// Creates an error token with a message.
Token Lexer::createErrorToken(const std::string& lexeme, const std::string& message) {
    return Token(TokenType::Error, lexeme, line, column, message);
}

// Skips characters until a safe recovery point (delimiter) is found after an error.
void Lexer::recoverFromError() {
    
    while (!eof()) {
        char c = peek();
        if (c == '\n' || c == ';' || c == '}' || c == ')' || c == ']') {
            break;
        }
        advance();
    }
}

// Calls nextToken() and recovers from exceptions, returning an error recovery token if needed.
Token Lexer::safeNextToken() {
    try {
        return nextToken();
    } catch (const std::exception& e) {
        recoverFromError();
        return Token(TokenType::ErrorRecovery, "", line, column, e.what());
    }
}
