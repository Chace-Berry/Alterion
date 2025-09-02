#include "ast_complete.h"
#include <sstream>

// Implementation of toString methods and visitor accept methods

// StringLiteral
void StringLiteral::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// NumberLiteral  
void NumberLiteral::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// BooleanLiteral
void BooleanLiteral::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// NullLiteral
void NullLiteral::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// Identifier
void Identifier::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// ValueBinding
void ValueBinding::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// BinaryExpression
void BinaryExpression::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// UnaryExpression
void UnaryExpression::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// CallExpression
std::string CallExpression::toString() const {
    std::stringstream ss;
    ss << callee->toString() << "(";
    for (size_t i = 0; i < arguments.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << arguments[i]->toString();
    }
    ss << ")";
    return ss.str();
}

void CallExpression::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// MemberExpression
std::string MemberExpression::toString() const {
    if (computed) {
        return object->toString() + "[" + property->toString() + "]";
    } else {
        return object->toString() + "." + property->toString();
    }
}

void MemberExpression::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// ArrayExpression
std::string ArrayExpression::toString() const {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < elements.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << elements[i]->toString();
    }
    ss << "]";
    return ss.str();
}

void ArrayExpression::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// ObjectExpression
std::string ObjectExpression::toString() const {
    std::stringstream ss;
    ss << "{";
    for (size_t i = 0; i < properties.size(); ++i) {
        if (i > 0) ss << ", ";
        const auto& prop = properties[i];
        if (prop->computed) {
            ss << "[" << prop->key->toString() << "]";
        } else {
            ss << prop->key->toString();
        }
        ss << ": " << prop->value->toString();
    }
    ss << "}";
    return ss.str();
}

void ObjectExpression::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// ConditionalExpression
std::string ConditionalExpression::toString() const {
    return test->toString() + " ? " + consequent->toString() + " : " + alternate->toString();
}

void ConditionalExpression::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// Assignment
std::string Assignment::toString() const {
    return target + " " + operator_ + " " + value->toString() + ";";
}

void Assignment::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// VariableDeclaration
std::string VariableDeclaration::toString() const {
    std::string result = kind + " " + name;
    if (typeAnnotation.has_value()) {
        result += ": " + *typeAnnotation;
    }
    if (initializer) {
        result += " = " + initializer->toString();
    }
    return result + ";";
}

void VariableDeclaration::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// ExpressionStatement
void ExpressionStatement::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// BlockStatement
std::string BlockStatement::toString() const {
    std::stringstream ss;
    ss << "{\n";
    for (const auto& stmt : statements) {
        ss << "  " << stmt->toString() << "\n";
    }
    ss << "}";
    return ss.str();
}

void BlockStatement::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// IfStatement
std::string IfStatement::toString() const {
    std::string result = "if (" + test->toString() + ") " + consequent->toString();
    if (alternate) {
        result += " else " + alternate->toString();
    }
    return result;
}

void IfStatement::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// WhileStatement
std::string WhileStatement::toString() const {
    return "while (" + test->toString() + ") " + body->toString();
}

void WhileStatement::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// ForStatement
std::string ForStatement::toString() const {
    std::string result = "for (";
    if (init) result += init->toString();
    result += "; ";
    if (test) result += test->toString();
    result += "; ";
    if (update) result += update->toString();
    result += ") " + body->toString();
    return result;
}

void ForStatement::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// ForInStatement
std::string ForInStatement::toString() const {
    return "for " + variable + " in " + iterable->toString() + " " + body->toString();
}

void ForInStatement::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// ReturnStatement
std::string ReturnStatement::toString() const {
    if (argument) {
        return "return " + argument->toString() + ";";
    } else {
        return "return;";
    }
}

