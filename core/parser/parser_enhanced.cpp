#include "../include/parser.h"
#include "../include/ast_complete.h"
#include "../include/token.h"
#include <algorithm>
#include <cctype>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <sstream>

// Phase 2B: Enhanced Alterion Parser with Async Blocks and Control Flow

Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)), current(0) {}

// Token navigation methods
Token& Parser::peek() {
    if (current >= tokens.size()) {
        static Token eofToken(TokenType::EOFToken, "", 0, 0);
        return eofToken;
    }
    return tokens[current];
}

Token& Parser::advance() {
    if (!isAtEnd()) current++;
    return tokens[current - 1];
}

bool Parser::isAtEnd() {
    return current >= tokens.size() || peek().type == TokenType::EOFToken;
}

bool Parser::check(TokenType type) {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

bool Parser::matchKeyword(const std::string& keyword) {
    return check(TokenType::Keyword) && peek().value == keyword;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    
    Token& currentToken = peek();
    throw ParseError(message + ", got '" + currentToken.value + "'", currentToken.line, currentToken.column);
}

Token Parser::consumeKeyword(const std::string& keyword, const std::string& message) {
    if (matchKeyword(keyword)) return advance();
    
    Token& currentToken = peek();
    throw ParseError(message + ", got '" + currentToken.value + "'", currentToken.line, currentToken.column);
}

bool Parser::checkNext(TokenType type) {
    if (current + 1 >= tokens.size()) return false;
    return tokens[current + 1].type == type;
}

void Parser::synchronize() {
    advance();
    
    while (!isAtEnd()) {
        if (tokens[current - 1].type == TokenType::SemiColon) return;
        
        switch (peek().type) {
            case TokenType::Keyword:
                if (peek().value == "component" || peek().value == "import" || 
                    peek().value == "function" || peek().value == "if" || 
                    peek().value == "while" || peek().value == "for" || 
                    peek().value == "return" || peek().value == "try") {
                    return;
                }
                break;
            case TokenType::BraceClose:
            case TokenType::ParenClose:
            case TokenType::BracketClose:
                return;
            default:
                break;
        }
        advance();
    }
}

// Main parsing method
std::unique_ptr<Program> Parser::parse() {
    try {
        return parseProgram();
    } catch (const ParseError& error) {
        std::cerr << "Parse Error at " << error.line << ":" << error.column << " - " << error.message << std::endl;
        throw;
    }
}

// Top-level parsing
std::unique_ptr<Program> Parser::parseProgram() {
    auto program = std::make_unique<Program>();
    
    while (!isAtEnd()) {
        try {
            if (matchKeyword("import")) {
                program->globalStatements.push_back(parseImport());
            } else if (matchKeyword("export")) {
                program->globalStatements.push_back(parseExport());
            } else if (matchKeyword("component")) {
                program->components.push_back(parseComponent());
            } else if (matchKeyword("function") || check(TokenType::AtModifier)) {
                program->functions.push_back(parseFunction());
            } else {
                program->globalStatements.push_back(parseStatement());
            }
        } catch (const ParseError& error) {
            std::cerr << "Skipping invalid statement: " << error.message << std::endl;
            synchronize();
        }
    }
    
    return program;
}

// Component parsing with enhanced syntax
ComponentPtr Parser::parseComponent() {
    // Parse modifiers (@async, @deprecated, etc.)
    std::vector<std::string> modifiers;
    while (check(TokenType::AtModifier)) {
        modifiers.push_back(advance().value);
    }
    
    consumeKeyword("component", "Expected 'component'");
    Token nameToken = consume(TokenType::Identifier, "Expected component name");
    
    consume(TokenType::BraceOpen, "Expected '{' after component name");
    
    auto component = std::make_unique<Component>(nameToken.value);
    component->modifiers = std::move(modifiers);
    
    // Parse component body: state declarations, methods, ALTX content
    while (!check(TokenType::BraceClose) && !isAtEnd()) {
        try {
            if (check(TokenType::Identifier) && checkNext(TokenType::Assign)) {
                // State variable assignment: name = value
                component->statements.push_back(parseAssignment());
            } else if (check(TokenType::Identifier) && checkNext(TokenType::BraceOpen)) {
                // Method definition: methodName { ... }
                component->statements.push_back(parseMethodDefinition());
            } else if (check(TokenType::Less)) {
                // ALTX content: <div>...</div>
                auto altxNodes = parseALTXContent();
                component->body.insert(component->body.end(), altxNodes.begin(), altxNodes.end());
            } else if (matchKeyword("render")) {
                // Render method: render { ... }
                consume(TokenType::BraceOpen, "Expected '{' after 'render'");
                auto altxNodes = parseALTXContent();
                component->body.insert(component->body.end(), altxNodes.begin(), altxNodes.end());
                consume(TokenType::BraceClose, "Expected '}' after render body");
            } else {
                // Regular statement
                component->statements.push_back(parseStatement());
            }
        } catch (const ParseError& error) {
            std::cerr << "Error in component: " << error.message << std::endl;
            synchronize();
        }
    }
    
    consume(TokenType::BraceClose, "Expected '}' after component body");
    return component;
}

// Function parsing with modifiers
FunctionPtr Parser::parseFunction() {
    std::vector<std::string> modifiers;
    
    // Parse modifiers (@async, @pure, @test, etc.)
    while (check(TokenType::AtModifier)) {
        modifiers.push_back(advance().value);
    }
    
    consumeKeyword("function", "Expected 'function'");
    Token nameToken = consume(TokenType::Identifier, "Expected function name");
    
    consume(TokenType::ParenOpen, "Expected '(' after function name");
    auto parameters = parseParameterList();
    consume(TokenType::ParenClose, "Expected ')' after parameters");
    
    // Optional return type annotation
    std::optional<std::string> returnType;
    if (match({TokenType::Colon})) {
        returnType = consume(TokenType::Identifier, "Expected return type").value;
    }
    
    auto body = std::dynamic_pointer_cast<BlockStatement>(parseBlockStatement());
    
    auto function = std::make_unique<Function>(nameToken.value, parameters, body);
    function->modifiers = std::move(modifiers);
    function->returnType = returnType;
    
    return function;
}

// Enhanced statement parsing with async blocks and control flow
StatementPtr Parser::parseStatement() {
    // Async block: async{[try][catch][finally]}
    if (check(TokenType::AsyncBlockStart)) {
        return parseAsyncBlock();
    }
    
    // Control flow statements
    if (matchKeyword("if")) {
        return parseIfStatement();
    }
    if (matchKeyword("for")) {
        return parseForStatement();
    }
    if (matchKeyword("while")) {
        return parseWhileStatement();
    }
    if (matchKeyword("return")) {
        return parseReturnStatement();
    }
    if (matchKeyword("try")) {
        return parseTryStatement();
    }
    if (matchKeyword("throw")) {
        return parseThrowStatement();
    }
    if (matchKeyword("break")) {
        advance();
        consume(TokenType::SemiColon, "Expected ';' after 'break'");
        return std::make_unique<BreakStatement>();
    }
    if (matchKeyword("continue")) {
        advance();
        consume(TokenType::SemiColon, "Expected ';' after 'continue'");
        return std::make_unique<ContinueStatement>();
    }
    
    // Variable declarations
    if (matchKeyword("let") || matchKeyword("const") || matchKeyword("var")) {
        return parseVariableDeclaration();
    }
    
    // Block statement
    if (check(TokenType::BraceOpen)) {
        return parseBlockStatement();
    }
    
    // Assignment or expression statement
    if (check(TokenType::Identifier) && checkNext(TokenType::Assign)) {
        return parseAssignment();
    }
    
    // Expression statement
    auto expr = parseExpression();
    consume(TokenType::SemiColon, "Expected ';' after expression");
    return std::make_unique<ExpressionStatement>(std::move(expr));
}

// NEW: Async block parsing - async{[try][catch][finally]}
StatementPtr Parser::parseAsyncBlock() {
    advance(); // consume 'async{'
    
    // Parse try block (required)
    consume(TokenType::BracketOpen, "Expected '[' for try block");
    auto tryBlock = std::static_pointer_cast<BlockStatement>(parseBlockStatement());
    consume(TokenType::BracketClose, "Expected ']' after try block");
    
    // Parse optional catch block
    std::shared_ptr<BlockStatement> catchBlock = nullptr;
    std::string catchVariable;
    if (check(TokenType::BracketOpen)) {
        advance(); // consume '['
        if (matchKeyword("catch")) {
            if (match({TokenType::ParenOpen})) {
                catchVariable = consume(TokenType::Identifier, "Expected catch variable").value;
                consume(TokenType::ParenClose, "Expected ')' after catch variable");
            }
            catchBlock = std::static_pointer_cast<BlockStatement>(parseBlockStatement());
        }
        consume(TokenType::BracketClose, "Expected ']' after catch block");
    }
    
    // Parse optional finally block
    std::shared_ptr<BlockStatement> finallyBlock = nullptr;
    if (check(TokenType::BracketOpen)) {
        advance(); // consume '['
        if (matchKeyword("finally")) {
            finallyBlock = std::static_pointer_cast<BlockStatement>(parseBlockStatement());
        }
        consume(TokenType::BracketClose, "Expected ']' after finally block");
    }
    
    consume(TokenType::BraceClose, "Expected '}' after async block");
    
    return std::make_unique<AsyncBlock>(tryBlock, catchBlock, finallyBlock);
}

// Enhanced for statement parsing - supports for i (6)[] and for name in names[]
StatementPtr Parser::parseForStatement() {
    consumeKeyword("for", "Expected 'for'");
    
    // Check for for-in pattern: for name in names[]
    if (check(TokenType::Identifier) && 
        (current + 2 < tokens.size() && tokens[current + 1].value == "in")) {
        
        std::string variable = advance().value;
        consumeKeyword("in", "Expected 'in'");
        auto iterable = parseExpression();
        consume(TokenType::BracketOpen, "Expected '[' for for-in body");
        auto body = parseBlockStatement();
        consume(TokenType::BracketClose, "Expected ']' after for-in body");
        
        return std::make_unique<ForInStatement>(variable, std::move(iterable), 
                                              std::static_pointer_cast<BlockStatement>(body));
    }
    
    // Standard for loop or for i (count)[] pattern
    Token variable = consume(TokenType::Identifier, "Expected loop variable");
    consume(TokenType::ParenOpen, "Expected '(' in for loop");
    
    // Check for range pattern: for i (6) or for i (start, end)
    if (check(TokenType::Number)) {
        Token countToken = advance();
        
        // Check for range: (start, end)
        if (match({TokenType::Comma})) {
            Token endToken = consume(TokenType::Number, "Expected end number");
            consume(TokenType::ParenClose, "Expected ')' after range");
            consume(TokenType::BracketOpen, "Expected '[' for loop body");
            auto body = parseBlockStatement();
            consume(TokenType::BracketClose, "Expected ']' after loop body");
            
            // Create range for loop
            auto startExpr = std::make_unique<NumberLiteral>(countToken.value);
            auto endExpr = std::make_unique<NumberLiteral>(endToken.value);
            auto init = std::make_unique<VariableDeclaration>("let", variable.value, std::move(startExpr));
            auto test = std::make_unique<BinaryExpression>(
                std::make_unique<Identifier>(variable.value),
                "<",
                std::move(endExpr)
            );
            auto update = std::make_unique<Assignment>(variable.value, "+=", 
                                                     std::make_unique<NumberLiteral>("1"));
            
            return std::make_unique<ForStatement>(std::move(init), std::move(test), 
                                                std::move(update), 
                                                std::static_pointer_cast<BlockStatement>(body));
        } else {
            // Simple count loop: for i (6)
            consume(TokenType::ParenClose, "Expected ')' after count");
            consume(TokenType::BracketOpen, "Expected '[' for loop body");
            auto body = parseBlockStatement();
            consume(TokenType::BracketClose, "Expected ']' after loop body");
            
            // Create count for loop: for(let i = 0; i < count; i++)
            auto init = std::make_unique<VariableDeclaration>("let", variable.value, 
                                                            std::make_unique<NumberLiteral>("0"));
            auto test = std::make_unique<BinaryExpression>(
                std::make_unique<Identifier>(variable.value),
                "<",
                std::make_unique<NumberLiteral>(countToken.value)
            );
            auto update = std::make_unique<Assignment>(variable.value, "+=", 
                                                     std::make_unique<NumberLiteral>("1"));
            
            return std::make_unique<ForStatement>(std::move(init), std::move(test), 
                                                std::move(update), 
                                                std::static_pointer_cast<BlockStatement>(body));
        }
    }
    
    // Standard C-style for loop
    StatementPtr init = nullptr;
    if (!check(TokenType::SemiColon)) {
        if (matchKeyword("let") || matchKeyword("const") || matchKeyword("var")) {
            init = parseVariableDeclaration();
        } else {
            init = std::make_unique<ExpressionStatement>(parseExpression());
        }
    }
    consume(TokenType::SemiColon, "Expected ';' after for loop initializer");
    
    ExpressionPtr test = nullptr;
    if (!check(TokenType::SemiColon)) {
        test = parseExpression();
    }
    consume(TokenType::SemiColon, "Expected ';' after for loop condition");
    
    StatementPtr update = nullptr;
    if (!check(TokenType::ParenClose)) {
        update = std::make_unique<ExpressionStatement>(parseExpression());
    }
    consume(TokenType::ParenClose, "Expected ')' after for loop");
    
    auto body = std::static_pointer_cast<BlockStatement>(parseStatement());
    
    return std::make_unique<ForStatement>(std::move(init), std::move(test), 
                                        std::move(update), body);
}

// Enhanced if statement parsing - supports if (cond)[]
StatementPtr Parser::parseIfStatement() {
    consumeKeyword("if", "Expected 'if'");
    consume(TokenType::ParenOpen, "Expected '(' after 'if'");
    auto test = parseExpression();
    consume(TokenType::ParenClose, "Expected ')' after if condition");
    
    // Support both {} and [] syntax for if bodies
    StatementPtr consequent;
    if (check(TokenType::BracketOpen)) {
        advance(); // consume '['
        consequent = parseBlockStatement();
        consume(TokenType::BracketClose, "Expected ']' after if body");
    } else {
        consequent = parseStatement();
    }
    
    StatementPtr alternate = nullptr;
    if (matchKeyword("else")) {
        if (check(TokenType::BracketOpen)) {
            advance(); // consume '['
            alternate = parseBlockStatement();
            consume(TokenType::BracketClose, "Expected ']' after else body");
        } else {
            alternate = parseStatement();
        }
    }
    
    return std::make_unique<IfStatement>(std::move(test), std::move(consequent), std::move(alternate));
}


// Variable declaration parsing
StatementPtr Parser::parseVariableDeclaration() {
    std::string kind;
    if (matchKeyword("let")) kind = "let";
    else if (matchKeyword("const")) kind = "const";
    else if (matchKeyword("var")) kind = "var";
    else throw ParseError("Expected variable declaration keyword", peek().line, peek().column);
    
    Token nameToken = consume(TokenType::Identifier, "Expected variable name");
    
    // Optional type annotation
    std::optional<std::string> typeAnnotation;
    if (match({TokenType::Colon})) {
        typeAnnotation = consume(TokenType::Identifier, "Expected type").value;
    }
    
    ExpressionPtr initializer = nullptr;
    if (match({TokenType::Assign})) {
        initializer = parseExpression();
    }
    
    consume(TokenType::SemiColon, "Expected ';' after variable declaration");
    
    auto varDecl = std::make_unique<VariableDeclaration>(kind, nameToken.value, std::move(initializer));
    varDecl->typeAnnotation = typeAnnotation;
    return varDecl;
}

// Assignment parsing with enhanced operators
StatementPtr Parser::parseAssignment() {
    Token nameToken = consume(TokenType::Identifier, "Expected variable name");
    
    std::string operator_;
    if (match({TokenType::Assign})) operator_ = "=";
    else if (match({TokenType::PlusAssign})) operator_ = "+=";
    else if (match({TokenType::MinusAssign})) operator_ = "-=";
    else if (match({TokenType::MultiplyAssign})) operator_ = "*=";
    else if (match({TokenType::DivideAssign})) operator_ = "/=";
    else if (match({TokenType::PowerAssign})) operator_ = "**=";
    else throw ParseError("Expected assignment operator", peek().line, peek().column);
    
    auto value = parseExpression();
    consume(TokenType::SemiColon, "Expected ';' after assignment");
    
    return std::make_unique<Assignment>(nameToken.value, operator_, std::move(value));
}

// Method definition parsing (for components)
StatementPtr Parser::parseMethodDefinition() {
    Token nameToken = consume(TokenType::Identifier, "Expected method name");
    consume(TokenType::BraceOpen, "Expected '{' after method name");
    
    std::vector<StatementPtr> statements;
    while (!check(TokenType::BraceClose) && !isAtEnd()) {
        statements.push_back(parseStatement());
    }
    
    consume(TokenType::BraceClose, "Expected '}' after method body");
    
    auto body = std::make_unique<BlockStatement>();
    body->statements = std::move(statements);
    
    std::vector<std::string> parameters; // Empty for component methods
    auto function = std::make_unique<Function>(nameToken.value, parameters, body);
    
    return function;
}

// Block statement parsing
StatementPtr Parser::parseBlockStatement() {
    consume(TokenType::BraceOpen, "Expected '{'");
    
    auto block = std::make_unique<BlockStatement>();
    
    while (!check(TokenType::BraceClose) && !isAtEnd()) {
        try {
            block->statements.push_back(parseStatement());
        } catch (const ParseError& error) {
            std::cerr << "Error in block: " << error.message << std::endl;
            synchronize();
        }
    }
    
    consume(TokenType::BraceClose, "Expected '}'");
    return block;
}

// Return statement parsing
StatementPtr Parser::parseReturnStatement() {
    consumeKeyword("return", "Expected 'return'");
    
    ExpressionPtr argument = nullptr;
    if (!check(TokenType::SemiColon)) {
        argument = parseExpression();
    }
    
    consume(TokenType::SemiColon, "Expected ';' after return statement");
    return std::make_unique<ReturnStatement>(std::move(argument));
}

// Try statement parsing
StatementPtr Parser::parseTryStatement() {
    consumeKeyword("try", "Expected 'try'");
    auto block = std::static_pointer_cast<BlockStatement>(parseBlockStatement());
    
    std::shared_ptr<BlockStatement> catchBlock = nullptr;
    std::string catchVariable;
    if (matchKeyword("catch")) {
        if (match({TokenType::ParenOpen})) {
            catchVariable = consume(TokenType::Identifier, "Expected catch variable").value;
            consume(TokenType::ParenClose, "Expected ')' after catch variable");
        }
        catchBlock = std::static_pointer_cast<BlockStatement>(parseBlockStatement());
    }
    
    std::shared_ptr<BlockStatement> finallyBlock = nullptr;
    if (matchKeyword("finally")) {
        finallyBlock = std::static_pointer_cast<BlockStatement>(parseBlockStatement());
    }
    
    return std::make_unique<TryStatement>(block, catchBlock, finallyBlock, catchVariable);
}

// Throw statement parsing
StatementPtr Parser::parseThrowStatement() {
    consumeKeyword("throw", "Expected 'throw'");
    auto argument = parseExpression();
    consume(TokenType::SemiColon, "Expected ';' after throw statement");
    return std::make_unique<ThrowStatement>(std::move(argument));
}

// While statement parsing
StatementPtr Parser::parseWhileStatement() {
    consumeKeyword("while", "Expected 'while'");
    consume(TokenType::ParenOpen, "Expected '(' after 'while'");
    auto test = parseExpression();
    consume(TokenType::ParenClose, "Expected ')' after while condition");
    auto body = std::static_pointer_cast<BlockStatement>(parseStatement());
    
    return std::make_unique<WhileStatement>(std::move(test), body);
}

// Enhanced expression parsing with precedence climbing
ExpressionPtr Parser::parseExpression() {
    return parseLogicalOr();
}

ExpressionPtr Parser::parseLogicalOr() {
    auto expr = parseLogicalAnd();
    
    while (match({TokenType::LogicalOr})) {
        std::string operator_ = tokens[current - 1].value;
        auto right = parseLogicalAnd();
        expr = std::make_unique<BinaryExpression>(std::move(expr), operator_, std::move(right));
    }
    
    return expr;
}

ExpressionPtr Parser::parseLogicalAnd() {
    auto expr = parseEquality();
    
    while (match({TokenType::LogicalAnd})) {
        std::string operator_ = tokens[current - 1].value;
        auto right = parseEquality();
        expr = std::make_unique<BinaryExpression>(std::move(expr), operator_, std::move(right));
    }
    
    return expr;
}

ExpressionPtr Parser::parseEquality() {
    auto expr = parseComparison();
    
    while (match({TokenType::Equal, TokenType::NotEqual})) {
        std::string operator_ = tokens[current - 1].value;
        auto right = parseComparison();
        expr = std::make_unique<BinaryExpression>(std::move(expr), operator_, std::move(right));
    }
    
    return expr;
}

ExpressionPtr Parser::parseComparison() {
    auto expr = parseTerm();
    
    while (match({TokenType::Less, TokenType::Greater, TokenType::LessEqual, TokenType::GreaterEqual})) {
        std::string operator_ = tokens[current - 1].value;
        auto right = parseTerm();
        expr = std::make_unique<BinaryExpression>(std::move(expr), operator_, std::move(right));
    }
    
    return expr;
}

ExpressionPtr Parser::parseTerm() {
    auto expr = parseFactor();
    
    while (match({TokenType::Plus, TokenType::Minus})) {
        std::string operator_ = tokens[current - 1].value;
        auto right = parseFactor();
        expr = std::make_unique<BinaryExpression>(std::move(expr), operator_, std::move(right));
    }
    
    return expr;
}

ExpressionPtr Parser::parseFactor() {
    auto expr = parseUnary();
    
    while (match({TokenType::Multiply, TokenType::Divide, TokenType::Power})) {
        std::string operator_ = tokens[current - 1].value;
        auto right = parseUnary();
        expr = std::make_unique<BinaryExpression>(std::move(expr), operator_, std::move(right));
    }
    
    return expr;
}

ExpressionPtr Parser::parseUnary() {
    if (match({TokenType::LogicalNot, TokenType::Minus})) {
        std::string operator_ = tokens[current - 1].value;
        auto right = parseUnary();
        return std::make_unique<UnaryExpression>(operator_, std::move(right));
    }
    
    return parseCall();
}

ExpressionPtr Parser::parseCall() {
    auto expr = parsePrimary();
    
    while (true) {
        if (match({TokenType::ParenOpen})) {
            // Function call
            std::vector<ExpressionPtr> arguments;
            if (!check(TokenType::ParenClose)) {
                do {
                    arguments.push_back(parseExpression());
                } while (match({TokenType::Comma}));
            }
            consume(TokenType::ParenClose, "Expected ')' after arguments");
            expr = std::make_unique<CallExpression>(std::move(expr), std::move(arguments));
        } else if (match({TokenType::Dot})) {
            // Member access
            Token nameToken = consume(TokenType::Identifier, "Expected property name after '.'");
            auto property = std::make_unique<Identifier>(nameToken.value);
            expr = std::make_unique<MemberExpression>(std::move(expr), std::move(property), false);
        } else if (match({TokenType::BracketOpen})) {
            // Computed member access
            auto property = parseExpression();
            consume(TokenType::BracketClose, "Expected ']' after computed property");
            expr = std::make_unique<MemberExpression>(std::move(expr), std::move(property), true);
        } else {
            break;
        }
    }
    
    return expr;
}

ExpressionPtr Parser::parsePrimary() {
    // Literals
    if (match({TokenType::Boolean})) {
        return std::make_unique<BooleanLiteral>(tokens[current - 1].value == "true");
    }
    
    if (match({TokenType::Number})) {
        return std::make_unique<NumberLiteral>(tokens[current - 1].value);
    }
    
    if (match({TokenType::String})) {
        return std::make_unique<StringLiteral>(tokens[current - 1].value);
    }
    
    if (matchKeyword("null") || matchKeyword("none")) {
        advance();
        return std::make_unique<NullLiteral>();
    }
    
    // Identifier or value binding
    if (match({TokenType::Identifier})) {
        return std::make_unique<Identifier>(tokens[current - 1].value);
    }
    
    // Value binding {!name}
    if (match({TokenType::ValueBinding})) {
        return std::make_unique<ValueBinding>(tokens[current - 1].value);
    }
    
    // Parenthesized expression
    if (match({TokenType::ParenOpen})) {
        auto expr = parseExpression();
        consume(TokenType::ParenClose, "Expected ')' after expression");
        return expr;
    }
    
    // Array literal
    if (check(TokenType::BracketOpen)) {
        return parseArrayExpression();
    }
    
    // Object literal
    if (check(TokenType::BraceOpen)) {
        return parseObjectExpression();
    }
    
    throw ParseError("Unexpected token in expression", peek().line, peek().column);
}

ExpressionPtr Parser::parseArrayExpression() {
    consume(TokenType::BracketOpen, "Expected '['");
    
    std::vector<ExpressionPtr> elements;
    if (!check(TokenType::BracketClose)) {
        do {
            elements.push_back(parseExpression());
        } while (match({TokenType::Comma}));
    }
    
    consume(TokenType::BracketClose, "Expected ']'");
    return std::make_unique<ArrayExpression>(std::move(elements));
}

ExpressionPtr Parser::parseObjectExpression() {
    consume(TokenType::BraceOpen, "Expected '{'");
    
    std::vector<std::unique_ptr<Property>> properties;
    
    while (!check(TokenType::BraceClose) && !isAtEnd()) {
        // Property key
        ExpressionPtr key;
        bool computed = false;
        
        if (match({TokenType::BracketOpen})) {
            computed = true;
            key = parseExpression();
            consume(TokenType::BracketClose, "Expected ']' after computed property");
        } else if (match({TokenType::Identifier})) {
            key = std::make_unique<Identifier>(tokens[current - 1].value);
        } else if (match({TokenType::String})) {
            key = std::make_unique<StringLiteral>(tokens[current - 1].value);
        } else {
            throw ParseError("Expected property name", peek().line, peek().column);
        }
        
        consume(TokenType::Colon, "Expected ':' after property key");
        auto value = parseExpression();
        
        auto property = std::make_unique<Property>();
        property->key = std::move(key);
        property->value = std::move(value);
        property->computed = computed;
        
        properties.push_back(std::move(property));
        
        if (!match({TokenType::Comma})) break;
    }
    
    consume(TokenType::BraceClose, "Expected '}'");
    return std::make_unique<ObjectExpression>(std::move(properties));
}

// Import statement parsing
StatementPtr Parser::parseImport() {
    consumeKeyword("import", "Expected 'import'");
    
    std::vector<std::string> bindings;
    
    consume(TokenType::BraceOpen, "Expected '{' after 'import'");
    if (!check(TokenType::BraceClose)) {
        do {
            Token binding = consume(TokenType::Identifier, "Expected import binding");
            bindings.push_back(binding.value);
        } while (match({TokenType::Comma}));
    }
    consume(TokenType::BraceClose, "Expected '}' after import bindings");
    
    consumeKeyword("from", "Expected 'from' after import bindings");
    Token sourceToken = consume(TokenType::String, "Expected import source string");
    consume(TokenType::SemiColon, "Expected ';' after import statement");
    
    return std::make_unique<Import>(bindings, sourceToken.value);
}

// Export statement parsing
StatementPtr Parser::parseExport() {
    consumeKeyword("export", "Expected 'export'");
    
    bool isDefault = false;
    if (matchKeyword("default")) {
        isDefault = true;
    }
    
    StatementPtr declaration = nullptr;
    if (matchKeyword("function")) {
        declaration = parseFunction();
    } else if (matchKeyword("component")) {
        declaration = parseComponent();
    } else {
        // Export expression
        auto expr = parseExpression();
        consume(TokenType::SemiColon, "Expected ';' after export expression");
        declaration = std::make_unique<ExpressionStatement>(std::move(expr));
    }
    
    return std::make_unique<Export>(std::move(declaration), isDefault);
}

// Parameter list parsing
std::vector<std::string> Parser::parseParameterList() {
    std::vector<std::string> parameters;
    
    if (!check(TokenType::ParenClose)) {
        do {
            Token param = consume(TokenType::Identifier, "Expected parameter name");
            parameters.push_back(param.value);
            
            // Optional type annotation
            if (match({TokenType::Colon})) {
                consume(TokenType::Identifier, "Expected parameter type");
                // Type information could be stored separately if needed
            }
        } while (match({TokenType::Comma}));
    }
    
    return parameters;
}

// ALTX content parsing (simplified for Phase 2B)
std::vector<ASTNodePtr> Parser::parseALTXContent() {
    std::vector<ASTNodePtr> nodes;
    
    while (!check(TokenType::BraceClose) && !isAtEnd()) {
        if (check(TokenType::Less)) {
            nodes.push_back(parseTag());
        } else if (check(TokenType::Text)) {
            nodes.push_back(parseTextContent());
        } else {
            break;
        }
    }
    
    return nodes;
}

TagPtr Parser::parseTag() {
    consume(TokenType::Less, "Expected '<'");
    Token tagNameToken = consume(TokenType::Identifier, "Expected tag name");
    
    auto tag = std::make_unique<Tag>(tagNameToken.value);
    
    // Parse attributes (simplified)
    while (!check(TokenType::Greater) && !check(TokenType::SlashGreater) && !isAtEnd()) {
        advance(); // Skip attribute parsing for now
    }
    
    if (match({TokenType::SlashGreater})) {
        tag->isSelfClosing = true;
    } else {
        consume(TokenType::Greater, "Expected '>'");
        
        // Parse children (simplified)
        while (!check(TokenType::Less) && !isAtEnd()) {
            if (check(TokenType::Text)) {
                tag->children.push_back(parseTextContent());
            } else {
                break;
            }
        }
        
        // Closing tag
        if (check(TokenType::Less)) {
            advance();
            consume(TokenType::Slash, "Expected '/' in closing tag");
            consume(TokenType::Identifier, "Expected closing tag name");
            consume(TokenType::Greater, "Expected '>' after closing tag");
        }
    }
    
    return tag;
}

std::unique_ptr<TextContent> Parser::parseTextContent() {
    Token textToken = consume(TokenType::Text, "Expected text content");
    return std::make_unique<TextContent>(textToken.value);
}

std::unique_ptr<Attribute> Parser::parseAttribute() {
    // Simplified attribute parsing
    Token nameToken = consume(TokenType::Identifier, "Expected attribute name");
    
    ExpressionPtr value = nullptr;
    if (match({TokenType::Assign})) {
        value = parseExpression();
    }
    
    return std::make_unique<Attribute>(nameToken.value, std::move(value));
}

std::vector<StyleProperty> Parser::parseStyleProperty(const std::string& styleContent) {
    // Simplified style parsing - just return empty for now
    return std::vector<StyleProperty>();
}
