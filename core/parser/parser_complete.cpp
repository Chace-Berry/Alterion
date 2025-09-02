#include "../include/parser.h"
#include "../include/ast_complete.h"
#include "../include/token.h"
#include <algorithm>
#include <cctype>
#include <memory>
#include <stdexcept>
#include <iostream>

// Complete Alterion Language Parser Implementation

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

std::unique_ptr<Program> Parser::parseProgram() {
    auto program = std::make_unique<Program>();
    
    while (!isAtEnd()) {
        try {
            if (matchKeyword("component")) {
                program->components.push_back(parseComponent());
            } else if (matchKeyword("import")) {
                program->globalStatements.push_back(parseImport());
            } else if (matchKeyword("export")) {
                program->globalStatements.push_back(parseExport());
            } else if (matchKeyword("function") || matchKeyword("fn")) {
                program->functions.push_back(parseFunction());
            } else if (check(TokenType::AtModifier)) {
                // Handle global modifiers
                program->globalStatements.push_back(parseModifiedStatement());
            } else {
                // Global statement
                program->globalStatements.push_back(parseStatement());
            }
        } catch (const ParseError& error) {
            std::cerr << "Parse Error: " << error.message << " at " << error.line << ":" << error.column << std::endl;
            synchronize();
        }
    }
    
    return program;
}

ComponentPtr Parser::parseComponent() {
    Token componentToken = advance(); // consume 'component'
    
    Token nameToken = consume(TokenType::Identifier, "Expected component name");
    std::string componentName = nameToken.value;
    
    consume(TokenType::BraceOpen, "Expected '{' after component name");
    
    auto component = std::make_unique<Component>(componentName, ComponentType::MIXED, 
                                                nameToken.line, nameToken.column);
    
    // Parse component body
    while (!check(TokenType::BraceClose) && !isAtEnd()) {
        if (matchKeyword("render")) {
            // Parse render method
            consume(TokenType::Colon, "Expected ':' after 'render'");
            component->body = parseALTXContent();
        } else if (check(TokenType::Identifier) && checkNext(TokenType::Equals)) {
            // State variable
            component->statements.push_back(parseAssignment());
        } else if (check(TokenType::Identifier) && checkNext(TokenType::BraceOpen)) {
            // Method definition
            component->statements.push_back(parseMethodDefinition());
        } else if (check(TokenType::TagOpen)) {
            // Direct ALTX content
            component->body.push_back(parseTag());
        } else {
            // Other statements
            component->statements.push_back(parseStatement());
        }
    }
    
    consume(TokenType::BraceClose, "Expected '}' after component body");
    return component;
}

std::vector<ASTNodePtr> Parser::parseALTXContent() {
    std::vector<ASTNodePtr> content;
    
    while (!check(TokenType::BraceClose) && !isAtEnd() && 
           !matchKeyword("render") && !check(TokenType::Identifier)) {
        
        if (check(TokenType::TagOpen)) {
            content.push_back(parseTag());
        } else if (check(TokenType::Text)) {
            content.push_back(parseTextContent());
        } else if (check(TokenType::ExpressionStart)) {
            content.push_back(parseEmbeddedExpression());
        } else {
            advance(); // Skip unknown tokens
        }
    }
    
    return content;
}

TagPtr Parser::parseTag() {
    Token tagToken = consume(TokenType::TagOpen, "Expected tag");
    auto tag = std::make_unique<Tag>(tagToken.value, tagToken.line, tagToken.column);
    
    // Parse attributes
    while (!check(TokenType::TagEnd) && !check(TokenType::TagSelfClose) && !isAtEnd()) {
        if (check(TokenType::AttributeName)) {
            tag->attributes.push_back(parseAttribute());
        } else if (check(TokenType::StyleProperty)) {
            Token styleToken = advance();
            tag->styles = parseStyleProperty(styleToken.value);
        } else {
            advance(); // Skip unknown tokens
        }
    }
    
    if (match({TokenType::TagSelfClose})) {
        tag->isSelfClosing = true;
        return tag;
    }
    
    consume(TokenType::TagEnd, "Expected '>' after tag");
    
    // Parse tag content
    while (!check(TokenType::TagClose) && !isAtEnd()) {
        if (check(TokenType::TagOpen)) {
            tag->children.push_back(parseTag());
        } else if (check(TokenType::Text)) {
            tag->children.push_back(parseTextContent());
        } else if (check(TokenType::ExpressionStart)) {
            tag->children.push_back(parseEmbeddedExpression());
        } else {
            advance(); // Skip unknown tokens
        }
    }
    
    if (check(TokenType::TagClose)) {
        Token closeTag = advance();
        if (closeTag.value != tag->tagName) {
            throw ParseError("Mismatched closing tag: expected </" + tag->tagName + 
                           "> but got </" + closeTag.value + ">", 
                           closeTag.line, closeTag.column);
        }
    }
    
    return tag;
}

