#pragma once
#include "type_system.h"
#include "ast_complete.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <optional>

namespace Alterion {

// Type environment for variable and function scoping
class TypeEnvironment {
private:
    std::vector<std::unordered_map<std::string, TypePtr>> scopes;
    
public:
    TypeEnvironment() {
        // Global scope
        pushScope();
    }
    
    void pushScope() {
        scopes.emplace_back();
    }
    
    void popScope() {
        if (scopes.size() > 1) { // Keep global scope
            scopes.pop_back();
        }
    }
    
    void define(const std::string& name, TypePtr type) {
        if (!scopes.empty()) {
            scopes.back()[name] = std::move(type);
        }
    }
    
    std::optional<TypePtr> lookup(const std::string& name) const {
        // Search from innermost to outermost scope
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) {
                return found->second;
            }
        }
        return std::nullopt;
    }
    
    bool exists(const std::string& name) const {
        return lookup(name).has_value();
    }
    
    void update(const std::string& name, TypePtr type) {
        // Update in the scope where it exists
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) {
                found->second = std::move(type);
                return;
            }
        }
        // If not found, define in current scope
        define(name, std::move(type));
    }
};

// Type error reporting
struct TypeError {
    std::string message;
    size_t line, column;
    
    TypeError(const std::string& msg, size_t l = 0, size_t c = 0)
        : message(msg), line(l), column(c) {}
};

// Type checker visitor
class TypeChecker : public ASTVisitor {
private:
    TypeEnvironment env;
    std::vector<TypeError> errors;
    TypePtr currentType; // Result of last visit
    bool strictMode; // Enable strict type checking
    
    // Built-in function types
    std::unordered_map<std::string, TypePtr> builtins;
    
    void initializeBuiltins() {
        // console.log(message: any): void
        auto consoleLogParams = std::vector<TypePtr>{TypeFactory::createAny()};
        auto consoleLogType = TypeFactory::createFunction(
            std::move(consoleLogParams), 
            TypeFactory::createNull()
        );
        builtins["console.log"] = consoleLogType;
        
        // Array.length: Int
        builtins["Array.length"] = TypeFactory::createInt();
        
        // String.length: Int
        builtins["String.length"] = TypeFactory::createInt();
        
        // Math functions
        auto mathParams = std::vector<TypePtr>{TypeFactory::createFloat()};
        auto mathType = TypeFactory::createFunction(
            std::move(mathParams), 
            TypeFactory::createFloat()
        );
        builtins["Math.sqrt"] = mathType;
        builtins["Math.pow"] = TypeFactory::createFunction(
            std::vector<TypePtr>{TypeFactory::createFloat(), TypeFactory::createFloat()},
            TypeFactory::createFloat()
        );
    }
    
    void reportError(const std::string& message, size_t line = 0, size_t column = 0) {
        errors.emplace_back(message, line, column);
    }
    
    TypePtr inferBinaryOperationType(const std::string& operator_, TypePtr left, TypePtr right) {
        // Arithmetic operators
        if (operator_ == "+" || operator_ == "-" || operator_ == "*" || operator_ == "/" || operator_ == "**") {
            if (left->isPrimitive() && right->isPrimitive()) {
                auto leftPrim = std::static_pointer_cast<PrimitiveType>(left);
                auto rightPrim = std::static_pointer_cast<PrimitiveType>(right);
                
                // Special case: string concatenation
                if (operator_ == "+" && 
                    (leftPrim->primitiveKind == PrimitiveType::PrimitiveKind::String ||
                     rightPrim->primitiveKind == PrimitiveType::PrimitiveKind::String)) {
                    return TypeFactory::createString();
                }
                
                // Numeric operations
                if ((leftPrim->primitiveKind == PrimitiveType::PrimitiveKind::Int ||
                     leftPrim->primitiveKind == PrimitiveType::PrimitiveKind::Float) &&
                    (rightPrim->primitiveKind == PrimitiveType::PrimitiveKind::Int ||
                     rightPrim->primitiveKind == PrimitiveType::PrimitiveKind::Float)) {
                    
                    // Float if either operand is float
                    if (leftPrim->primitiveKind == PrimitiveType::PrimitiveKind::Float ||
                        rightPrim->primitiveKind == PrimitiveType::PrimitiveKind::Float) {
                        return TypeFactory::createFloat();
                    }
                    return TypeFactory::createInt();
                }
            }
            reportError("Incompatible types for operator " + operator_);
            return TypeFactory::createUnknown();
        }
        
        // Comparison operators
        if (operator_ == "==" || operator_ == "!=" || operator_ == "<" || 
            operator_ == ">" || operator_ == "<=" || operator_ == ">=") {
            // Type compatibility check
            if (!left->isAssignableTo(right) && !right->isAssignableTo(left)) {
                reportError("Cannot compare incompatible types");
            }
            return TypeFactory::createBool();
        }
        
        // Logical operators
        if (operator_ == "&&" || operator_ == "||") {
            // Both operands should be boolean-like
            return TypeFactory::createBool();
        }
        
        reportError("Unknown binary operator: " + operator_);
        return TypeFactory::createUnknown();
    }
    
