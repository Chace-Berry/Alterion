class Property {
public:
    ExpressionPtr key;
    ExpressionPtr value;
    bool computed;
};
#pragma once
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <variant>
#include <optional>
#include "token.h"

// Forward declarations
class ASTNode;
class Expression;
class Statement;
class Component;
class Tag;
class Function;
class ControlFlow;
class Import;
class Export;

// Smart pointer aliases
using ASTNodePtr = std::unique_ptr<ASTNode>;
using ExpressionPtr = std::unique_ptr<Expression>;
using StatementPtr = std::unique_ptr<Statement>;
using ComponentPtr = std::unique_ptr<Component>;
using TagPtr = std::unique_ptr<Tag>;
using FunctionPtr = std::unique_ptr<Function>;
using ControlFlowPtr = std::unique_ptr<ControlFlow>;

// Enums for different node types
enum class ComponentType {
    ALTX,     // UI components with JSX-like syntax
    ALTS,     // Script components  
    MIXED     // Components with both UI and script
};

enum class FunctionType {
    REGULAR,
    ASYNC,
    GENERATOR,
    ASYNC_GENERATOR
};

enum class ControlFlowType {
    IF,
    WHILE,
    FOR,
    FOR_IN,
    MATCH,
    TRY_CATCH,
    ASYNC_BLOCK
};

enum class ExpressionType {
    LITERAL,
    IDENTIFIER,
    BINARY,
    UNARY,
    CALL,
    MEMBER_ACCESS,
    ARRAY_ACCESS,
    CONDITIONAL
};

// Base AST Node
class ASTNode {
public:
    std::vector<std::shared_ptr<ASTNode>> children;
    size_t line, column;
    
    ASTNode(size_t l = 0, size_t c = 0) : line(l), column(c) {}
    virtual ~ASTNode() = default;
    virtual std::string getNodeType() const = 0;
    virtual std::string toString() const = 0;
    virtual void accept(class ASTVisitor& visitor) = 0;
};

// Base Expression class
class Expression : public ASTNode {
public:
    ExpressionType exprType;
    std::optional<std::string> typeAnnotation;
    
    Expression(ExpressionType type, size_t l = 0, size_t c = 0) 
        : ASTNode(l, c), exprType(type) {}
    virtual ~Expression() = default;
};

// Base Statement class
class Statement : public ASTNode {
public:
    Statement(size_t l = 0, size_t c = 0) : ASTNode(l, c) {}
    virtual ~Statement() = default;
};

// Literal Expressions
class StringLiteral : public Expression {
public:
    std::string value;
    
    StringLiteral(const std::string& val, size_t l = 0, size_t c = 0) 
        : Expression(ExpressionType::LITERAL, l, c), value(val) {}
    
    std::string getNodeType() const override { return "StringLiteral"; }
    std::string toString() const override { return "\"" + value + "\""; }
    void accept(class ASTVisitor& visitor) override;
};

class NumberLiteral : public Expression {
public:
    std::string value;
    bool isFloat;
    
    NumberLiteral(const std::string& val, bool isFloat = false, size_t l = 0, size_t c = 0)
        : Expression(ExpressionType::LITERAL, l, c), value(val), isFloat(isFloat) {}
    
    std::string getNodeType() const override { return "NumberLiteral"; }
    std::string toString() const override { return value; }
    void accept(class ASTVisitor& visitor) override;
};

class BooleanLiteral : public Expression {
public:
    bool value;
    
    BooleanLiteral(bool val, size_t l = 0, size_t c = 0)
        : Expression(ExpressionType::LITERAL, l, c), value(val) {}
    
    std::string getNodeType() const override { return "BooleanLiteral"; }
    std::string toString() const override { return value ? "true" : "false"; }
    void accept(class ASTVisitor& visitor) override;
};

class NullLiteral : public Expression {
public:
    NullLiteral(size_t l = 0, size_t c = 0) : Expression(ExpressionType::LITERAL, l, c) {}
    
    std::string getNodeType() const override { return "NullLiteral"; }
    std::string toString() const override { return "null"; }
    void accept(class ASTVisitor& visitor) override;
};

// Identifier and Variable References
class Identifier : public Expression {
public:
    std::string name;
    
    Identifier(const std::string& n, size_t l = 0, size_t c = 0)
        : Expression(ExpressionType::IDENTIFIER, l, c), name(n) {}
    
    std::string getNodeType() const override { return "Identifier"; }
    std::string toString() const override { return name; }
    void accept(class ASTVisitor& visitor) override;
};

