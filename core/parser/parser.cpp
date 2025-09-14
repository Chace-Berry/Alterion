#include "../include/parser.h"
#include "../include/token.h"
#include "../include/ast.h"
#include <stdexcept>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <iostream>

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), position(0) {}

std::shared_ptr<ASTNode> Parser::parse() {
    return parseProgram();
}

const Token& Parser::peek() const {
    if (position < tokens.size()) return tokens[position];
    static Token eof(TokenType::EOFToken, "", 0, 0);
    return eof;
}

const Token& Parser::advance() {
    if (position < tokens.size()) return tokens[position++];
    static Token eof(TokenType::EOFToken, "", 0, 0);
    return eof;
}

bool Parser::match(TokenType type) {
    if (peek().type == type) {
        advance();
        return true;
    }
    return false;
}

bool Parser::eof() const {
    return position >= tokens.size() || peek().type == TokenType::EOFToken;
}

void Parser::expectToken(TokenType type, const std::string& message) {
    if (!match(type)) {
        throw std::runtime_error("Parse error: " + message + " at line " + 
                                std::to_string(peek().line) + ", column " + 
                                std::to_string(peek().column));
    }
}

std::shared_ptr<ASTNode> Parser::parseProgram() {
    auto node = std::make_shared<ASTNode>(ASTNodeType::Program);
    node->line = peek().line;
    node->column = peek().column;
    size_t stuckCount = 0;
    size_t lastPos = position;
    
    while (!eof()) {
        size_t before = position;
        try {
            if (peek().type == TokenType::Keyword && peek().value == "component") {
                node->children.push_back(parseComponent());
            } else if (peek().type == TokenType::Keyword && peek().value == "function") {
                node->children.push_back(parseFunction());
            } else if (peek().type == TokenType::Keyword && 
                      (peek().value == "let" || peek().value == "const" || peek().value == "var")) {
                node->children.push_back(parseVariable());
            } else {
                node->children.push_back(parseStatement());
            }
        } catch (const std::exception& e) {
            node->children.push_back(parseError(e.what()));
            skipToNextStatement();
        }
        
        if (position == before) {
            stuckCount++;
            advance();
            if (stuckCount > 1000) {
                std::cout << "ERROR: Parser stuck in infinite loop at position " << position << std::endl;
                break;
            }
        } else {
            stuckCount = 0;
            lastPos = position;
        }
    }
    return node;
}