    TypePtr inferUnaryOperationType(const std::string& operator_, TypePtr operand) {
        if (operator_ == "!") {
            return TypeFactory::createBool();
        }
        
        if (operator_ == "-" || operator_ == "+") {
            if (operand->isPrimitive()) {
                auto prim = std::static_pointer_cast<PrimitiveType>(operand);
                if (prim->primitiveKind == PrimitiveType::PrimitiveKind::Int ||
                    prim->primitiveKind == PrimitiveType::PrimitiveKind::Float) {
                    return operand;
                }
            }
            reportError("Unary " + operator_ + " requires numeric type");
            return TypeFactory::createUnknown();
        }
        
        reportError("Unknown unary operator: " + operator_);
        return TypeFactory::createUnknown();
    }
    
public:
    TypeChecker(bool strict = false) : strictMode(strict) {
        initializeBuiltins();
        currentType = TypeFactory::createUnknown();
    }
    
    // Get type checking results
    TypePtr getLastType() const { return currentType; }
    const std::vector<TypeError>& getErrors() const { return errors; }
    bool hasErrors() const { return !errors.empty(); }
    
    // Clear previous results
    void reset() {
        errors.clear();
        currentType = TypeFactory::createUnknown();
    }
    
    // Visitor methods
    void visit(StringLiteral& node) override {
        currentType = TypeFactory::createString();
    }
    
    void visit(NumberLiteral& node) override {
        // Determine if it's int or float
        if (node.value.find('.') != std::string::npos) {
            currentType = TypeFactory::createFloat();
        } else {
            currentType = TypeFactory::createInt();
        }
    }
    
    void visit(BooleanLiteral& node) override {
        currentType = TypeFactory::createBool();
    }
    
    void visit(NullLiteral& node) override {
        currentType = TypeFactory::createNull();
    }
    
    void visit(Identifier& node) override {
        auto type = env.lookup(node.name);
        if (type) {
            currentType = *type;
        } else {
            reportError("Undefined variable: " + node.name);
            currentType = TypeFactory::createUnknown();
        }
    }
    
    void visit(ValueBinding& node) override {
        // Value bindings are external references, assume any for now
        currentType = TypeFactory::createAny();
    }
    
    void visit(BinaryExpression& node) override {
        node.left->accept(*this);
        TypePtr leftType = currentType;
        
        node.right->accept(*this);
        TypePtr rightType = currentType;
        
        currentType = inferBinaryOperationType(node.operator_, leftType, rightType);
    }
    
    void visit(UnaryExpression& node) override {
        node.operand->accept(*this);
        TypePtr operandType = currentType;
        
        currentType = inferUnaryOperationType(node.operator_, operandType);
    }
    
    void visit(CallExpression& node) override {
        node.callee->accept(*this);
        TypePtr calleeType = currentType;
        
        if (!calleeType->isFunction()) {
            reportError("Cannot call non-function value");
            currentType = TypeFactory::createUnknown();
            return;
        }
        
        auto funcType = std::static_pointer_cast<FunctionType>(calleeType);
        
        // Check argument count
        if (node.arguments.size() != funcType->parameterTypes.size()) {
            reportError("Function expects " + std::to_string(funcType->parameterTypes.size()) +
                       " arguments, got " + std::to_string(node.arguments.size()));
        }
        
        // Check argument types
        for (size_t i = 0; i < std::min(node.arguments.size(), funcType->parameterTypes.size()); ++i) {
            node.arguments[i]->accept(*this);
            TypePtr argType = currentType;
            
            if (!argType->isAssignableTo(funcType->parameterTypes[i])) {
                reportError("Argument " + std::to_string(i + 1) + " type mismatch");
            }
        }
        
        currentType = funcType->returnType;
    }
    
