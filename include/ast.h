#pragma once
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <variant>
#include "token.h"


class ASTNode;
class Expression;
class Statement;
class Component;
class Tag;


using ASTNodePtr = std::unique_ptr<ASTNode>;
using ExpressionPtr = std::unique_ptr<Expression>;
using StatementPtr = std::unique_ptr<Statement>;
using ComponentPtr = std::unique_ptr<Component>;
using TagPtr = std::unique_ptr<Tag>;


class ASTNode {
public:
    size_t line, column;
    
    ASTNode(size_t l = 0, size_t c = 0) : line(l), column(c) {}
    virtual ~ASTNode() = default;
    virtual std::string getNodeType() const = 0;
    virtual std::string toString() const = 0;
};


class Expression : public ASTNode {
public:
    Expression(size_t l = 0, size_t c = 0) : ASTNode(l, c) {}
    virtual ~Expression() = default;
};


class Statement : public ASTNode {
public:
    Statement(size_t l = 0, size_t c = 0) : ASTNode(l, c) {}
    virtual ~Statement() = default;
    
};


class StringLiteral : public Expression {
public:
    std::string value;
    
    StringLiteral(const std::string& val, size_t l = 0, size_t c = 0) 
        : Expression(l, c), value(val) {}
    
    std::string getNodeType() const override { return "StringLiteral"; }
    std::string toString() const override { return "\"" + value + "\""; }
};

class NumberLiteral : public Expression {
public:
    std::string value; 
    
    NumberLiteral(const std::string& val, size_t l = 0, size_t c = 0)
        : Expression(l, c), value(val) {}
    
    std::string getNodeType() const override { return "NumberLiteral"; }
    std::string toString() const override { return value; }
};

class BooleanLiteral : public Expression {
public:
    bool value;
    
    BooleanLiteral(bool val, size_t l = 0, size_t c = 0)
        : Expression(l, c), value(val) {}
    
    std::string getNodeType() const override { return "BooleanLiteral"; }
    std::string toString() const override { return value ? "true" : "false"; }
};


class Identifier : public Expression {
public:
    std::string name;
    
    Identifier(const std::string& n, size_t l = 0, size_t c = 0)
        : Expression(l, c), name(n) {}
    
    std::string getNodeType() const override { return "Identifier"; }
    std::string toString() const override { return name; }
};


class ValueBinding : public Expression {
public:
    std::string bindingName; 
    
    ValueBinding(const std::string& name, size_t l = 0, size_t c = 0)
        : Expression(l, c), bindingName(name) {}
    
    std::string getNodeType() const override { return "ValueBinding"; }
    std::string toString() const override { return "!" + bindingName; }
};


class BinaryExpression : public Expression {
public:
    ExpressionPtr left;
    std::string operator_;
    ExpressionPtr right;
    
    BinaryExpression(ExpressionPtr l, const std::string& op, ExpressionPtr r, size_t line = 0, size_t col = 0)
        : Expression(line, col), left(std::move(l)), operator_(op), right(std::move(r)) {}
    
    std::string getNodeType() const override { return "BinaryExpression"; }
    std::string toString() const override { 
        return "(" + left->toString() + " " + operator_ + " " + right->toString() + ")"; 
    }
};


class UnaryExpression : public Expression {
public:
    std::string operator_;
    ExpressionPtr operand;
    
    UnaryExpression(const std::string& op, ExpressionPtr operand, size_t line = 0, size_t col = 0)
        : Expression(line, col), operator_(op), operand(std::move(operand)) {}
    
    std::string getNodeType() const override { return "UnaryExpression"; }
    std::string toString() const override { 
        return "(" + operator_ + operand->toString() + ")"; 
    }
};


class CallExpression : public Expression {
public:
    ExpressionPtr callee;
    std::vector<ExpressionPtr> arguments;
    
    CallExpression(ExpressionPtr c, std::vector<ExpressionPtr> args, size_t l = 0, size_t col = 0)
        : Expression(l, col), callee(std::move(c)), arguments(std::move(args)) {}
    
    std::string getNodeType() const override { return "CallExpression"; }
    std::string toString() const override { 
        std::string result = callee->toString() + "(";
        for (size_t i = 0; i < arguments.size(); ++i) {
            if (i > 0) result += ", ";
            result += arguments[i]->toString();
        }
        result += ")";
        return result;
    }
};


class Assignment : public Statement {
public:
    std::string identifier;
    ExpressionPtr value;
    
    Assignment(const std::string& id, ExpressionPtr val, size_t l = 0, size_t c = 0)
        : Statement(l, c), identifier(id), value(std::move(val)) {}
    
    std::string getNodeType() const override { return "Assignment"; }
    std::string toString() const override { 
        return identifier + " = " + value->toString(); 
    }
};


class Import : public Statement {
public:
    std::vector<std::string> bindings;  
    std::string fromComponent;          
    
