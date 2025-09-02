#include "../include/lexer.h"
#include <cctype>
#include <iostream>
#include <iomanip>
#include <unordered_set>

// Enhanced Alterion Lexer - Complete Implementation
// Supports all syntax from the Alterion Blueprint

const std::unordered_set<std::string> keywords = {
    "async", "component", "import", "extern", "for", "if", "else", "while", 
    "return", "break", "continue", "yield", "await", "throw", "try", "catch", "finally",
    "function", "render", "true", "false", "null", "none", "let", "const", "var"
};

const std::unordered_set<std::string> modifiers = {
    "async", "unsafe", "test", "deprecated", "export", "public", "private"
};

Lexer::Lexer(const std::string& inputText) 
    : input(inputText), position(0), line(1), column(1), state(LexerState::Normal), currentState(LexerState::Normal) {}

char Lexer::advance() {
    if (eof()) return '\0';
    char current = input[position++];
    if (current == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return current;
}

bool Lexer::match(char expected) {
    if (eof() || peek() != expected) return false;
    advance();
    return true;
}

char Lexer::peek() const {
    return eof() ? '\0' : input[position];
}

char Lexer::peekNext() const {
    return (position + 1 >= input.length()) ? '\0' : input[position + 1];
}

bool Lexer::eof() const {
    return position >= input.length();
}

void Lexer::pushState(LexerState newState) {
    stateStack.push(state);
    state = newState;
}

void Lexer::popState() {
    if (!stateStack.empty()) {
        state = stateStack.top();
        stateStack.pop();
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (!eof()) {
        char c = peek();
        
        // Skip whitespace
        if (std::isspace(c)) {
            advance();
            continue;
        }
        
        // Comments
        if (c == '/' && peekNext() == '/') {
            // Single line comment
            advance(); advance(); // Skip //
            while (!eof() && peek() != '\n') {
                advance();
            }
            continue;
        }
        
        if (c == '/' && peekNext() == '*') {
            // Multi-line comment
            advance(); advance(); // Skip /*
            while (!eof()) {
                if (peek() == '*' && peekNext() == '/') {
                    advance(); advance(); // Skip */
                    break;
                }
                advance();
            }
            continue;
        }
        
        // Numbers
        if (std::isdigit(c)) {
            tokens.push_back(processNumber());
            continue;
        }
        
        // Strings
        if (c == '"' || c == '\'') {
            tokens.push_back(processString());
            continue;
        }
        
        // Identifiers and keywords
        if (std::isalpha(c) || c == '_') {
            tokens.push_back(processIdentifier());
            continue;
        }
        
        // Process based on current state
switch (state) {
    case LexerState::Normal:
        tokens.push_back(processNormalState());
        break;
    case LexerState::Expression:
        tokens.push_back(processExpression());
        break;
    case LexerState::ALTXAttribute:
        tokens.push_back(processALTXAttribute());
        break;
    case LexerState::ALTXContent:
        tokens.push_back(processALTXContent());
        break;
    default:
        tokens.push_back(processNormalState());
        break;
        }
    }
    
    tokens.emplace_back(TokenType::EOFToken, "", line, column);
    return tokens;
}

Token Lexer::processNumber() {
    size_t startLine = line, startColumn = column;
    std::string value;
    
    // Handle different number formats: decimal, hex, binary
    if (peek() == '0') {
        value += advance();
        if (peek() == 'x' || peek() == 'X') {
            // Hexadecimal
            value += advance();
            while (!eof() && (std::isdigit(peek()) || 
                   (peek() >= 'a' && peek() <= 'f') || 
                   (peek() >= 'A' && peek() <= 'F'))) {
                value += advance();
            }
        } else if (peek() == 'b' || peek() == 'B') {
            // Binary
            value += advance();
            while (!eof() && (peek() == '0' || peek() == '1')) {
                value += advance();
            }
        } else {
            // Octal or decimal starting with 0
            while (!eof() && std::isdigit(peek())) {
                value += advance();
            }
        }
    } else {
        // Decimal number
        while (!eof() && std::isdigit(peek())) {
            value += advance();
        }
    }
    
    // Handle decimal point
    if (!eof() && peek() == '.' && std::isdigit(peekNext())) {
        value += advance(); // consume '.'
        while (!eof() && std::isdigit(peek())) {
            value += advance();
        }
    }
    
    return Token(TokenType::Number, value, startLine, startColumn);
}

Token Lexer::processString() {
    size_t startLine = line, startColumn = column;
    char quote = advance(); // consume opening quote
    std::string value;
    
    while (!eof() && peek() != quote) {
        if (peek() == '\\') {
            advance(); // consume backslash
            if (!eof()) {
                char escaped = advance();
                switch (escaped) {
                    case 'n': value += '\n'; break;
                    case 't': value += '\t'; break;
                    case 'r': value += '\r'; break;
                    case '\\': value += '\\'; break;
                    case '"': value += '"'; break;
                    case '\'': value += '\''; break;
                    default: value += escaped; break;
                }
            }
        } else {
            value += advance();
        }
    }
    
    if (!eof()) {
        advance(); // consume closing quote
    }
    
    return Token(TokenType::String, value, startLine, startColumn);
}

Token Lexer::processIdentifier() {
    size_t startLine = line, startColumn = column;
    std::string value;
    
    while (!eof() && (std::isalnum(peek()) || peek() == '_')) {
        value += advance();
    }
    
    // Check for keywords
    if (keywords.count(value)) {
        if (value == "true" || value == "false") {
            return Token(TokenType::Boolean, value, startLine, startColumn);
        } else if (value == "null" || value == "none") {
            return Token(TokenType::Null, value, startLine, startColumn);
        } else {
            return Token(TokenType::Keyword, value, startLine, startColumn);
        }
    }
    
    return Token(TokenType::Identifier, value, startLine, startColumn);
}

Token Lexer::processNormalState() {
    size_t startLine = line, startColumn = column;
    char c = advance();
    
    switch (c) {
        // Async block detection
        case 'a':
            if (position >= 5 && source.substr(position-5, 5) == "async") {
                if (peek() == '{') {
                    advance(); // consume '{'
                    return Token(TokenType::AsyncBlockStart, "async{", startLine, startColumn);
                }
            }
            // Fall back to identifier processing
            position--; column--;
            return processIdentifier();
            
        // Modifiers
        case '@':
            return Token(TokenType::AtModifier, "@", startLine, startColumn);
            
        // Operators
        case '+':
            if (match('=')) return Token(TokenType::PlusAssign, "+=", startLine, startColumn);
            return Token(TokenType::Plus, "+", startLine, startColumn);
            
        case '-':
            if (match('=')) return Token(TokenType::MinusAssign, "-=", startLine, startColumn);
            return Token(TokenType::Minus, "-", startLine, startColumn);
            
        case '*':
            if (match('*')) return Token(TokenType::Power, "**", startLine, startColumn);
            if (match('=')) return Token(TokenType::MultiplyAssign, "*=", startLine, startColumn);
            return Token(TokenType::Multiply, "*", startLine, startColumn);
            
        case '/':
            if (match('=')) return Token(TokenType::DivideAssign, "/=", startLine, startColumn);
            return Token(TokenType::Divide, "/", startLine, startColumn);
            
        case '%':
            return Token(TokenType::Modulo, "%", startLine, startColumn);
            
        case '=':
            if (match('=')) return Token(TokenType::Equal, "==", startLine, startColumn);
            return Token(TokenType::Assign, "=", startLine, startColumn);
            
        case '!':
            if (match('=')) return Token(TokenType::NotEqual, "!=", startLine, startColumn);
            return Token(TokenType::LogicalNot, "!", startLine, startColumn);
            
        case '<':
            if (match('=')) return Token(TokenType::LessEqual, "<=", startLine, startColumn);
            if (peek() == '/' || std::isalpha(peek())) {
                // Might be a tag
                pushState(LexerState::ALTXAttribute);
                return Token(TokenType::TagOpen, "<", startLine, startColumn);
            }
            return Token(TokenType::Less, "<", startLine, startColumn);
            
        case '>':
            if (match('=')) return Token(TokenType::GreaterEqual, ">=", startLine, startColumn);
            return Token(TokenType::Greater, ">", startLine, startColumn);
            
        case '&':
            if (match('&')) return Token(TokenType::LogicalAnd, "&&", startLine, startColumn);
            return Token(TokenType::Unknown, "&", startLine, startColumn);
            
        case '|':
            if (match('|')) return Token(TokenType::LogicalOr, "||", startLine, startColumn);
            return Token(TokenType::Unknown, "|", startLine, startColumn);
            
        // Braces and brackets
        case '{':
            if (position > 1 && source[position-2] == '!') {
                pushState(LexerState::Expression);
                return Token(TokenType::ExpressionStart, "{", startLine, startColumn);
            } else if (position > 0 && std::isalnum(source[position-2])) {
                pushState(LexerState::Expression);
                return Token(TokenType::ExpressionStart, "{", startLine, startColumn);
            }
            return Token(TokenType::BraceOpen, "{", startLine, startColumn);
            
        case '}':
            if (currentState == LexerState::Expression) {
                popState();
                return Token(TokenType::ExpressionEnd, "}", startLine, startColumn);
            }
            return Token(TokenType::BraceClose, "}", startLine, startColumn);
            
        case '[':
            return Token(TokenType::BracketOpen, "[", startLine, startColumn);
            
        case ']':
            if (peek() == '[') {
                advance(); // consume second '['
                return Token(TokenType::AsyncCatchStart, "][", startLine, startColumn);
            }
            return Token(TokenType::BracketClose, "]", startLine, startColumn);
            
        case '(':
            return Token(TokenType::ParenOpen, "(", startLine, startColumn);
            
        case ')':
            return Token(TokenType::ParenClose, ")", startLine, startColumn);
            
        // Punctuation
        case ':':
            return Token(TokenType::Colon, ":", startLine, startColumn);
            
        case ';':
            return Token(TokenType::SemiColon, ";", startLine, startColumn);
            
        case ',':
            return Token(TokenType::Comma, ",", startLine, startColumn);
            
        case '.':
            return Token(TokenType::Dot, ".", startLine, startColumn);
            
        default:
            return Token(TokenType::Unknown, std::string(1, c), startLine, startColumn);
    }
}

Token Lexer::processExpression() {
    size_t startLine = line, startColumn = column;
    char c = advance();
    
    // Handle value binding syntax {!name}
    if (c == '!') {
        return processValueBinding();
    }
    
    // Regular expression processing
    if (std::isalpha(c) || c == '_') {
        position--; column--; // Back up
        return processIdentifier();
    }
    
    // Handle expression end
    if (c == '}') {
        popState();
        return Token(TokenType::ExpressionEnd, "}", startLine, startColumn);
    }
    
    // Fall back to normal processing
    position--; column--; // Back up
    return processNormalState();
}

Token Lexer::processValueBinding() {
    size_t startLine = line, startColumn = column;
    std::string name;
    
    // Process the identifier after !
    while (!eof() && (std::isalnum(peek()) || peek() == '_')) {
        name += advance();
    }
    
    return Token(TokenType::ValueBinding, name, startLine, startColumn);
}

Token Lexer::processALTXAttribute() {
    size_t startLine = line, startColumn = column;
    char c = advance();
    
    // Handle tag closing
    if (c == '>') {
        popState();
        pushState(LexerState::ALTXContent);
        return Token(TokenType::TagClose, ">", startLine, startColumn);
    }
    
    if (c == '/') {
        if (match('>')) {
            popState();
            return Token(TokenType::TagSelfClose, "/>", startLine, startColumn);
        }
    }
    
    // Handle attributes
    if (std::isalpha(c) || c == '_') {
        position--; column--; // Back up
        return processIdentifier();
    }
    
    // Fall back to normal processing
    position--; column--; // Back up
    return processNormalState();
}

Token Lexer::processALTXContent() {
    size_t startLine = line, startColumn = column;
    
    // Look for tag end or expression start
    if (peek() == '<') {
        if (peekNext() == '/') {
            popState(); // Exit ALTX content
            advance(); // consume '<'
            advance(); // consume '/'
            return Token(TokenType::TagEnd, "</", startLine, startColumn);
        } else if (std::isalpha(peekNext())) {
            // New tag starting
            pushState(LexerState::ALTXAttribute);
            advance(); // consume '<'
            return Token(TokenType::TagOpen, "<", startLine, startColumn);
        }
    }
    
    if (peek() == '{') {
        pushState(LexerState::Expression);
        advance(); // consume '{'
        return Token(TokenType::ExpressionStart, "{", startLine, startColumn);
    }
    
    // Read text content
    std::string text;
    while (!eof() && peek() != '<' && peek() != '{') {
        text += advance();
    }
    
    if (!text.empty()) {
        return Token(TokenType::Text, text, startLine, startColumn);
    }
    
    // Fall back
    return processNormalState();
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