std::shared_ptr<ASTNode> Parser::parseComponent() {
    auto node = std::make_shared<ASTNode>(ASTNodeType::Component);
    node->line = peek().line;
    node->column = peek().column;
    
    expectToken(TokenType::Keyword, "Expected 'component' keyword");
    
    
    if (peek().type == TokenType::Identifier) {
        node->name = peek().value;
        advance();
    } else {
        throw std::runtime_error("Expected component name after 'component'");
    }
    
    
    if (match(TokenType::ParenOpen)) {
        while (!eof() && peek().type != TokenType::ParenClose) {
            if (peek().type == TokenType::Identifier) {
                auto param = std::make_shared<ASTNode>(ASTNodeType::Variable);
                param->name = peek().value;
                node->children.push_back(param);
                advance();
                
                if (peek().type == TokenType::Comma) {
                    advance();
                } else if (peek().type != TokenType::ParenClose) {
                    throw std::runtime_error("Expected ',' or ')' in parameter list");
                }
            } else {
                throw std::runtime_error("Expected parameter name");
            }
        }
        expectToken(TokenType::ParenClose, "Expected ')' after component parameters");
    }
    
    expectToken(TokenType::BraceOpen, "Expected '{' to start component body");
    
    
    size_t stuckCount = 0;
    size_t lastPos = position;
    
    while (!eof() && peek().type != TokenType::BraceClose) {
        size_t before = position;
        try {
            if (peek().type == TokenType::Keyword && peek().value == "render") {
                
                auto renderNode = std::make_shared<ASTNode>(ASTNodeType::Render);
                advance(); 
                
                
                if (peek().type == TokenType::Colon) {
                    advance();
                } else {
                    throw std::runtime_error("Expected ':' after 'render'");
                }
                
                
                renderNode->children.push_back(parseExpression());
                node->children.push_back(renderNode);
                
            } else if (peek().type == TokenType::Identifier) {
                
                auto propNode = std::make_shared<ASTNode>(ASTNodeType::Assignment);
                propNode->name = peek().value;
                advance();
                
                if (peek().type == TokenType::Equals) {
                    advance();
                    propNode->children.push_back(parseExpression());
                } else {
                    throw std::runtime_error("Expected '=' in property declaration");
                }
                
                node->children.push_back(propNode);
                
            } else if (peek().type == TokenType::AtModifier) {
                
                std::string modifier = peek().value;
                advance(); 
                if (modifier == "async") {
                    if (peek().type == TokenType::BraceOpen) {
                        advance(); 
                        auto asyncNode = std::make_shared<ASTNode>(ASTNodeType::AsyncBlock);
                        asyncNode->line = peek().line;
                        asyncNode->column = peek().column;
                        while (!eof() && peek().type != TokenType::BraceClose) {
                            if (peek().type == TokenType::SquareBracketOpen) {
                                asyncNode->children.push_back(parseBracketBlock(false));
                            } else {
                                skipToNextStatement(); 
                            }
                        }
                        expectToken(TokenType::BraceClose, "Expected '}' after async block");
                        node->children.push_back(asyncNode);
                    } else {
                        throw std::runtime_error("Expected '{' after @async");
                    }
                } else if (peek().type == TokenType::Identifier) {
                    auto funcNode = parseFunction();
                    funcNode->decorator = modifier;
                    node->children.push_back(funcNode);
                } else {
                    throw std::runtime_error("Expected function after decorator");
                }
                
            } else {
                
                node->children.push_back(parseStatement());
            }
        } catch (const std::exception& e) {
            node->children.push_back(parseError(e.what()));
            skipToNextStatement();
        }
        
        if (position == before) {
            stuckCount++;
            advance();
            if (stuckCount > 100) {
                std::cout << "ERROR: Parser stuck in component parsing at position " << position << std::endl;
                break;
            }
        } else {
            stuckCount = 0;
            lastPos = position;
        }
    }
    
    expectToken(TokenType::BraceClose, "Expected '}' to end component");
    
    return node;
}

std::shared_ptr<ASTNode> Parser::parseFunction() {
    auto node = std::make_shared<ASTNode>(ASTNodeType::Function);
    node->line = peek().line;
    node->column = peek().column;
    
    expectToken(TokenType::Keyword, "Expected 'function' keyword");
    
    
    if (peek().type == TokenType::Identifier) {
        node->name = peek().value;
        advance();
    } else {
        throw std::runtime_error("Expected function name after 'function'");
    }
    
    
    expectToken(TokenType::ParenOpen, "Expected '(' after function name");
    
    while (!eof() && peek().type != TokenType::ParenClose) {
        if (peek().type == TokenType::Identifier) {
            auto param = std::make_shared<ASTNode>(ASTNodeType::Variable);
            param->name = peek().value;
            node->children.push_back(param);
            advance();
            
            if (peek().type == TokenType::Comma) {
                advance();
            } else if (peek().type != TokenType::ParenClose) {
                throw std::runtime_error("Expected ',' or ')' in parameter list");
            }
        } else {
            throw std::runtime_error("Expected parameter name");
        }
    }
    
    expectToken(TokenType::ParenClose, "Expected ')' after parameters");
    
    
    node->children.push_back(parseBlock());
    
    return node;
}