std::unique_ptr<Attribute> Parser::parseAttribute() {
    Token nameToken = consume(TokenType::AttributeName, "Expected attribute name");
    
    if (match({TokenType::Equals})) {
        ExpressionPtr value;
        
        if (check(TokenType::String)) {
            value = std::make_unique<StringLiteral>(advance().value);
        } else if (check(TokenType::ExpressionStart)) {
            advance(); // consume '{'
            value = parseExpression();
            consume(TokenType::ExpressionEnd, "Expected '}' after expression");
        } else if (check(TokenType::ValueBinding)) {
            std::string binding = advance().value;
            std::string bindingName = binding.substr(1); // Remove '!'
            value = std::make_unique<ValueBinding>(bindingName);
        } else if (check(TokenType::Identifier)) {
            value = std::make_unique<Identifier>(advance().value);
        } else {
            throw ParseError("Expected attribute value after '='", peek().line, peek().column);
        }
        
        return std::make_unique<Attribute>(nameToken.value, std::move(value));
    } else {
        // Boolean attribute
        return std::make_unique<Attribute>(nameToken.value, 
                                         std::make_unique<BooleanLiteral>(true));
    }
}

std::vector<StyleProperty> Parser::parseStyleProperty(const std::string& styleContent) {
    std::vector<StyleProperty> styles;
    
    // Simple CSS-like property parsing
    size_t pos = 0;
    while (pos < styleContent.length()) {
        size_t colonPos = styleContent.find(':', pos);
        if (colonPos == std::string::npos) break;
        
        std::string property = styleContent.substr(pos, colonPos - pos);
        
        // Trim whitespace
        property.erase(0, property.find_first_not_of(" \\t"));
        property.erase(property.find_last_not_of(" \\t") + 1);
        
        pos = colonPos + 1;
        size_t semicolonPos = styleContent.find(';', pos);
        if (semicolonPos == std::string::npos) semicolonPos = styleContent.length();
        
        std::string value = styleContent.substr(pos, semicolonPos - pos);
        value.erase(0, value.find_first_not_of(" \\t"));
        value.erase(value.find_last_not_of(" \\t") + 1);
        
        if (!property.empty() && !value.empty()) {
            styles.emplace_back(property, value);
        }
        
        pos = semicolonPos + 1;
    }
    
    return styles;
}

std::unique_ptr<TextContent> Parser::parseTextContent() {
    Token textToken = consume(TokenType::Text, "Expected text content");
    return std::make_unique<TextContent>(textToken.value, textToken.line, textToken.column);
}

StatementPtr Parser::parseEmbeddedExpression() {
    consume(TokenType::ExpressionStart, "Expected '{'");
    auto expr = parseExpression();
    consume(TokenType::ExpressionEnd, "Expected '}' after expression");
    return std::make_unique<ExpressionStatement>(std::move(expr));
}

StatementPtr Parser::parseImport() {
    Token importToken = advance(); // consume 'import'
    
    consume(TokenType::BraceOpen, "Expected '{' after 'import'");
    
    std::vector<std::string> bindings = parseImportList();
    
    consume(TokenType::BraceClose, "Expected '}' after import list");
    consumeKeyword("from", "Expected 'from' after import bindings");
    
    Token sourceToken = consume(TokenType::String, "Expected module name");
    std::string source = sourceToken.value;
    
    return std::make_unique<Import>(std::move(bindings), source, false, 
                                   importToken.line, importToken.column);
}

std::vector<std::string> Parser::parseImportList() {
    std::vector<std::string> bindings;
    
    if (!check(TokenType::BraceClose)) {
        do {
            Token identifier = consume(TokenType::Identifier, "Expected identifier in import list");
            bindings.push_back(identifier.value);
        } while (match({TokenType::Comma}));
    }
    
    return bindings;
}

