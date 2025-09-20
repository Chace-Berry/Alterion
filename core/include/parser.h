#pragma once
#include "token.h"
#include "ast.h"
#include <vector>
#include <memory>
#include <string>

class Parser {
public:
	Parser(const std::vector<Token>& tokens);
	std::shared_ptr<ASTNode> parse(); 

private:
	const std::vector<Token>& tokens;
	size_t position;

	
	const Token& peek() const;
	const Token& advance();
	bool match(TokenType type);
	bool eof() const;
	void expectToken(TokenType type, const std::string& message);
	int getOperatorPrecedence(const std::string& op);
	void skipToNextStatement();

	
	std::shared_ptr<ASTNode> parseProgram();
	std::shared_ptr<ASTNode> parseComponent();
	std::shared_ptr<ASTNode> parseFunction();
	std::shared_ptr<ASTNode> parseVariable();
	std::shared_ptr<ASTNode> parseAssignment();
	std::shared_ptr<ASTNode> parseStatement();
	std::shared_ptr<ASTNode> parseExpression();
	std::shared_ptr<ASTNode> parseBinaryExpression(int minPrecedence);
	std::shared_ptr<ASTNode> parsePrimaryExpression();
    std::shared_ptr<ASTNode> parseStatement(bool inBlockContext);
    std::shared_ptr<ASTNode> parsePrimaryExpression(bool inBlockContext);
	std::shared_ptr<ASTNode> parseBlock();
	std::shared_ptr<ASTNode> parseIf();
	std::shared_ptr<ASTNode> parseFor();
	std::shared_ptr<ASTNode> parseWhile();
	std::shared_ptr<ASTNode> parseReturn();
	std::shared_ptr<ASTNode> parseRender();
	std::shared_ptr<ASTNode> parseTag();
	std::shared_ptr<ASTNode> parseAsyncBlock();
	std::shared_ptr<ASTNode> parseTryBlock();
	std::shared_ptr<ASTNode> parseCatchBlock();
	std::shared_ptr<ASTNode> parseFinallyBlock();
	std::shared_ptr<ASTNode> parseUIElement();
	std::shared_ptr<ASTNode> parseUIAttribute();
	std::shared_ptr<ASTNode> parseModifier();
	std::shared_ptr<ASTNode> parseValueBinding();
	std::shared_ptr<ASTNode> parseError(const std::string& message);
	std::shared_ptr<ASTNode> parseBracketBlock(bool allowExpressions = false);
	std::shared_ptr<ASTNode> parseThrowStatement();
	std::shared_ptr<ASTNode> parseAwaitStatement();
	std::shared_ptr<ASTNode> parseArrayLiteral();
	std::shared_ptr<ASTNode> parseObjectLiteral();
	std::shared_ptr<ASTNode> parseFunctionDefinition();
	std::shared_ptr<ASTNode> parseCallExpression(std::shared_ptr<ASTNode> callee);
	std::shared_ptr<ASTNode> parsePostfixExpression(std::shared_ptr<ASTNode> left);
	bool isExpressionTerminator() const;
    std::shared_ptr<ASTNode> parseAsyncBracketBlock();
    std::shared_ptr<ASTNode> parseExpressionWithPrecedence(int minPrecedence);

};