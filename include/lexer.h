#pragma once
#include "token.h"
#include <vector>
#include <string>
#include <optional>
#include <cstdint>
#include <stack>

class Lexer {
private:
    bool match(char expected);
    std::string input;
    size_t position;
    size_t line;
    size_t column;
    LexerState state;
    LexerState currentState; // Added for enhanced state tracking
    std::stack<LexerState> stateStack;
    std::string source; // For async block detection and compatibility

    char peek() const;
    char peekNext() const;
    char advance();
    bool eof() const;
    void pushState(LexerState newState);
    void popState();

    Token processIdentifier();
    Token processNormalState();
    Token processNumber();
    Token processString();
    Token processValueBinding();
    Token processALTXAttribute();
    Token processALTXContent();
    Token processExpression();

    bool isDigit(char c) const;
    bool isAlpha(char c) const;
    bool isAlphaNumeric(char c) const;

    // ...existing code...

public:
    explicit Lexer(const std::string& inputText);

    std::vector<Token> tokenize();
    // ...existing code...
};