StatementPtr Parser::parseExport() {
    Token exportToken = advance(); // consume 'export'
    
    bool isDefault = false;
    if (matchKeyword("default")) {
        isDefault = true;
    }
    
    StatementPtr declaration;
    
    if (matchKeyword("function") || matchKeyword("fn")) {
        declaration = parseFunction();
    } else if (matchKeyword("component")) {
        declaration = parseComponent();
    } else if (check(TokenType::Identifier)) {
        // Export variable or expression
        declaration = parseStatement();
    } else {
        throw ParseError("Expected declaration after 'export'", peek().line, peek().column);
    }
    
    return std::make_unique<Export>(std::move(declaration), isDefault, 
                                   exportToken.line, exportToken.column);
}

FunctionPtr Parser::parseFunction() {
    Token funcToken = advance(); // consume 'function' or 'fn'
    
    Token nameToken = consume(TokenType::Identifier, "Expected function name");
    std::string functionName = nameToken.value;
    
    consume(TokenType::ParenOpen, "Expected '(' after function name");
    
    std::vector<std::string> parameters = parseParameterList();
    
    consume(TokenType::ParenClose, "Expected ')' after parameters");
    
    StatementPtr body = parseBlockStatement();
    
    return std::make_unique<Function>(functionName, std::move(parameters), std::move(body),
                                     FunctionType::REGULAR, funcToken.line, funcToken.column);
}

std::vector<std::string> Parser::parseParameterList() {
    std::vector<std::string> parameters;
    
    if (!check(TokenType::ParenClose)) {
        do {
            Token param = consume(TokenType::Identifier, "Expected parameter name");
            parameters.push_back(param.value);
            
            // Skip optional type annotations for now
            if (match({TokenType::Colon})) {
                consume(TokenType::Identifier, "Expected type after ':'");
            }
        } while (match({TokenType::Comma}));
    }
    
    return parameters;
}

StatementPtr Parser::parseMethodDefinition() {
    Token nameToken = consume(TokenType::Identifier, "Expected method name");
    std::string methodName = nameToken.value;
    
    consume(TokenType::BraceOpen, "Expected '{' after method name");
    
    std::vector<StatementPtr> statements;
    while (!check(TokenType::BraceClose) && !isAtEnd()) {
        statements.push_back(parseStatement());
    }
    
    consume(TokenType::BraceClose, "Expected '}' after method body");
    
    auto body = std::make_unique<BlockStatement>(std::move(statements), nameToken.line, nameToken.column);
    
    return std::make_unique<Function>(methodName, std::vector<std::string>(), std::move(body),
                                     FunctionType::REGULAR, nameToken.line, nameToken.column);
}

StatementPtr Parser::parseModifiedStatement() {
    std::vector<std::string> modifiers;
    
    while (check(TokenType::AtModifier)) {
        modifiers.push_back(advance().value);
    }
    
    // Parse the actual statement
    StatementPtr stmt = parseStatement();
    
    // Apply modifiers to the statement (implementation depends on statement type)
    // For now, just return the statement
    return stmt;
}

StatementPtr Parser::parseStatement() {
    if (matchKeyword("import")) {
        return parseImport();
    }
    
    if (matchKeyword("export")) {
        return parseExport();
    }
    
    if (matchKeyword("if")) {
        return parseIfStatement();
    }
    
    if (matchKeyword("while")) {
        return parseWhileStatement();
    }
    
    if (matchKeyword("for")) {
        return parseForStatement();
    }
    
    if (matchKeyword("return")) {
        return parseReturnStatement();
    }
    
    if (matchKeyword("break")) {
        advance();
        return std::make_unique<BreakStatement>(peek().line, peek().column);
    }
    
    if (matchKeyword("continue")) {
        advance();
        return std::make_unique<ContinueStatement>(peek().line, peek().column);
    }
    
    if (matchKeyword("try")) {
        return parseTryStatement();
    }
    
    if (matchKeyword("throw")) {
        return parseThrowStatement();
    }
    
    if (matchKeyword("let") || matchKeyword("const") || matchKeyword("var")) {
        return parseVariableDeclaration();
    }
    
    if (match({TokenType::BraceOpen})) {
        return parseBlockStatement();
    }
    
    // Check for assignment
    if (check(TokenType::Identifier) && checkNext(TokenType::Equals)) {
        return parseAssignment();
    }
    
    // Expression statement
    auto expr = parseExpression();
    return std::make_unique<ExpressionStatement>(std::move(expr), peek().line, peek().column);
}