    void visit(MemberExpression& node) override {
        node.object->accept(*this);
        TypePtr objectType = currentType;
        
        if (objectType->isObject()) {
            auto objType = std::static_pointer_cast<ObjectType>(objectType);
            
            if (node.computed) {
                // obj[key] - check key type and return element type
                node.property->accept(*this);
                // For now, assume valid access
                currentType = TypeFactory::createAny();
            } else {
                // obj.prop - check property exists
                if (auto ident = dynamic_cast<Identifier*>(node.property.get())) {
                    auto it = objType->properties.find(ident->name);
                    if (it != objType->properties.end()) {
                        currentType = it->second;
                    } else {
                        reportError("Property '" + ident->name + "' does not exist");
                        currentType = TypeFactory::createUnknown();
                    }
                }
            }
        } else if (objectType->isArray()) {
            if (node.computed) {
                // arr[index] - return element type
                auto arrType = std::static_pointer_cast<ArrayType>(objectType);
                currentType = arrType->elementType;
            } else {
                // arr.prop - built-in array properties
                if (auto ident = dynamic_cast<Identifier*>(node.property.get())) {
                    if (ident->name == "length") {
                        currentType = TypeFactory::createInt();
                    } else {
                        reportError("Unknown array property: " + ident->name);
                        currentType = TypeFactory::createUnknown();
                    }
                }
            }
        } else {
            reportError("Cannot access property of non-object type");
            currentType = TypeFactory::createUnknown();
        }
    }
    
    void visit(ArrayExpression& node) override {
        if (node.elements.empty()) {
            // Empty array - infer as Array<unknown>
            currentType = TypeFactory::createArray(TypeFactory::createUnknown());
            return;
        }
        
        // Infer element type from first element
        node.elements[0]->accept(*this);
        TypePtr elementType = currentType;
        
        // Check that all elements are compatible
        for (size_t i = 1; i < node.elements.size(); ++i) {
            node.elements[i]->accept(*this);
            if (!currentType->isAssignableTo(elementType)) {
                // Create union type if elements differ
                std::vector<TypePtr> unionTypes = {elementType, currentType};
                elementType = TypeFactory::createUnion(std::move(unionTypes));
            }
        }
        
        currentType = TypeFactory::createArray(elementType);
    }
    
    void visit(ObjectExpression& node) override {
        auto objType = std::make_shared<ObjectType>();
        
        for (const auto& prop : node.properties) {
            if (auto ident = dynamic_cast<Identifier*>(prop->key.get())) {
                prop->value->accept(*this);
                objType->addProperty(ident->name, currentType);
            } else if (auto strLit = dynamic_cast<StringLiteral*>(prop->key.get())) {
                prop->value->accept(*this);
                objType->addProperty(strLit->value, currentType);
            }
        }
        
        currentType = objType;
    }
    
    void visit(ConditionalExpression& node) override {
        node.test->accept(*this);
        // Test should be boolean-like (we'll be lenient)
        
        node.consequent->accept(*this);
        TypePtr trueType = currentType;
        
        node.alternate->accept(*this);
        TypePtr falseType = currentType;
        
        // Result type is union of both branches
        if (trueType->equals(falseType)) {
            currentType = trueType;
        } else {
            std::vector<TypePtr> unionTypes = {trueType, falseType};
            currentType = TypeFactory::createUnion(std::move(unionTypes));
        }
    }
    
    void visit(VariableDeclaration& node) override {
        TypePtr varType;
        
        if (node.initializer) {
            // Infer type from initializer
            node.initializer->accept(*this);
            varType = currentType;
        } else if (node.typeAnnotation) {
            // Use explicit type annotation
            varType = parseTypeAnnotation(*node.typeAnnotation);
        } else {
            // No initializer or annotation
            varType = TypeFactory::createUnknown();
        }
        
        env.define(node.name, varType);
        currentType = varType;
    }
    
    void visit(Assignment& node) override {
        auto varType = env.lookup(node.target);
        if (!varType) {
            reportError("Undefined variable: " + node.target);
            currentType = TypeFactory::createUnknown();
            return;
        }
        
        node.value->accept(*this);
        TypePtr valueType = currentType;
        
        // Check assignment compatibility
        if (!valueType->isAssignableTo(*varType)) {
            reportError("Cannot assign " + valueType->toString() + 
                       " to " + (*varType)->toString());
        }
        
        currentType = *varType;
    }
    
    // Helper to parse type annotations
    TypePtr parseTypeAnnotation(const std::string& annotation) {
        if (annotation == "number" || annotation == "Int") return TypeFactory::createInt();
        if (annotation == "float" || annotation == "Float") return TypeFactory::createFloat();
        if (annotation == "string" || annotation == "String") return TypeFactory::createString();
        if (annotation == "boolean" || annotation == "Bool") return TypeFactory::createBool();
        if (annotation == "any") return TypeFactory::createAny();
        
        // Array types: Array<T>
        if (annotation.starts_with("Array<") && annotation.ends_with(">")) {
            std::string elementTypeName = annotation.substr(6, annotation.length() - 7);
            auto elementType = parseTypeAnnotation(elementTypeName);
            return TypeFactory::createArray(elementType);
        }
        
        // Default to unknown for unrecognized types
        return TypeFactory::createUnknown();
    }
    