class ValueBinding : public Expression {
public:
    std::string bindingName;
    
    ValueBinding(const std::string& name, size_t l = 0, size_t c = 0)
        : Expression(ExpressionType::IDENTIFIER, l, c), bindingName(name) {}
    
    std::string getNodeType() const override { return "ValueBinding"; }
    std::string toString() const override { return "!" + bindingName; }
    void accept(class ASTVisitor& visitor) override;
};

// Binary and Unary Expressions
class BinaryExpression : public Expression {
public:
    ExpressionPtr left;
    std::string operator_;
    ExpressionPtr right;
    
    BinaryExpression(ExpressionPtr l, const std::string& op, ExpressionPtr r, size_t line = 0, size_t col = 0)
        : Expression(ExpressionType::BINARY, line, col), left(std::move(l)), operator_(op), right(std::move(r)) {}
    
    std::string getNodeType() const override { return "BinaryExpression"; }
    std::string toString() const override { 
        return "(" + left->toString() + " " + operator_ + " " + right->toString() + ")"; 
    }
    void accept(class ASTVisitor& visitor) override;
};

class UnaryExpression : public Expression {
public:
    std::string operator_;
    ExpressionPtr operand;
    bool isPrefix;
    
    UnaryExpression(const std::string& op, ExpressionPtr operand, bool prefix = true, size_t l = 0, size_t c = 0)
        : Expression(ExpressionType::UNARY, l, c), operator_(op), operand(std::move(operand)), isPrefix(prefix) {}
    
    std::string getNodeType() const override { return "UnaryExpression"; }
    std::string toString() const override { 
        return isPrefix ? ("(" + operator_ + operand->toString() + ")") 
                        : ("(" + operand->toString() + operator_ + ")"); 
    }
    void accept(class ASTVisitor& visitor) override;
};

// Function Calls and Member Access
class CallExpression : public Expression {
public:
    ExpressionPtr callee;
    std::vector<ExpressionPtr> arguments;
    
    CallExpression(ExpressionPtr callee, std::vector<ExpressionPtr> args, size_t l = 0, size_t c = 0)
        : Expression(ExpressionType::CALL, l, c), callee(std::move(callee)), arguments(std::move(args)) {}
    
    std::string getNodeType() const override { return "CallExpression"; }
    std::string toString() const override;
    void accept(class ASTVisitor& visitor) override;
};

class MemberExpression : public Expression {
public:
    ExpressionPtr object;
    ExpressionPtr property;
    bool computed; // true for obj[prop], false for obj.prop
    
    MemberExpression(ExpressionPtr obj, ExpressionPtr prop, bool computed, size_t l = 0, size_t c = 0)
        : Expression(ExpressionType::MEMBER_ACCESS, l, c), object(std::move(obj)), 
          property(std::move(prop)), computed(computed) {}
    
    std::string getNodeType() const override { return "MemberExpression"; }
    std::string toString() const override;
    void accept(class ASTVisitor& visitor) override;
};

// Array and Object Expressions
class ArrayExpression : public Expression {
public:
    std::vector<ExpressionPtr> elements;
    
    ArrayExpression(std::vector<ExpressionPtr> elements, size_t l = 0, size_t c = 0)
        : Expression(ExpressionType::LITERAL, l, c), elements(std::move(elements)) {}
    
    std::string getNodeType() const override { return "ArrayExpression"; }
    std::string toString() const override;
    void accept(class ASTVisitor& visitor) override;
};

class ObjectProperty {
public:
    ExpressionPtr key;
    ExpressionPtr value;
    bool computed;
    
    ObjectProperty(ExpressionPtr k, ExpressionPtr v, bool comp = false)
        : key(std::move(k)), value(std::move(v)), computed(comp) {}
};

class ObjectExpression : public Expression {
public:
    std::vector<std::unique_ptr<ObjectProperty>> properties;
    
    ObjectExpression(std::vector<std::unique_ptr<ObjectProperty>> props, size_t l = 0, size_t c = 0)
        : Expression(ExpressionType::LITERAL, l, c), properties(std::move(props)) {}
    
    std::string getNodeType() const override { return "ObjectExpression"; }
    std::string toString() const override;
    void accept(class ASTVisitor& visitor) override;
};

// Conditional Expression (ternary operator)
class ConditionalExpression : public Expression {
public:
    ExpressionPtr test;
    ExpressionPtr consequent;
    ExpressionPtr alternate;
    
