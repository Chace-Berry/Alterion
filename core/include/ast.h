#pragma once
#include <string>
#include <vector>
#include <memory>
#include <optional>


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
    FunctionCall,  
    Literal,
    Identifier,
    Tag,
    Attribute,
    Render,
    Error,
    AsyncBlock,
    TryBlock,
    CatchBlock,
    FinallyBlock,
    UIElement,
    UIAttribute,
    Modifier,
    ValueBinding
};

struct ASTNode {
    ASTNodeType type;
    std::string name;
    std::vector<std::shared_ptr<ASTNode>> children;
    std::optional<std::string> value;
    std::optional<std::string> variableType;  
    size_t line = 0;
    size_t column = 0;
    std::optional<std::string> error;
    std::optional<std::string> decorator; 

    ASTNode(ASTNodeType t, const std::string& n = "", size_t l = 0, size_t c = 0)
        : type(t), name(n), line(l), column(c) {}
};


std::string astToJson(const std::shared_ptr<ASTNode>& node, int indent = 0);