StatementPtr Parser::parseBlockStatement() {
    Token braceToken = tokens[current - 1]; // Already consumed '{'
    std::vector<StatementPtr> statements;
    
    while (!check(TokenType::BraceClose) && !isAtEnd()) {
        statements.push_back(parseStatement());
    }
    
    consume(TokenType::BraceClose, "Expected '}' after block");
    
    return std::make_unique<BlockStatement>(std::move(statements), braceToken.line, braceToken.column);
}

StatementPtr Parser::parseIfStatement() {
    Token ifToken = tokens[current - 1]; // Already consumed 'if'
    
    consume(TokenType::ParenOpen, "Expected '(' after 'if'");
    auto condition = parseExpression();
    consume(TokenType::ParenClose, "Expected ')' after if condition");
    
    auto thenBranch = parseStatement();
    StatementPtr elseBranch = nullptr;
    
    if (matchKeyword("else")) {
        elseBranch = parseStatement();
    }
    
    return std::make_unique<IfStatement>(std::move(condition), std::move(thenBranch), 
                                        std::move(elseBranch), ifToken.line, ifToken.column);
}

StatementPtr Parser::parseWhileStatement() {
    Token whileToken = tokens[current - 1]; // Already consumed 'while'
    
    consume(TokenType::ParenOpen, "Expected '(' after 'while'");
    auto condition = parseExpression();
    consume(TokenType::ParenClose, "Expected ')' after while condition");
    
    auto body = parseStatement();
    
    return std::make_unique<WhileStatement>(std::move(condition), std::move(body), 
                                          whileToken.line, whileToken.column);
}

StatementPtr Parser::parseForStatement() {
    Token forToken = tokens[current - 1]; // Already consumed 'for'
    
    // Check for for-in loop
    if (check(TokenType::Identifier)) {
        size_t saved = current;
        advance(); // identifier
        if (matchKeyword("in")) {
            current = saved; // backtrack
            return parseForInStatement();
        }
        current = saved; // backtrack
    }
    
    consume(TokenType::ParenOpen, "Expected '(' after 'for'");
    
    // Init
    StatementPtr init = nullptr;
    if (!check(TokenType::SemiColon)) {
        if (matchKeyword("let") || matchKeyword("const") || matchKeyword("var")) {
            init = parseVariableDeclaration();
        } else {
            auto expr = parseExpression();
            init = std::make_unique<ExpressionStatement>(std::move(expr));
        }
    }
    consume(TokenType::SemiColon, "Expected ';' after for loop initializer");
    
    // Condition
    ExpressionPtr condition = nullptr;
    if (!check(TokenType::SemiColon)) {
        condition = parseExpression();
    }
    consume(TokenType::SemiColon, "Expected ';' after for loop condition");
    
    // Update
    ExpressionPtr update = nullptr;
    if (!check(TokenType::ParenClose)) {
        update = parseExpression();
    }
    consume(TokenType::ParenClose, "Expected ')' after for clauses");
    
    auto body = parseStatement();
    
    return std::make_unique<ForStatement>(std::move(init), std::move(condition), 
                                         std::move(update), std::move(body), 
                                         forToken.line, forToken.column);
}

StatementPtr Parser::parseForInStatement() {
    Token forToken = tokens[current - 1]; // Already consumed 'for'
    
    Token varToken = consume(TokenType::Identifier, "Expected variable name in for-in loop");
    consumeKeyword("in", "Expected 'in' in for-in loop");
    auto iterable = parseExpression();
    
    auto body = parseStatement();
    
    return std::make_unique<ForInStatement>(varToken.value, std::move(iterable), 
                                          std::move(body), forToken.line, forToken.column);
}

StatementPtr Parser::parseReturnStatement() {
    Token returnToken = tokens[current - 1]; // Already consumed 'return'
    
    ExpressionPtr value = nullptr;
    if (!check(TokenType::SemiColon) && !check(TokenType::BraceClose) && !isAtEnd()) {
        value = parseExpression();
    }
    
    return std::make_unique<ReturnStatement>(std::move(value), returnToken.line, returnToken.column);
}

StatementPtr Parser::parseTryStatement() {
    Token tryToken = tokens[current - 1]; // Already consumed 'try'
    
    auto block = parseBlockStatement();
    
    auto tryStmt = std::make_unique<TryStatement>(std::move(block), tryToken.line, tryToken.column);
    
    if (matchKeyword("catch")) {
        consume(TokenType::ParenOpen, "Expected '(' after 'catch'");
        Token varToken = consume(TokenType::Identifier, "Expected catch variable");
        consume(TokenType::ParenClose, "Expected ')' after catch variable");
        
        tryStmt->catchVariable = varToken.value;
        tryStmt->catchBlock = parseBlockStatement();
    }
    
    if (matchKeyword("finally")) {
        tryStmt->finallyBlock = parseBlockStatement();
    }
    
    return tryStmt;
}