    Import(std::vector<std::string> b, const std::string& from, size_t l = 0, size_t c = 0)
        : Statement(l, c), bindings(std::move(b)), fromComponent(from) {}
    
    std::string getNodeType() const override { return "Import"; }
    std::string toString() const override { 
        std::string result = "import {";
        for (size_t i = 0; i < bindings.size(); ++i) {
            if (i > 0) result += ", ";
            result += bindings[i];
        }
        result += "} from " + fromComponent;
        return result;
    }
};


class StyleProperty {
public:
    std::string property;
    std::string value;
    
    StyleProperty(const std::string& prop, const std::string& val)
        : property(prop), value(val) {}
};


class Attribute {
public:
    std::string name;
    ExpressionPtr value; 
    
    Attribute(const std::string& n, ExpressionPtr val)
        : name(n), value(std::move(val)) {}
};


class Tag : public ASTNode {
public:
    std::string tagName;
    std::vector<std::unique_ptr<Attribute>> attributes;
    std::vector<StyleProperty> styles;
    std::vector<ASTNodePtr> children; 
    bool isSelfClosing;
    
    Tag(const std::string& name, size_t l = 0, size_t c = 0)
        : ASTNode(l, c), tagName(name), isSelfClosing(false) {}
    
    std::string getNodeType() const override { return "Tag"; }
    std::string toString() const override { 
        std::string result = "<" + tagName;
        if (!attributes.empty() || !styles.empty()) {
            result += " [attrs/styles]";
        }
        if (isSelfClosing) {
            result += "/>";
        } else {
            result += ">";
            for (const auto& child : children) {
                result += child->toString();
            }
            result += "</" + tagName + ">";
        }
        return result;
    }
};


class TextContent : public ASTNode {
public:
    std::string content;
    
    TextContent(const std::string& text, size_t l = 0, size_t c = 0)
        : ASTNode(l, c), content(text) {}
    
    std::string getNodeType() const override { return "TextContent"; }
    std::string toString() const override { return content; }
};


class ExpressionStatement : public Statement {
public:
    ExpressionPtr expression;
    
    ExpressionStatement(ExpressionPtr expr, size_t l = 0, size_t c = 0)
        : Statement(l, c), expression(std::move(expr)) {}
    
    std::string getNodeType() const override { return "ExpressionStatement"; }
    std::string toString() const override { return expression->toString(); }
    
};


class PrintStatement : public Statement {
public:
    ExpressionPtr expression;
    
    PrintStatement(ExpressionPtr expr, size_t l = 0, size_t c = 0)
        : Statement(l, c), expression(std::move(expr)) {}
    
    std::string getNodeType() const override { return "PrintStatement"; }
    std::string toString() const override { 
        return "print " + expression->toString(); 
    }
};


enum class ComponentType {
    ALTX,  
    ALTS   
};


class Component : public ASTNode {
public:
    std::string name;
    ComponentType type;
    std::vector<StatementPtr> statements; 
    std::vector<ASTNodePtr> body;         
    
    Component(const std::string& n, ComponentType t, size_t l = 0, size_t c = 0)
        : ASTNode(l, c), name(n), type(t) {}
    
    std::string getNodeType() const override { return "Component"; }
    std::string toString() const override { 
        std::string result = "component " + name + " {";
        for (const auto& stmt : statements) {
            result += " " + stmt->toString() + ";";
        }
        for (const auto& node : body) {
            result += " " + node->toString();
        }
        result += " }";
        return result;
    }
};


class Program : public ASTNode {
public:
    std::vector<ComponentPtr> components;
    std::vector<StatementPtr> globalStatements; 
    
    Program(size_t l = 0, size_t c = 0) : ASTNode(l, c) {}
    
    std::string getNodeType() const override { return "Program"; }
    std::string toString() const override { 
        std::string result = "Program {";
        for (const auto& stmt : globalStatements) {
            result += " " + stmt->toString() + ";";
        }
        for (const auto& comp : components) {
            result += " " + comp->toString();
        }
        result += " }";
        return result;
    }
    std::vector<std::string> toStringVector() const;
};


class VNode {
public:
    std::string type;           
    std::map<std::string, std::string> props;  
    std::map<std::string, std::string> style;  
    std::vector<std::unique_ptr<VNode>> children;
    std::string textContent;    
    
    VNode(const std::string& t) : type(t) {}
    
    
    static std::unique_ptr<VNode> createElement(const std::string& tagName) {
        return std::make_unique<VNode>(tagName);
    }
    
    static std::unique_ptr<VNode> createTextNode(const std::string& text) {
        auto node = std::make_unique<VNode>("text");
        node->textContent = text;
        return node;
    }
    
    void setAttribute(const std::string& name, const std::string& value) {
        props[name] = value;
    }
    
    void setStyle(const std::string& property, const std::string& value) {
        style[property] = value;
    }
    
    void appendChild(std::unique_ptr<VNode> child) {
        children.push_back(std::move(child));
    }

};