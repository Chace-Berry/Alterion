#pragma once
#include "token.h"
#include <vector>
#include <string>
#include <optional>
#include <cstdint>
#include <stack>

class Lexer {
    // Processes single-line and multi-line comments and emits a Comment token.
    Token processComment();
private:
    std::optional<Token> lastCommentToken;
    
    uint32_t peekCodepoint() const;
    uint32_t peekAdvanceCodepoint() const;
    uint32_t advanceCodepoint();
    std::string input;
    size_t position;
    size_t line;
    size_t column;
    LexerState state;
    std::vector<LexerState> stateStack;
    bool isUTF8Error;

    
    char peek() const;
    char peekAdvance() const;
    char advance();
    bool eof() const;
    bool match(char expected);
    void enterState(LexerState newState);
    void exitState();
    void skipWhitespace();

    
    bool isDigit(char c) const;
    bool isAlpha(char c) const;
    bool isAlphaNumeric(char c) const;
    bool isOperatorStartChar(char c) const;

    
    Token processNumber();
    Token processIdentifierOrKeyword();
    Token processString();
    Token processOperator();
    Token processTag();
    Token processTagEnd();
    Token processTextContent();
    Token processValueBinding();
    Token processStyleProperty();
    Token processALTXAttribute();
    Token processALTXContent();
    Token processExpression();
    Token processStyleValue();
    Token nextToken();

    
    Token createErrorToken(const std::string& lexeme, const std::string& message);
    void recoverFromError();
    Token safeNextToken();

    
    std::string getTokenTypeName(TokenType type) const;
    void debugPrintTokens(const std::vector<Token>& tokens) const;

public:
    explicit Lexer(const std::string& source);
    std::vector<Token> tokenize();
};