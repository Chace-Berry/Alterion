#pragma once
#include "ast_complete.h"
#include "type_system.h"
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <memory>

namespace Alterion {

// Phase 2D: Code Generation System
// Transforms Alterion AST to JavaScript/TypeScript

// Code generation configuration
struct CodeGenConfig {
    enum class Target {
        JavaScript,     // Generate JavaScript (ES6+)
        TypeScript,     // Generate TypeScript with full type annotations
        React,          // Generate React components
        ReactNative     // Generate React Native components
    };
    
    enum class ModuleSystem {
        ESModules,      // import/export (default)
        CommonJS,       // require/module.exports
        AMD,            // AMD modules
        UMD             // Universal Module Definition
    };
    
    Target target = Target::TypeScript;
    ModuleSystem moduleSystem = ModuleSystem::ESModules;
    bool generateSourceMaps = true;
    bool minify = false;
    bool includeComments = true;
    std::string indentation = "  "; // 2 spaces
    int maxLineLength = 120;
    bool strictMode = true;
    
    // React-specific options
    bool useArrowFunctions = true;
    bool useHooks = true;
    bool generatePropTypes = false;
    
    // TypeScript-specific options
    bool generateTypeDefinitions = true;
    bool strictNullChecks = true;
    bool noImplicitAny = false;
};

// Source map support
struct SourceLocation {
    size_t line;
    size_t column;
    std::string filename;
    
    SourceLocation(size_t l = 0, size_t c = 0, const std::string& f = "")
        : line(l), column(c), filename(f) {}
};

// Generated code with source mapping
struct GeneratedCode {
    std::string code;
    std::vector<SourceLocation> sourceMaps;
    std::unordered_map<std::string, std::string> metadata;
    
    void addMapping(const SourceLocation& original, size_t generatedLine, size_t generatedCol) {
        // Source map implementation would go here
    }
};

// Code generation context
class CodeGenContext {
private:
    std::vector<std::string> scopes;
    std::unordered_map<std::string, std::string> imports;
    std::unordered_map<std::string, std::string> exports;
    std::vector<std::string> dependencies;
    int indentLevel;
    CodeGenConfig config;
    
public:
    explicit CodeGenContext(const CodeGenConfig& cfg = CodeGenConfig{}) 
        : indentLevel(0), config(cfg) {
        scopes.emplace_back("global");
    }
    
    void pushScope(const std::string& scopeName = "") {
        scopes.push_back(scopeName.empty() ? "anonymous" : scopeName);
        indentLevel++;
    }
    
    void popScope() {
        if (scopes.size() > 1) {
            scopes.pop_back();
            indentLevel--;
        }
    }
    
    std::string getCurrentScope() const {
        return scopes.empty() ? "global" : scopes.back();
    }
    
    std::string getIndent() const {
        std::string result;
        for (int i = 0; i < indentLevel; ++i) {
            result += config.indentation;
        }
        return result;
    }
    
    void addImport(const std::string& module, const std::string& import) {
        imports[module] = import;
    }
    
    void addExport(const std::string& name, const std::string& value) {
        exports[name] = value;
    }
    
    void addDependency(const std::string& dep) {
        dependencies.push_back(dep);
    }
    
    const CodeGenConfig& getConfig() const { return config; }
    const std::unordered_map<std::string, std::string>& getImports() const { return imports; }
    const std::unordered_map<std::string, std::string>& getExports() const { return exports; }
    const std::vector<std::string>& getDependencies() const { return dependencies; }
};

// Main code generator
class CodeGenerator : public ASTVisitor {
private:
    std::stringstream output;
    CodeGenContext context;
    GeneratedCode result;
    bool needsSemicolon;
    
    // Utility methods
    void emit(const std::string& code) {
        output << code;
    }
    
    void emitLine(const std::string& code = "") {
        if (!code.empty()) {
            output << context.getIndent() << code;
        }
        output << "\n";
    }
    