    ConditionalExpression(ExpressionPtr test, ExpressionPtr consequent, ExpressionPtr alternate, size_t l = 0, size_t c = 0)
        : Expression(ExpressionType::CONDITIONAL, l, c), test(std::move(test)), 
          consequent(std::move(consequent)), alternate(std::move(alternate)) {}
    
    std::string getNodeType() const override { return "ConditionalExpression"; }
    std::string toString() const override;
    void accept(class ASTVisitor& visitor) override;
};

// Assignment and Variable Declaration
class Assignment : public Statement {
public:
    std::string target;
    ExpressionPtr value;
    std::string operator_; // =, +=, -=, etc.
    
    Assignment(const std::string& target, ExpressionPtr value, const std::string& op = "=", size_t l = 0, size_t c = 0)
        : Statement(l, c), target(target), value(std::move(value)), operator_(op) {}
    
    std::string getNodeType() const override { return "Assignment"; }
    std::string toString() const override;
    void accept(class ASTVisitor& visitor) override;
};

class VariableDeclaration : public Statement {
public:
    std::string name;
    ExpressionPtr initializer;
    std::string kind; // let, const, var
    std::optional<std::string> typeAnnotation;
    
    VariableDeclaration(const std::string& name, ExpressionPtr init, const std::string& kind = "let", size_t l = 0, size_t c = 0)
        : Statement(l, c), name(name), initializer(std::move(init)), kind(kind) {}
    
    std::string getNodeType() const override { return "VariableDeclaration"; }
    std::string toString() const override;
    void accept(class ASTVisitor& visitor) override;
};

// Expression Statement
class ExpressionStatement : public Statement {
public:
    ExpressionPtr expression;
    
    ExpressionStatement(ExpressionPtr expr, size_t l = 0, size_t c = 0)
        : Statement(l, c), expression(std::move(expr)) {}
    
    std::string getNodeType() const override { return "ExpressionStatement"; }
    std::string toString() const override { return expression->toString() + ";"; }
    void accept(class ASTVisitor& visitor) override;
};

// Block Statement
class BlockStatement : public Statement {
public:
    std::vector<StatementPtr> statements;
    
    BlockStatement(std::vector<StatementPtr> stmts, size_t l = 0, size_t c = 0)
        : Statement(l, c), statements(std::move(stmts)) {}
    
    std::string getNodeType() const override { return "BlockStatement"; }
    std::string toString() const override;
    void accept(class ASTVisitor& visitor) override;
};

// Control Flow Statements
class IfStatement : public Statement {
public:
    ExpressionPtr test;
    StatementPtr consequent;
    StatementPtr alternate; // optional else clause
    
    IfStatement(ExpressionPtr test, StatementPtr consequent, StatementPtr alternate = nullptr, size_t l = 0, size_t c = 0)
        : Statement(l, c), test(std::move(test)), consequent(std::move(consequent)), alternate(std::move(alternate)) {}
    
    std::string getNodeType() const override { return "IfStatement"; }
    std::string toString() const override;
    void accept(class ASTVisitor& visitor) override;
};

class WhileStatement : public Statement {
public:
    ExpressionPtr test;
    StatementPtr body;
    
    WhileStatement(ExpressionPtr test, StatementPtr body, size_t l = 0, size_t c = 0)
        : Statement(l, c), test(std::move(test)), body(std::move(body)) {}
    
    std::string getNodeType() const override { return "WhileStatement"; }
    std::string toString() const override;
    void accept(class ASTVisitor& visitor) override;
};

class ForStatement : public Statement {
public:
    StatementPtr init;     // optional
    ExpressionPtr test;    // optional  
    ExpressionPtr update;  // optional
    StatementPtr body;
    
    ForStatement(StatementPtr init, ExpressionPtr test, ExpressionPtr update, StatementPtr body, size_t l = 0, size_t c = 0)
        : Statement(l, c), init(std::move(init)), test(std::move(test)), update(std::move(update)), body(std::move(body)) {}
    
    std::string getNodeType() const override { return "ForStatement"; }
    std::string toString() const override;
    void accept(class ASTVisitor& visitor) override;
};

class ForInStatement : public Statement {
public:
    std::string variable;
    ExpressionPtr iterable;
    StatementPtr body;
    
    ForInStatement(const std::string& var, ExpressionPtr iterable, StatementPtr body, size_t l = 0, size_t c = 0)
        : Statement(l, c), variable(var), iterable(std::move(iterable)), body(std::move(body)) {}
    