StatementPtr Parser::parseThrowStatement() {
    Token throwToken = tokens[current - 1]; // Already consumed 'throw'
    
    auto expr = parseExpression();
    
    return std::make_unique<ThrowStatement>(std::move(expr), throwToken.line, throwToken.column);
}

StatementPtr Parser::parseVariableDeclaration() {
    Token kindToken = tokens[current - 1]; // Already consumed 'let', 'const', or 'var'
    
    Token nameToken = consume(TokenType::Identifier, "Expected variable name");
    
    ExpressionPtr initializer = nullptr;
    if (match({TokenType::Equals})) {
        initializer = parseExpression();
    }
    
    return std::make_unique<VariableDeclaration>(nameToken.value, std::move(initializer), 
                                               kindToken.value, nameToken.line, nameToken.column);
}

StatementPtr Parser::parseAssignment() {
    Token identifier = consume(TokenType::Identifier, "Expected identifier");
    
    std::string operator_ = "=";
    if (check(TokenType::Operator) && 
        (peek().value == "+=" || peek().value == "-=" || peek().value == "*=" || peek().value == "/=")) {
        operator_ = advance().value;
    } else {
        consume(TokenType::Equals, "Expected '=' in assignment");
    }
    
    ExpressionPtr value = parseExpression();
    
    return std::make_unique<Assignment>(identifier.value, std::move(value), operator_, 
                                       identifier.line, identifier.column);
}

// Expression parsing with precedence climbing
ExpressionPtr Parser::parseExpression() {
    return parseLogicalOr();
}

ExpressionPtr Parser::parseLogicalOr() {
    auto expr = parseLogicalAnd();
    
    while (check(TokenType::Operator) && peek().value == "||") {
        std::string operator_ = advance().value;
        auto right = parseLogicalAnd();
        expr = std::make_unique<BinaryExpression>(std::move(expr), operator_, std::move(right));
    }
    
    return expr;
}

ExpressionPtr Parser::parseLogicalAnd() {
    auto expr = parseEquality();
    
    while (check(TokenType::Operator) && peek().value == "&&") {
        std::string operator_ = advance().value;
        auto right = parseEquality();
        expr = std::make_unique<BinaryExpression>(std::move(expr), operator_, std::move(right));
    }
    
    return expr;
}

ExpressionPtr Parser::parseEquality() {
    auto expr = parseComparison();
    
    while (check(TokenType::Operator) && 
           (peek().value == "==" || peek().value == "!=")) {
        std::string operator_ = advance().value;
        auto right = parseComparison();
        expr = std::make_unique<BinaryExpression>(std::move(expr), operator_, std::move(right));
    }
    
    return expr;
}

ExpressionPtr Parser::parseComparison() {
    auto expr = parseTerm();
    
    while (check(TokenType::Operator) && 
           (peek().value == ">" || peek().value == ">=" || 
            peek().value == "<" || peek().value == "<=")) {
        std::string operator_ = advance().value;
        auto right = parseTerm();
        expr = std::make_unique<BinaryExpression>(std::move(expr), operator_, std::move(right));
    }
    
    return expr;
}

ExpressionPtr Parser::parseTerm() {
    auto expr = parseFactor();
    
    while (check(TokenType::Operator) && 
           (peek().value == "+" || peek().value == "-")) {
        std::string operator_ = advance().value;
        auto right = parseFactor();
        expr = std::make_unique<BinaryExpression>(std::move(expr), operator_, std::move(right));
    }
    
    return expr;
}

ExpressionPtr Parser::parseFactor() {
    auto expr = parseUnary();
    
    while (check(TokenType::Operator) && 
           (peek().value == "*" || peek().value == "/" || peek().value == "%")) {
        std::string operator_ = advance().value;
        auto right = parseUnary();
        expr = std::make_unique<BinaryExpression>(std::move(expr), operator_, std::move(right));
    }
    
    return expr;
}

ExpressionPtr Parser::parseUnary() {
    if (check(TokenType::Operator) && 
        (peek().value == "!" || peek().value == "-" || peek().value == "+")) {
        std::string operator_ = advance().value;
        auto right = parseUnary();
        return std::make_unique<UnaryExpression>(operator_, std::move(right));
    }
    
    return parseCall();
}