    void emitSemicolon() {
        if (needsSemicolon) {
            output << ";";
            needsSemicolon = false;
        }
    }
    
    std::string escapeLiteral(const std::string& str) {
        std::string result;
        for (char c : str) {
            switch (c) {
                case '"': result += "\\\""; break;
                case '\'': result += "\\'"; break;
                case '\\': result += "\\\\"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += c; break;
            }
        }
        return result;
    }
    
    std::string generateTypeName(const TypePtr& type) {
        if (!type || context.getConfig().target == CodeGenConfig::Target::JavaScript) {
            return "";
        }
        
        switch (type->kind) {
            case Type::Kind::Primitive: {
                auto prim = std::static_pointer_cast<PrimitiveType>(type);
                switch (prim->primitiveKind) {
                    case PrimitiveType::PrimitiveKind::Int: return "number";
                    case PrimitiveType::PrimitiveKind::Float: return "number";
                    case PrimitiveType::PrimitiveKind::Bool: return "boolean";
                    case PrimitiveType::PrimitiveKind::String: return "string";
                    case PrimitiveType::PrimitiveKind::Null: return "null";
                }
                break;
            }
            case Type::Kind::Array: {
                auto arr = std::static_pointer_cast<ArrayType>(type);
                return generateTypeName(arr->elementType) + "[]";
            }
            case Type::Kind::Object: {
                auto obj = std::static_pointer_cast<ObjectType>(type);
                std::string result = "{ ";
                bool first = true;
                for (const auto& [name, propType] : obj->properties) {
                    if (!first) result += ", ";
                    result += name + ": " + generateTypeName(propType);
                    first = false;
                }
                result += " }";
                return result;
            }
            case Type::Kind::Function: {
                auto func = std::static_pointer_cast<FunctionType>(type);
                std::string result = "(";
                for (size_t i = 0; i < func->parameterTypes.size(); ++i) {
                    if (i > 0) result += ", ";
                    result += "arg" + std::to_string(i) + ": " + generateTypeName(func->parameterTypes[i]);
                }
                result += ") => " + generateTypeName(func->returnType);
                return result;
            }
            case Type::Kind::Union: {
                auto union_ = std::static_pointer_cast<UnionType>(type);
                std::string result;
                for (size_t i = 0; i < union_->types.size(); ++i) {
                    if (i > 0) result += " | ";
                    result += generateTypeName(union_->types[i]);
                }
                return result;
            }
            case Type::Kind::Optional: {
                auto opt = std::static_pointer_cast<OptionalType>(type);
                return generateTypeName(opt->innerType) + " | undefined";
            }
            case Type::Kind::Any:
                return "any";
            case Type::Kind::Unknown:
                return "unknown";
            default:
                return "any";
        }
        return "any";
    }
    
    std::string generateTypeAnnotation(const TypePtr& type) {
        if (context.getConfig().target == CodeGenConfig::Target::JavaScript) {
            return "";
        }
        std::string typeName = generateTypeName(type);
        return typeName.empty() ? "" : ": " + typeName;
    }
    
public:
    explicit CodeGenerator(const CodeGenConfig& config = CodeGenConfig{}) 
        : context(config), needsSemicolon(false) {}
    
    GeneratedCode generate(Program& program) {
        program.accept(*this);
        result.code = output.str();
        return result;
    }
    
    // Visitor implementations
    void visit(StringLiteral& node) override {
        emit("\"" + escapeLiteral(node.value) + "\"");
    }
    
    void visit(NumberLiteral& node) override {
        emit(node.value);
    }
    
    void visit(BooleanLiteral& node) override {
        emit(node.value ? "true" : "false");
    }
    
    void visit(NullLiteral& node) override {
        emit("null");
    }
    
    void visit(Identifier& node) override {
        emit(node.name);
    }
    
    void visit(ValueBinding& node) override {
        // External value bindings become props access
        emit("props." + node.name);
    }
    