std::shared_ptr<ASTNode> Parser::parseVariable() {
    auto node = std::make_shared<ASTNode>(ASTNodeType::Variable);
    node->line = peek().line;
    node->column = peek().column;
    
    
    if (peek().type == TokenType::Keyword && 
        (peek().value == "let" || peek().value == "const" || peek().value == "var")) {
        node->variableType = peek().value;
        advance();
    } else {
        throw std::runtime_error("Expected variable declaration keyword");
    }
    
    
    if (peek().type == TokenType::Identifier) {
        node->name = peek().value;
        advance();
    } else {
        throw std::runtime_error("Expected variable name");
    }
    
    
    if (peek().type == TokenType::Equals) {
        advance();
        node->children.push_back(parseExpression());
    }
    
    
    if (peek().type == TokenType::SemiColon) {
        advance();
    }
    
    return node;
}

std::shared_ptr<ASTNode> Parser::parseAssignment() {
    auto node = std::make_shared<ASTNode>(ASTNodeType::Assignment);
    node->line = peek().line;
    node->column = peek().column;
    
    
    if (peek().type == TokenType::Identifier) {
        node->name = peek().value;
        advance();
    } else {
        throw std::runtime_error("Expected identifier for assignment");
    }
    
    
    if (peek().type == TokenType::Equals) {
        advance();
    } else {
        throw std::runtime_error("Expected '=' in assignment");
    }
    
    
    node->children.push_back(parseExpression());
    
    return node;
}

std::shared_ptr<ASTNode> Parser::parseStatement() {
    if (peek().type == TokenType::Keyword) {
        const std::string& keyword = peek().value;
        
        if (keyword == "if") {
            return parseIf();
        } else if (keyword == "for") {
            return parseFor();
        } else if (keyword == "while") {
            return parseWhile();
        } else if (keyword == "return") {
            return parseReturn();
        } else if (keyword == "render") {
            return parseRender();
        } else if (keyword == "async") {
            return parseAsyncBlock();
        } else if (keyword == "try") {
            return parseTryBlock();
        }
    }
    
    
    if (peek().type == TokenType::Identifier) {
        size_t saved_pos = position;
        advance(); 
        if (peek().type == TokenType::Equals) {
            position = saved_pos; 
            auto stmt = parseAssignment();
            if (peek().type == TokenType::SemiColon) {
                advance();
            }
            return stmt;
        }
        position = saved_pos; 
    }
    
    
    if (peek().type == TokenType::Operator && peek().value == "<") {
        return parseUIElement();
    }
    
    
    auto node = std::make_shared<ASTNode>(ASTNodeType::Statement);
    node->line = peek().line;
    node->column = peek().column;
    size_t exprStart = position;
    node->children.push_back(parseExpression());
    
    if (position == exprStart && !eof()) {
        advance();
    }
    if (peek().type == TokenType::SemiColon) {
        advance();
    }
    return node;
}

std::shared_ptr<ASTNode> Parser::parseExpression() {
    return parseBinaryExpression(0);
}