    // Additional visit methods for other node types...
    // (Function, Component, etc. will be implemented as needed)
    
    void visit(Function& node) override {
        env.pushScope();
        
        // Add parameters to scope
        std::vector<TypePtr> paramTypes;
        for (size_t i = 0; i < node.parameters.size(); ++i) {
            TypePtr paramType;
            if (i < node.parameterTypes.size() && !node.parameterTypes[i].empty()) {
                paramType = parseTypeAnnotation(node.parameterTypes[i]);
            } else {
                paramType = TypeFactory::createAny();
            }
            paramTypes.push_back(paramType);
            env.define(node.parameters[i], paramType);
        }
        
        // Check function body
        node.body->accept(*this);
        TypePtr bodyType = currentType;
        
        // Determine return type
        TypePtr returnType;
        if (node.returnType) {
            returnType = parseTypeAnnotation(*node.returnType);
        } else {
            returnType = bodyType; // Infer from body
        }
        
        env.popScope();
        
        // Check if function is async
        bool isAsync = std::find(node.modifiers.begin(), node.modifiers.end(), "@async") != node.modifiers.end();
        
        auto funcType = TypeFactory::createFunction(std::move(paramTypes), returnType, isAsync);
        env.define(node.name, funcType);
        currentType = funcType;
    }
    
    void visit(Component& node) override {
        auto compType = TypeFactory::createComponent(node.name);
        env.define(node.name, compType);
        currentType = compType;
    }
    
    // Default implementations for other AST nodes
    void visit(ExpressionStatement& node) override {
        node.expression->accept(*this);
    }
    
    void visit(BlockStatement& node) override {
        env.pushScope();
        for (const auto& stmt : node.statements) {
            stmt->accept(*this);
        }
        env.popScope();
    }
    
    void visit(IfStatement& node) override {
        node.test->accept(*this);
        node.consequent->accept(*this);
        if (node.alternate) {
            node.alternate->accept(*this);
        }
    }
    
    void visit(WhileStatement& node) override {
        node.test->accept(*this);
        node.body->accept(*this);
    }
    
    void visit(ForStatement& node) override {
        env.pushScope();
        if (node.init) node.init->accept(*this);
        if (node.test) node.test->accept(*this);
        if (node.update) node.update->accept(*this);
        node.body->accept(*this);
        env.popScope();
    }
    
    void visit(ForInStatement& node) override {
        env.pushScope();
        node.iterable->accept(*this);
        // Infer loop variable type from iterable
        TypePtr iterableType = currentType;
        if (iterableType->isArray()) {
            auto arrType = std::static_pointer_cast<ArrayType>(iterableType);
            env.define(node.variable, arrType->elementType);
        } else {
            env.define(node.variable, TypeFactory::createAny());
        }
        node.body->accept(*this);
        env.popScope();
    }
    
    void visit(ReturnStatement& node) override {
        if (node.argument) {
            node.argument->accept(*this);
        } else {
            currentType = TypeFactory::createNull();
        }
    }
    
    void visit(BreakStatement& node) override {
        currentType = TypeFactory::createNull();
    }
    
    void visit(ContinueStatement& node) override {
        currentType = TypeFactory::createNull();
    }
    
    void visit(Import& node) override {
        // Import statements don't have a type
        currentType = TypeFactory::createNull();
    }
    
    void visit(Export& node) override {
        if (node.declaration) {
            node.declaration->accept(*this);
        }
    }
    
    void visit(Tag& node) override {
        // ALTX tags return component elements
        currentType = TypeFactory::createAny();
    }
    
    void visit(TextContent& node) override {
        currentType = TypeFactory::createString();
    }
    
    void visit(Program& node) override {
        for (const auto& stmt : node.globalStatements) {
            stmt->accept(*this);
        }
        for (const auto& func : node.functions) {
            func->accept(*this);
        }
        for (const auto& comp : node.components) {
            comp->accept(*this);
        }
    }
    
    void visit(AsyncBlock& node) override {
        node.tryBlock->accept(*this);
        if (node.catchBlock) {
            node.catchBlock->accept(*this);
        }
        if (node.finallyBlock) {
            node.finallyBlock->accept(*this);
        }
    }
    
    void visit(TryStatement& node) override {
        node.block->accept(*this);
        if (node.catchBlock) {
            env.pushScope();
            if (!node.catchVariable.empty()) {
                env.define(node.catchVariable, TypeFactory::createAny());
            }
            node.catchBlock->accept(*this);
            env.popScope();
        }
        if (node.finallyBlock) {
            node.finallyBlock->accept(*this);
        }
    }
    
    void visit(ThrowStatement& node) override {
        node.argument->accept(*this);
    }
};

} // namespace Alterion