    void visit(BinaryExpression& node) override {
        bool needsParens = true; // Simplified - should check precedence
        
        if (needsParens) emit("(");
        node.left->accept(*this);
        emit(" " + node.operator_ + " ");
        node.right->accept(*this);
        if (needsParens) emit(")");
    }
    
    void visit(UnaryExpression& node) override {
        emit(node.operator_);
        node.operand->accept(*this);
    }
    
    void visit(CallExpression& node) override {
        node.callee->accept(*this);
        emit("(");
        for (size_t i = 0; i < node.arguments.size(); ++i) {
            if (i > 0) emit(", ");
            node.arguments[i]->accept(*this);
        }
        emit(")");
    }
    
    void visit(MemberExpression& node) override {
        node.object->accept(*this);
        if (node.computed) {
            emit("[");
            node.property->accept(*this);
            emit("]");
        } else {
            emit(".");
            node.property->accept(*this);
        }
    }
    
    void visit(ArrayExpression& node) override {
        emit("[");
        for (size_t i = 0; i < node.elements.size(); ++i) {
            if (i > 0) emit(", ");
            node.elements[i]->accept(*this);
        }
        emit("]");
    }
    
    void visit(ObjectExpression& node) override {
        emit("{\n");
        context.pushScope();
        
        for (size_t i = 0; i < node.properties.size(); ++i) {
            if (i > 0) emit(",\n");
            emit(context.getIndent());
            
            const auto& prop = node.properties[i];
            if (prop->computed) {
                emit("[");
                prop->key->accept(*this);
                emit("]");
            } else {
                prop->key->accept(*this);
            }
            emit(": ");
            prop->value->accept(*this);
        }
        
        context.popScope();
        emit("\n" + context.getIndent() + "}");
    }
    
    void visit(ConditionalExpression& node) override {
        node.test->accept(*this);
        emit(" ? ");
        node.consequent->accept(*this);
        emit(" : ");
        node.alternate->accept(*this);
    }
    
    void visit(VariableDeclaration& node) override {
        bool isConst = (node.kind == "const");
        emit(isConst ? "const " : "let ");
        emit(node.name);
        
        // Add type annotation for TypeScript
        if (context.getConfig().target == CodeGenConfig::Target::TypeScript && node.typeAnnotation) {
            emit(": " + *node.typeAnnotation);
        }
        
        if (node.initializer) {
            emit(" = ");
            node.initializer->accept(*this);
        }
        
        needsSemicolon = true;
    }
    
    void visit(Assignment& node) override {
        emit(node.target + " " + node.operator_ + " ");
        node.value->accept(*this);
        needsSemicolon = true;
    }
    
    void visit(ExpressionStatement& node) override {
        node.expression->accept(*this);
        needsSemicolon = true;
    }
    
    void visit(BlockStatement& node) override {
        emit("{\n");
        context.pushScope();
        
        for (const auto& stmt : node.statements) {
            emit(context.getIndent());
            stmt->accept(*this);
            emitSemicolon();
            emit("\n");
        }
        
        context.popScope();
        emit(context.getIndent() + "}");
    }
    
    void visit(IfStatement& node) override {
        emit("if (");
        node.test->accept(*this);
        emit(") ");
        node.consequent->accept(*this);
        
        if (node.alternate) {
            emit(" else ");
            node.alternate->accept(*this);
        }
    }
    
    void visit(WhileStatement& node) override {
        emit("while (");
        node.test->accept(*this);
        emit(") ");
        node.body->accept(*this);
    }
    
    void visit(ForStatement& node) override {
        emit("for (");
        if (node.init) {
            node.init->accept(*this);
        }
        emit("; ");
        if (node.test) {
            node.test->accept(*this);
        }
        emit("; ");
        if (node.update) {
            node.update->accept(*this);
        }
        emit(") ");
        node.body->accept(*this);
    }
    