void ReturnStatement::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// BreakStatement
void BreakStatement::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// ContinueStatement
void ContinueStatement::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// Function
std::string Function::toString() const {
    std::stringstream ss;
    
    // Add modifiers
    for (const auto& modifier : modifiers) {
        ss << modifier << " ";
    }
    
    ss << "function " << name << "(";
    for (size_t i = 0; i < parameters.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << parameters[i];
        if (i < parameterTypes.size() && !parameterTypes[i].empty()) {
            ss << ": " << parameterTypes[i];
        }
    }
    ss << ")";
    
    if (returnType.has_value()) {
        ss << ": " << *returnType;
    }
    
    ss << " " << body->toString();
    return ss.str();
}

void Function::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// Import
std::string Import::toString() const {
    std::stringstream ss;
    ss << "import {";
    for (size_t i = 0; i < bindings.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << bindings[i];
    }
    ss << "} from \"" << source << "\";";
    return ss.str();
}

void Import::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// Export
std::string Export::toString() const {
    std::string result = "export ";
    if (isDefault) {
        result += "default ";
    }
    if (declaration) {
        result += declaration->toString();
    }
    return result;
}

void Export::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// Attribute
std::string Attribute::toString() const {
    if (value) {
        return name + "=" + value->toString();
    } else {
        return name;
    }
}

// Tag
std::string Tag::toString() const {
    std::stringstream ss;
    ss << "<" << tagName;
    
    // Attributes
    for (const auto& attr : attributes) {
        ss << " " << attr->toString();
    }
    
    // Styles
    if (!styles.empty()) {
        ss << " style=\"";
        for (size_t i = 0; i < styles.size(); ++i) {
            if (i > 0) ss << "; ";
            ss << styles[i].property << ": " << styles[i].value;
        }
        ss << "\"";
    }
    
    if (isSelfClosing) {
        ss << " />";
    } else {
        ss << ">";
        
        // Children
        for (const auto& child : children) {
            ss << child->toString();
        }
        
        ss << "</" << tagName << ">";
    }
    
    return ss.str();
}

void Tag::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// TextContent
void TextContent::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// Component
std::string Component::toString() const {
    std::stringstream ss;
    
    // Modifiers
    for (const auto& modifier : modifiers) {
        ss << modifier << " ";
    }
    
    ss << "component " << name << " {\n";
    
    // Statements (state, methods, etc.)
    for (const auto& stmt : statements) {
        ss << "  " << stmt->toString() << "\n";
    }
    
    // ALTX body
    if (!body.empty()) {
        ss << "  render:\n";
        for (const auto& node : body) {
            ss << "    " << node->toString() << "\n";
        }
    }
    
    ss << "}";
    return ss.str();
}

void Component::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// Program
std::string Program::toString() const {
    std::stringstream ss;
    
    // Global statements
    for (const auto& stmt : globalStatements) {
        ss << stmt->toString() << "\n";
    }
    
    // Functions
    for (const auto& func : functions) {
        ss << func->toString() << "\n\n";
    }
    
    // Components
    for (const auto& comp : components) {
        ss << comp->toString() << "\n\n";
    }
    
    return ss.str();
}

void Program::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// AsyncBlock
std::string AsyncBlock::toString() const {
    std::string result = "async{[" + tryBlock->toString() + "]";
    if (catchBlock) {
        result += "[" + catchBlock->toString() + "]";
    }
    if (finallyBlock) {
        result += "[" + finallyBlock->toString() + "]";
    }
    result += "}";
    return result;
}

void AsyncBlock::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// TryStatement
std::string TryStatement::toString() const {
    std::string result = "try " + block->toString();
    if (catchBlock) {
        result += " catch";
        if (!catchVariable.empty()) {
            result += " (" + catchVariable + ")";
        }
        result += " " + catchBlock->toString();
    }
    if (finallyBlock) {
        result += " finally " + finallyBlock->toString();
    }
    return result;
}

void TryStatement::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

// ThrowStatement
std::string ThrowStatement::toString() const {
    return "throw " + argument->toString() + ";";
}

void ThrowStatement::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}
