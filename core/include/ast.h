#pragma once
#include <string>
#include <vector>
#include <memory>
#include <optional>

// AST Node Types
enum class ASTNodeType {
    Program,
    Component,
    Function,
    Variable,
    Assignment,
    Expression,
    Statement,
    If,
    For,
    While,
    Return,
    Block,
    Call,
    Literal,
    Identifier,
    Tag,
    Attribute,
    Render,
    Error
};

struct ASTNode {
    ASTNodeType type;
    std::string name;
    std::vector<std::shared_ptr<ASTNode>> children;
    std::optional<std::string> value;
    size_t line = 0;
    size_t column = 0;
    std::optional<std::string> error;

    ASTNode(ASTNodeType t, const std::string& n = "", size_t l = 0, size_t c = 0)
        : type(t), name(n), line(l), column(c) {}
};

// Utility for JSON serialization
std::string astToJson(const std::shared_ptr<ASTNode>& node, int indent = 0);