    std::string getNodeType() const override { return "ForInStatement"; }
    std::string toString() const override;
    void accept(class ASTVisitor& visitor) override;
};

// Return, Break, Continue
class ReturnStatement : public Statement {
public:
    ExpressionPtr argument; // optional
    
    ReturnStatement(ExpressionPtr arg = nullptr, size_t l = 0, size_t c = 0)
        : Statement(l, c), argument(std::move(arg)) {}
    
    std::string getNodeType() const override { return "ReturnStatement"; }
    std::string toString() const override;
    void accept(class ASTVisitor& visitor) override;
};

class BreakStatement : public Statement {
public:
    BreakStatement(size_t l = 0, size_t c = 0) : Statement(l, c) {}
    
    std::string getNodeType() const override { return "BreakStatement"; }
    std::string toString() const override { return "break;"; }
    void accept(class ASTVisitor& visitor) override;
};

class ContinueStatement : public Statement {
public:
    ContinueStatement(size_t l = 0, size_t c = 0) : Statement(l, c) {}
    
    std::string getNodeType() const override { return "ContinueStatement"; }
    std::string toString() const override { return "continue;"; }
    void accept(class ASTVisitor& visitor) override;
};

// Function Definition
class Function : public Statement {
public:
    std::string name;
    std::vector<std::string> parameters;
    std::vector<std::string> parameterTypes; // optional type annotations
    StatementPtr body;
    FunctionType funcType;
    std::optional<std::string> returnType;
    std::vector<std::string> modifiers; // @async, @unsafe, etc.
    
    Function(const std::string& name, std::vector<std::string> params, StatementPtr body, 
             FunctionType type = FunctionType::REGULAR, size_t l = 0, size_t c = 0)
        : Statement(l, c), name(name), parameters(std::move(params)), body(std::move(body)), funcType(type) {}
    
    std::string getNodeType() const override { return "Function"; }
    std::string toString() const override;
    void accept(class ASTVisitor& visitor) override;
};

// Import and Export
class Import : public Statement {
public:
    std::vector<std::string> bindings;
    std::string source;
    bool isDefault;
    
    Import(std::vector<std::string> bindings, const std::string& source, bool isDefault = false, size_t l = 0, size_t c = 0)
        : Statement(l, c), bindings(std::move(bindings)), source(source), isDefault(isDefault) {}
    
    std::string getNodeType() const override { return "Import"; }
    std::string toString() const override;
    void accept(class ASTVisitor& visitor) override;
};

class Export : public Statement {
public:
    StatementPtr declaration;
    std::vector<std::string> bindings;
    std::string source; // for re-exports
    bool isDefault;
    
    Export(StatementPtr decl, bool isDefault = false, size_t l = 0, size_t c = 0)
        : Statement(l, c), declaration(std::move(decl)), isDefault(isDefault) {}
    
    std::string getNodeType() const override { return "Export"; }
    std::string toString() const override;
    void accept(class ASTVisitor& visitor) override;
};

// ALTX/JSX-like structures
class Attribute {
public:
    std::string name;
    ExpressionPtr value; // can be null for boolean attributes
    
    Attribute(const std::string& name, ExpressionPtr value = nullptr)
        : name(name), value(std::move(value)) {}
    
    std::string toString() const;
};

class StyleProperty {
public:
    std::string property;
    std::string value;
    
    StyleProperty(const std::string& prop, const std::string& val)
        : property(prop), value(val) {}
};

class Tag : public ASTNode {
public:
    std::string tagName;
    std::vector<std::unique_ptr<Attribute>> attributes;
    std::vector<ASTNodePtr> children;
    std::vector<StyleProperty> styles;
    bool isSelfClosing;
    
    Tag(const std::string& name, size_t l = 0, size_t c = 0)
        : ASTNode(l, c), tagName(name), isSelfClosing(false) {}
    
    std::string getNodeType() const override { return "Tag"; }
    std::string toString() const override;
    void accept(class ASTVisitor& visitor) override;
};

class TextContent : public ASTNode {
public:
    std::string content;
    
    TextContent(const std::string& content, size_t l = 0, size_t c = 0)
        : ASTNode(l, c), content(content) {}
    
    std::string getNodeType() const override { return "TextContent"; }
    std::string toString() const override { return content; }
    void accept(class ASTVisitor& visitor) override;
};

// Component Definition
class Component : public Statement {
public:
    std::string name;
    ComponentType type;
    std::vector<StatementPtr> statements; // state, methods, etc.
    std::vector<ASTNodePtr> body;         // ALTX content
    std::vector<std::string> modifiers;   // @component, etc.
    