std::shared_ptr<ASTNode> Parser::parseBinaryExpression(int minPrecedence) {
    auto left = parsePrimaryExpression();
    
    while (!eof() && peek().type == TokenType::Operator) {
        const std::string& op = peek().value;
        
        
        if (op == "=" || op == ";" || op == ")" || op == "}" || op == ">" || op == "]") {
            break;
        }
        
        int precedence = getOperatorPrecedence(op);
        if (precedence < minPrecedence) {
            break;
        }
        
        advance(); 
        
        auto right = parseBinaryExpression(precedence + 1);
        
        auto opNode = std::make_shared<ASTNode>(ASTNodeType::Expression);
        opNode->name = op;
        opNode->children.push_back(left);
        opNode->children.push_back(right);
        left = opNode;
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parsePrimaryExpression() {
    auto node = std::make_shared<ASTNode>(ASTNodeType::Expression);
    node->line = peek().line;
    node->column = peek().column;
    
    if (peek().type == TokenType::Identifier) {
        node->name = peek().value;
        advance();
        
        
        if (peek().type == TokenType::ParenOpen) {
            auto callNode = std::make_shared<ASTNode>(ASTNodeType::FunctionCall);
            callNode->line = peek().line;
            callNode->column = peek().column;
            callNode->name = node->name;
            advance(); 
            
            while (!eof() && peek().type != TokenType::ParenClose) {
                callNode->children.push_back(parseExpression());
                
                if (peek().type == TokenType::Comma) {
                    advance();
                } else if (peek().type != TokenType::ParenClose) {
                    throw std::runtime_error("Expected ',' or ')' in function call");
                }
            }
            
            expectToken(TokenType::ParenClose, "Expected ')' after function arguments");
            return callNode;
        }
        
        
        while (!eof()) {
            if (peek().type == TokenType::SquareBracketOpen) {
                advance(); 
                node->children.push_back(parseExpression());
                expectToken(TokenType::SquareBracketClose, "Expected ']' after array index");
            } else if (peek().type == TokenType::Dot) {
                advance(); 
                if (peek().type == TokenType::Identifier) {
                    auto propertyNode = std::make_shared<ASTNode>(ASTNodeType::Expression);
                    propertyNode->line = peek().line;
                    propertyNode->column = peek().column;
                    propertyNode->name = peek().value;
                    advance();
                    node->children.push_back(propertyNode);
                } else {
                    throw std::runtime_error("Expected property name after '.'");
                }
            } else {
                break;
            }
        }
        
    } else if (peek().type == TokenType::Number) {
        node->value = peek().value;
        advance();
    } else if (peek().type == TokenType::String) {
        node->value = peek().value;
        advance();
    } else if (peek().type == TokenType::Keyword && 
               (peek().value == "true" || peek().value == "false" || 
                peek().value == "null" || peek().value == "undefined")) {
        node->value = peek().value;
        advance();
    } else if (match(TokenType::ParenOpen)) {
        
        node = parseExpression();
        expectToken(TokenType::ParenClose, "Expected ')' after expression");
    } else if (peek().type == TokenType::SquareBracketOpen) {
        
        advance(); 
    node->name = "array";
    node->line = peek().line;
    node->column = peek().column;
        while (!eof() && peek().type != TokenType::SquareBracketClose) {
            node->children.push_back(parseExpression());
            
            if (peek().type == TokenType::Comma) {
                advance();
            } else if (peek().type != TokenType::SquareBracketClose) {
                throw std::runtime_error("Expected ',' or ']' in array literal");
            }
        }
        expectToken(TokenType::SquareBracketClose, "Expected ']' after array elements");
    } else if (peek().type == TokenType::Operator) {
        
        if (peek().value == "<") {
            return parseUIElement();
        } else {
            
            node->name = peek().value;
            advance();
            node->children.push_back(parsePrimaryExpression());
        }
    } else {
        throw std::runtime_error("Unexpected token in expression: " + peek().value);
    }
    
    return node;
}

int Parser::getOperatorPrecedence(const std::string& op) {
    if (op == "||") return 1;
    if (op == "&&") return 2;
    if (op == "==" || op == "!=" || op == "<=" || op == ">=" || op == "<" || op == ">") return 3;
    if (op == "+" || op == "-") return 4;
    if (op == "*" || op == "/" || op == "%") return 5;
    return 0;
}

std::shared_ptr<ASTNode> Parser::parseBlock() {
    auto node = std::make_shared<ASTNode>(ASTNodeType::Block);
    node->line = peek().line;
    node->column = peek().column;
    
    expectToken(TokenType::BraceOpen, "Expected '{' to start block");
    
    while (!eof() && peek().type != TokenType::BraceClose) {
        try {
            node->children.push_back(parseStatement());
        } catch (const std::exception& e) {
            node->children.push_back(parseError(e.what()));
            skipToNextStatement();
        }
    }
    
    expectToken(TokenType::BraceClose, "Expected '}' to end block");
    
    return node;
}

std::shared_ptr<ASTNode> Parser::parseIf() {
    auto node = std::make_shared<ASTNode>(ASTNodeType::If);
    node->line = peek().line;
    node->column = peek().column;
    
    expectToken(TokenType::Keyword, "Expected 'if' keyword");
    expectToken(TokenType::ParenOpen, "Expected '(' after 'if'");
    
    
    node->children.push_back(parseExpression());
    
    expectToken(TokenType::ParenClose, "Expected ')' after if condition");
    
    
    node->children.push_back(parseStatement());
    
    
    if (peek().type == TokenType::Keyword && peek().value == "else") {
        advance();
        node->children.push_back(parseStatement());
    }
    
    return node;
}

std::shared_ptr<ASTNode> Parser::parseFor() {
    auto node = std::make_shared<ASTNode>(ASTNodeType::For);
    node->line = peek().line;
    node->column = peek().column;
    
    expectToken(TokenType::Keyword, "Expected 'for' keyword");
    expectToken(TokenType::ParenOpen, "Expected '(' after 'for'");
    
    
    if (peek().type != TokenType::SemiColon) {
        node->children.push_back(parseStatement());
    } else {
    auto emptyStmt = std::make_shared<ASTNode>(ASTNodeType::Statement);
    emptyStmt->line = node->line;
    emptyStmt->column = node->column;
    emptyStmt->name = "";
    node->children.push_back(emptyStmt);
        advance(); 
    }
    
    
    if (peek().type != TokenType::SemiColon) {
        node->children.push_back(parseExpression());
    } else {
    auto emptyExpr = std::make_shared<ASTNode>(ASTNodeType::Expression);
    emptyExpr->line = node->line;
    emptyExpr->column = node->column;
    emptyExpr->name = "";
    node->children.push_back(emptyExpr);
    }
    expectToken(TokenType::SemiColon, "Expected ';' after for condition");
    
    
    if (peek().type != TokenType::ParenClose) {
        node->children.push_back(parseExpression());
    } else {
    auto emptyExpr2 = std::make_shared<ASTNode>(ASTNodeType::Expression);
    emptyExpr2->line = node->line;
    emptyExpr2->column = node->column;
    emptyExpr2->name = "";
    node->children.push_back(emptyExpr2);
    }
    
    expectToken(TokenType::ParenClose, "Expected ')' after for clauses");
    
    
    node->children.push_back(parseStatement());
    
    return node;
}

std::shared_ptr<ASTNode> Parser::parseWhile() {
    auto node = std::make_shared<ASTNode>(ASTNodeType::While);
    node->line = peek().line;
    node->column = peek().column;
    
    expectToken(TokenType::Keyword, "Expected 'while' keyword");
    expectToken(TokenType::ParenOpen, "Expected '(' after 'while'");
    
    
    node->children.push_back(parseExpression());
    
    expectToken(TokenType::ParenClose, "Expected ')' after while condition");
    
    
    node->children.push_back(parseStatement());
    
    return node;
}

std::shared_ptr<ASTNode> Parser::parseReturn() {
    auto node = std::make_shared<ASTNode>(ASTNodeType::Return);
    node->line = peek().line;
    node->column = peek().column;
    
    expectToken(TokenType::Keyword, "Expected 'return' keyword");
    
    
    if (peek().type != TokenType::SemiColon && !eof()) {
        node->children.push_back(parseExpression());
    }
    
    if (peek().type == TokenType::SemiColon) {
        advance();
    }
    
    return node;
}

std::shared_ptr<ASTNode> Parser::parseRender() {
    auto node = std::make_shared<ASTNode>(ASTNodeType::Render);
    node->line = peek().line;
    node->column = peek().column;
    
    expectToken(TokenType::Keyword, "Expected 'render' keyword");
    
    
    node->children.push_back(parseExpression());
    
    return node;
}

std::shared_ptr<ASTNode> Parser::parseTag() {
    return parseUIElement();
}

std::shared_ptr<ASTNode> Parser::parseAsyncBlock() {
    auto node = std::make_shared<ASTNode>(ASTNodeType::AsyncBlock);
    node->line = peek().line;
    node->column = peek().column;
    
    expectToken(TokenType::Keyword, "Expected 'async' keyword");
    
    
    if (peek().type == TokenType::BraceOpen) {
        node->children.push_back(parseBlock());
    } else {
        node->children.push_back(parseStatement());
    }
    
    return node;
}

std::shared_ptr<ASTNode> Parser::parseTryBlock() {
    auto node = std::make_shared<ASTNode>(ASTNodeType::TryBlock);
    
    expectToken(TokenType::Keyword, "Expected 'try' keyword");
    
    
    node->children.push_back(parseBlock());
    
    
    while (peek().type == TokenType::Keyword && peek().value == "catch") {
        node->children.push_back(parseCatchBlock());
    }
    
    
    if (peek().type == TokenType::Keyword && peek().value == "finally") {
        node->children.push_back(parseFinallyBlock());
    }
    
    return node;
}

std::shared_ptr<ASTNode> Parser::parseCatchBlock() {
    auto node = std::make_shared<ASTNode>(ASTNodeType::CatchBlock);
    
    expectToken(TokenType::Keyword, "Expected 'catch' keyword");
    
    
    if (match(TokenType::ParenOpen)) {
        if (peek().type == TokenType::Identifier) {
            node->name = peek().value;
            advance();
        }
        expectToken(TokenType::ParenClose, "Expected ')' after catch parameter");
    }
    
    
    node->children.push_back(parseBlock());
    
    return node;
}

std::shared_ptr<ASTNode> Parser::parseFinallyBlock() {
    auto node = std::make_shared<ASTNode>(ASTNodeType::FinallyBlock);
    
    expectToken(TokenType::Keyword, "Expected 'finally' keyword");
    
    
    node->children.push_back(parseBlock());
    
    return node;
}

std::shared_ptr<ASTNode> Parser::parseUIElement() {
    auto node = std::make_shared<ASTNode>(ASTNodeType::UIElement);
    
    
    if (peek().type == TokenType::Operator && peek().value == "<") {
        advance();
        
        if (peek().type == TokenType::Identifier) {
            node->name = peek().value;
            advance();
        } else {
            throw std::runtime_error("Expected element name after '<'");
        }
        
        
        while (!eof() && peek().type == TokenType::Identifier) {
            node->children.push_back(parseUIAttribute());
        }
        
        
        if (peek().type == TokenType::Operator && peek().value == "/") {
            advance();
            expectToken(TokenType::Operator, "Expected '>' after '/'");
            
            return node;
        }
        
        
        if (peek().type == TokenType::Operator && peek().value == ">") {
            advance();
        } else {
            throw std::runtime_error("Expected '>' after element name");
        }
        
        
        size_t stuckCount = 0;
        size_t lastPos = position;
        while (!eof()) {
            if (peek().type == TokenType::Operator && peek().value == "<") {
                size_t saved_pos = position;
                advance();
                if (peek().type == TokenType::Operator && peek().value == "/") {
                    advance();
                    if (peek().type == TokenType::Identifier && peek().value == node->name) {
                        advance();
                        expectToken(TokenType::Operator, "Expected '>' after closing tag");
                        break;
                    }
                }
                position = saved_pos;
                node->children.push_back(parseUIElement());
            } else {
                size_t exprStart = position;
                node->children.push_back(parseExpression());
                
                if (position == exprStart && !eof()) {
                    advance();
                }
                break;
            }
            
            if (position == lastPos) {
                stuckCount++;
                if (stuckCount > 1000) break;
            } else {
                stuckCount = 0;
                lastPos = position;
            }
        }
    }
    
    return node;
}

std::shared_ptr<ASTNode> Parser::parseUIAttribute() {
    auto node = std::make_shared<ASTNode>(ASTNodeType::UIAttribute);
    node->line = peek().line;
    node->column = peek().column;
    node->name = "";
    
    if (peek().type == TokenType::Identifier) {
        node->name = peek().value;
        advance();
        
        if (peek().type == TokenType::Equals) {
            advance();
            
            if (peek().type == TokenType::String) {
                node->value = peek().value;
                advance();
            } else if (peek().type == TokenType::BraceOpen) {
                
                node->children.push_back(parseValueBinding());
            } else {
                throw std::runtime_error("Expected string or value binding after '='");
            }
        }
    }
    
    return node;
}

std::shared_ptr<ASTNode> Parser::parseModifier() {
    auto node = std::make_shared<ASTNode>(ASTNodeType::Modifier);
    node->line = peek().line;
    node->column = peek().column;
    node->name = "";
    
    if (peek().type == TokenType::Identifier) {
        node->name = peek().value;
        advance();
        
        
        if (match(TokenType::ParenOpen)) {
            while (!eof() && peek().type != TokenType::ParenClose) {
                node->children.push_back(parseExpression());
                
                if (peek().type == TokenType::Comma) {
                    advance();
                } else if (peek().type != TokenType::ParenClose) {
                    throw std::runtime_error("Expected ',' or ')' in modifier parameters");
                }
            }
            expectToken(TokenType::ParenClose, "Expected ')' after modifier parameters");
        }
    }
    
    return node;
}

std::shared_ptr<ASTNode> Parser::parseValueBinding() {
    auto node = std::make_shared<ASTNode>(ASTNodeType::ValueBinding);
    node->line = peek().line;
    node->column = peek().column;
    
    expectToken(TokenType::BraceOpen, "Expected '{' for value binding");
    
    node->children.push_back(parseExpression());
    
    expectToken(TokenType::BraceClose, "Expected '}' after value binding");
    
    return node;
}

std::shared_ptr<ASTNode> Parser::parseError(const std::string& message) {
    auto node = std::make_shared<ASTNode>(ASTNodeType::Error);
    node->error = message;
    
    if (!eof()) {
        node->line = peek().line;
        node->column = peek().column;
    }
    return node;
}

void Parser::skipToNextStatement() {
    bool advanced = false;
    while (!eof() && 
           peek().type != TokenType::SemiColon && 
           peek().type != TokenType::BraceOpen &&
           peek().type != TokenType::BraceClose &&
           !(peek().type == TokenType::Keyword && 
             (peek().value == "if" || peek().value == "for" || peek().value == "while" ||
              peek().value == "function" || peek().value == "component" || peek().value == "return"))) {
        advance();
        advanced = true;
    }
    if (!advanced && !eof()) {
        advance(); 
    }
    if (peek().type == TokenType::SemiColon) {
        advance();
    }
}

std::shared_ptr<ASTNode> Parser::parseBracketBlock(bool allowExpressions) {
    auto node = std::make_shared<ASTNode>(allowExpressions ? ASTNodeType::Expression : ASTNodeType::Block);
    node->line = peek().line;
    node->column = peek().column;
    expectToken(TokenType::SquareBracketOpen, "Expected '[' to start block or array");
    if (allowExpressions) {
        
        while (!eof() && peek().type != TokenType::SquareBracketClose) {
            node->children.push_back(parseExpression());
            if (peek().type == TokenType::Comma) {
                advance();
            } else if (peek().type != TokenType::SquareBracketClose) {
                throw std::runtime_error("Expected ',' or ']' in array literal");
            }
        }
    } else {
        
        while (!eof() && peek().type != TokenType::SquareBracketClose) {
            try {
                node->children.push_back(parseStatement());
            } catch (const std::exception& e) {
                node->children.push_back(parseError(e.what()));
                skipToNextStatement();
            }
        }
    }
    expectToken(TokenType::SquareBracketClose, "Expected ']' to end block or array");
    return node;
}