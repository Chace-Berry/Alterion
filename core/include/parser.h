#pragma once
#include "ast_complete.h"
#include "token.h"
#include <vector>
#include <string>
#include <memory>
#include <initializer_list>
#include <stdexcept>

class ParseError : public std::runtime_error {
public:
    StatementPtr parseAsyncBlock();
    size_t line, column;
    std::string message;
    
    ParseError(const std::string& msg, size_t l, size_t c)
        : std::runtime_error(msg), message(msg), line(l), column(c) {}
};

class Parser {
private:
    std::vector<Token> tokens;
    size_t current;
    
    // Token navigation
    Token& peek();
    Token& advance();
    bool isAtEnd();
    bool check(TokenType type);
    bool match(std::initializer_list<TokenType> types);
    bool matchKeyword(const std::string& keyword);
    Token consume(TokenType type, const std::string& message);
    Token consumeKeyword(const std::string& keyword, const std::string& message);
    void synchronize();
    bool checkNext(TokenType type);
    
    // Top-level parsing
    std::unique_ptr<Program> parseProgram();
    ComponentPtr parseComponent();
    FunctionPtr parseFunction();
    StatementPtr parseImport();
    StatementPtr parseExport();
    
    // ALTX/JSX parsing
    std::vector<ASTNodePtr> parseALTXContent();
    TagPtr parseTag();
    std::unique_ptr<Attribute> parseAttribute();
    std::vector<StyleProperty> parseStyleProperty(const std::string& styleContent);
    std::unique_ptr<TextContent> parseTextContent();
    StatementPtr parseEmbeddedExpression();
    
    // Statement parsing
    StatementPtr parseStatement();
    StatementPtr parseBlockStatement();
    StatementPtr parseIfStatement();
    StatementPtr parseWhileStatement();
    StatementPtr parseForStatement();
    StatementPtr parseForInStatement();
    StatementPtr parseReturnStatement();
    StatementPtr parseTryStatement();
    StatementPtr parseThrowStatement();
    StatementPtr parseVariableDeclaration();
    StatementPtr parseAssignment();
    StatementPtr parseMethodDefinition();
    StatementPtr parseModifiedStatement();
    
    // Expression parsing (precedence climbing)
    ExpressionPtr parseExpression();
    ExpressionPtr parseLogicalOr();
    ExpressionPtr parseLogicalAnd();
    ExpressionPtr parseEquality();
    ExpressionPtr parseComparison();
    ExpressionPtr parseTerm();
    ExpressionPtr parseFactor();
    ExpressionPtr parseUnary();
    ExpressionPtr parseCall();
    ExpressionPtr parsePrimary();
    ExpressionPtr parseArrayExpression();
    ExpressionPtr parseObjectExpression();
    
    // Helper methods
    std::vector<std::string> parseParameterList();
    std::vector<std::string> parseImportList();

public:
    explicit Parser(std::vector<Token> tokens);
    
    // Main parsing entry point
    std::unique_ptr<Program> parse();
};