    Component(const std::string& name, ComponentType type, size_t l = 0, size_t c = 0)
        : Statement(l, c), name(name), type(type) {}
    
    std::string getNodeType() const override { return "Component"; }
    std::string toString() const override;
    void accept(class ASTVisitor& visitor) override;
};

// Program (root node)
class Program : public ASTNode {
public:
    std::vector<StatementPtr> globalStatements;
    std::vector<ComponentPtr> components;
    std::vector<FunctionPtr> functions;
    
    Program() : ASTNode(0, 0) {}
    
    std::string getNodeType() const override { return "Program"; }
    std::string toString() const override;
    void accept(class ASTVisitor& visitor) override;
};

// Async Block (special Alterion feature)
class AsyncBlock : public Statement {
public:
    StatementPtr tryBlock;
    StatementPtr catchBlock; // optional
    StatementPtr finallyBlock; // optional
    
    AsyncBlock(StatementPtr tryBlock, StatementPtr catchBlock = nullptr, StatementPtr finallyBlock = nullptr, size_t l = 0, size_t c = 0)
        : Statement(l, c), tryBlock(std::move(tryBlock)), catchBlock(std::move(catchBlock)), finallyBlock(std::move(finallyBlock)) {}
    
    std::string getNodeType() const override { return "AsyncBlock"; }
    std::string toString() const override;
    void accept(class ASTVisitor& visitor) override;
};

// Try-Catch-Finally
class TryStatement : public Statement {
public:
    StatementPtr block;
    std::string catchVariable; // optional
    StatementPtr catchBlock;   // optional
    StatementPtr finallyBlock; // optional
    
    TryStatement(StatementPtr block, size_t l = 0, size_t c = 0)
        : Statement(l, c), block(std::move(block)) {}
    
    std::string getNodeType() const override { return "TryStatement"; }
    std::string toString() const override;
    void accept(class ASTVisitor& visitor) override;
};

class ThrowStatement : public Statement {
public:
    ExpressionPtr argument;
    
    ThrowStatement(ExpressionPtr arg, size_t l = 0, size_t c = 0)
        : Statement(l, c), argument(std::move(arg)) {}
    
    std::string getNodeType() const override { return "ThrowStatement"; }
    std::string toString() const override;
    void accept(class ASTVisitor& visitor) override;
};

// Visitor Pattern for AST traversal
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    
    // Expressions
    virtual void visit(StringLiteral& node) = 0;
    virtual void visit(NumberLiteral& node) = 0;
    virtual void visit(BooleanLiteral& node) = 0;
    virtual void visit(NullLiteral& node) = 0;
    virtual void visit(Identifier& node) = 0;
    virtual void visit(ValueBinding& node) = 0;
    virtual void visit(BinaryExpression& node) = 0;
    virtual void visit(UnaryExpression& node) = 0;
    virtual void visit(CallExpression& node) = 0;
    virtual void visit(MemberExpression& node) = 0;
    virtual void visit(ArrayExpression& node) = 0;
    virtual void visit(ObjectExpression& node) = 0;
    virtual void visit(ConditionalExpression& node) = 0;
    
    // Statements
    virtual void visit(Assignment& node) = 0;
    virtual void visit(VariableDeclaration& node) = 0;
    virtual void visit(ExpressionStatement& node) = 0;
    virtual void visit(BlockStatement& node) = 0;
    virtual void visit(IfStatement& node) = 0;
    virtual void visit(WhileStatement& node) = 0;
    virtual void visit(ForStatement& node) = 0;
    virtual void visit(ForInStatement& node) = 0;
    virtual void visit(ReturnStatement& node) = 0;
    virtual void visit(BreakStatement& node) = 0;
    virtual void visit(ContinueStatement& node) = 0;
    virtual void visit(Function& node) = 0;
    virtual void visit(Import& node) = 0;
    virtual void visit(Export& node) = 0;
    virtual void visit(Component& node) = 0;
    virtual void visit(AsyncBlock& node) = 0;
    virtual void visit(TryStatement& node) = 0;
    virtual void visit(ThrowStatement& node) = 0;
    
    // ALTX
    virtual void visit(Tag& node) = 0;
    virtual void visit(TextContent& node) = 0;
    
    // Program
    virtual void visit(Program& node) = 0;
};

// The ParseError class is defined in parser.h and should be removed from here.