    void visit(ForInStatement& node) override {
        emit("for (const " + node.variable + " of ");
        node.iterable->accept(*this);
        emit(") ");
        node.body->accept(*this);
    }
    
    void visit(ReturnStatement& node) override {
        emit("return");
        if (node.argument) {
            emit(" ");
            node.argument->accept(*this);
        }
        needsSemicolon = true;
    }
    
    void visit(BreakStatement& node) override {
        emit("break");
        needsSemicolon = true;
    }
    
    void visit(ContinueStatement& node) override {
        emit("continue");
        needsSemicolon = true;
    }
    
    void visit(Function& node) override {
        // Handle modifiers
        bool isAsync = std::find(node.modifiers.begin(), node.modifiers.end(), "@async") != node.modifiers.end();
        
        if (isAsync) emit("async ");
        emit("function " + node.name + "(");
        
        // Parameters with type annotations
        for (size_t i = 0; i < node.parameters.size(); ++i) {
            if (i > 0) emit(", ");
            emit(node.parameters[i]);
            
            if (context.getConfig().target == CodeGenConfig::Target::TypeScript && 
                i < node.parameterTypes.size() && !node.parameterTypes[i].empty()) {
                emit(": " + node.parameterTypes[i]);
            }
        }
        
        emit(")");
        
        // Return type annotation
        if (context.getConfig().target == CodeGenConfig::Target::TypeScript && node.returnType) {
            emit(": " + *node.returnType);
        }
        
        emit(" ");
        node.body->accept(*this);
    }
    
    void visit(Import& node) override {
        if (context.getConfig().moduleSystem == CodeGenConfig::ModuleSystem::ESModules) {
            emit("import { ");
            for (size_t i = 0; i < node.bindings.size(); ++i) {
                if (i > 0) emit(", ");
                emit(node.bindings[i]);
            }
            emit(" } from \"" + node.source + "\"");
        } else {
            // CommonJS
            emit("const { ");
            for (size_t i = 0; i < node.bindings.size(); ++i) {
                if (i > 0) emit(", ");
                emit(node.bindings[i]);
            }  
            emit(" } = require(\"" + node.source + "\")");
        }
        needsSemicolon = true;
    }
    
    void visit(Export& node) override {
        if (context.getConfig().moduleSystem == CodeGenConfig::ModuleSystem::ESModules) {
            emit("export ");
            if (node.isDefault) {
                emit("default ");
            }
        }
        
        if (node.declaration) {
            node.declaration->accept(*this);
        }
        
        if (context.getConfig().moduleSystem == CodeGenConfig::ModuleSystem::CommonJS && !node.isDefault) {
            emit("\nmodule.exports = { ...module.exports, " + node.name + " }");
            needsSemicolon = true;
        }
    }
    
    // Component generation - this is where the magic happens!
    void visit(Component& node) override;
    
    // Async block generation
    void visit(AsyncBlock& node) override;
    
    // ALTX/JSX generation
    void visit(Tag& node) override;
    void visit(TextContent& node) override;
    
    void visit(Program& node) override {
        // Generate imports first
        for (const auto& stmt : node.globalStatements) {
            if (auto import = dynamic_cast<Import*>(stmt.get())) {
                import->accept(*this);
                emitSemicolon();
                emitLine();
            }
        }
        
        if (!node.globalStatements.empty()) emitLine();
        
        // Generate functions
        for (const auto& func : node.functions) {
            func->accept(*this);
            emitLine();
            emitLine();
        }
        
        // Generate components
        for (const auto& comp : node.components) {
            comp->accept(*this);
            emitLine();
            emitLine();
        }
        
        // Generate other global statements
        for (const auto& stmt : node.globalStatements) {
            if (!dynamic_cast<Import*>(stmt.get())) {
                stmt->accept(*this);
                emitSemicolon();
                emitLine();
            }
        }
    }
    
    // Additional visitor methods will be implemented...
    void visit(TryStatement& node) override;
    void visit(ThrowStatement& node) override;
};

} // namespace Alterion