ExpressionPtr Parser::parseCall() {
    auto expr = parsePrimary();
    
    while (true) {
        if (match({TokenType::ParenOpen})) {
            std::vector<ExpressionPtr> arguments;
            
            if (!check(TokenType::ParenClose)) {
                do {
                    arguments.push_back(parseExpression());
                } while (match({TokenType::Comma}));
            }
            
            consume(TokenType::ParenClose, "Expected ')' after arguments");
            expr = std::make_unique<CallExpression>(std::move(expr), std::move(arguments));
        } else if (match({TokenType::Dot})) {
            Token name = consume(TokenType::Identifier, "Expected property name after '.'");
            auto property = std::make_unique<Identifier>(name.value);
            expr = std::make_unique<MemberExpression>(std::move(expr), std::move(property), false);
        } else if (match({TokenType::BracketOpen})) {
            auto index = parseExpression();
            consume(TokenType::BracketClose, "Expected ']' after array index");
            expr = std::make_unique<MemberExpression>(std::move(expr), std::move(index), true);
        } else {
            break;
        }
    }
    
    return expr;
}

ExpressionPtr Parser::parsePrimary() {
    if (match({TokenType::String})) {
        return std::make_unique<StringLiteral>(tokens[current - 1].value);
    }
    
    if (match({TokenType::Number})) {
        std::string numberStr = tokens[current - 1].value;
        bool isFloat = numberStr.find('.') != std::string::npos || 
                      numberStr.find('e') != std::string::npos || 
                      numberStr.find('E') != std::string::npos;
        return std::make_unique<NumberLiteral>(numberStr, isFloat);
    }
    
    if (matchKeyword("true")) {
        return std::make_unique<BooleanLiteral>(true);
    }
    
    if (matchKeyword("false")) {
        return std::make_unique<BooleanLiteral>(false);
    }
    
    if (matchKeyword("null") || matchKeyword("none")) {
        return std::make_unique<NullLiteral>();
    }
    
    if (match({TokenType::ValueBinding})) {
        std::string binding = tokens[current - 1].value;
        std::string bindingName = binding.substr(1); // Remove '!'
        return std::make_unique<ValueBinding>(bindingName);
    }
    
    if (match({TokenType::Identifier})) {
        return std::make_unique<Identifier>(tokens[current - 1].value);
    }
    
    if (match({TokenType::ParenOpen})) {
        auto expr = parseExpression();
        consume(TokenType::ParenClose, "Expected ')' after expression");
        return expr;
    }
    
    if (match({TokenType::BracketOpen})) {
        return parseArrayExpression();
    }
    
    if (match({TokenType::BraceOpen})) {
        return parseObjectExpression();
    }
    
    throw ParseError("Unexpected token in expression: '" + peek().value + "'", 
                     peek().line, peek().column);
}

ExpressionPtr Parser::parseArrayExpression() {
    std::vector<ExpressionPtr> elements;
    
    if (!check(TokenType::BracketClose)) {
        do {
            elements.push_back(parseExpression());
        } while (match({TokenType::Comma}));
    }
    
    consume(TokenType::BracketClose, "Expected ']' after array elements");
    
    return std::make_unique<ArrayExpression>(std::move(elements));
}

ExpressionPtr Parser::parseObjectExpression() {
    std::vector<std::unique_ptr<ObjectProperty>> properties;
    
    if (!check(TokenType::BraceClose)) {
        do {
            ExpressionPtr key;
            
            if (check(TokenType::Identifier)) {
                key = std::make_unique<StringLiteral>(advance().value);
            } else if (check(TokenType::String)) {
                key = std::make_unique<StringLiteral>(advance().value);
            } else if (match({TokenType::BracketOpen})) {
                key = parseExpression();
                consume(TokenType::BracketClose, "Expected ']' after computed property");
            } else {
                throw ParseError("Expected property name", peek().line, peek().column);
            }
            
            consume(TokenType::Colon, "Expected ':' after property name");
            auto value = parseExpression();
            
            properties.push_back(std::make_unique<ObjectProperty>(std::move(key), std::move(value)));
        } while (match({TokenType::Comma}));
    }
    
    consume(TokenType::BraceClose, "Expected '}' after object properties");
    
    return std::make_unique<ObjectExpression>(std::move(properties));
}

bool Parser::checkNext(TokenType type) {
    if (current + 1 >= tokens.size()) return false;
    return tokens[current + 1].type == type;
}
