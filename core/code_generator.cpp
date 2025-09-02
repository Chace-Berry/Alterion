#include "code_generator.h"
#include <algorithm>
#include <regex>

namespace Alterion {

// Component generation - converts Alterion components to React components
void CodeGenerator::visit(Component& node) {
    context.addDependency("react");
    
    if (context.getConfig().target == CodeGenConfig::Target::TypeScript) {
        context.addDependency("@types/react");
    }
    
    // Generate component interface for TypeScript
    if (context.getConfig().target == CodeGenConfig::Target::TypeScript) {
        emitLine("interface " + node.name + "Props {");
        context.pushScope(node.name + "Props");
        
        // Extract props from value bindings in render method
        if (node.renderMethod) {
            // This would analyze the AST to find value bindings and generate prop types
            emitLine("// Props will be inferred from value bindings");
        }
        
        context.popScope();
        emitLine("}");
        emitLine();
    }
    
    // Start component definition
    if (context.getConfig().useArrowFunctions) {
        emit("const " + node.name);
        if (context.getConfig().target == CodeGenConfig::Target::TypeScript) {
            emit(": React.FC<" + node.name + "Props>");
        }
        emit(" = (");
        
        // Props parameter
        if (context.getConfig().target == CodeGenConfig::Target::TypeScript) {
            emit("props: " + node.name + "Props");
        } else {
            emit("props");
        }
        emit(") => {\n");
    } else {
        emit("function " + node.name + "(");
        if (context.getConfig().target == CodeGenConfig::Target::TypeScript) {
            emit("props: " + node.name + "Props");
        } else {
            emit("props");
        }
        emit(")");
        
        if (context.getConfig().target == CodeGenConfig::Target::TypeScript) {
            emit(": JSX.Element");
        }
        emit(" {\n");
    }
    
    context.pushScope(node.name);
    
    // Generate state variables using React hooks
    for (const auto& state : node.stateVariables) {
        emitLine("const [" + state.name + ", set" + 
                capitalizeFirst(state.name) + "] = React.useState");
        
        if (context.getConfig().target == CodeGenConfig::Target::TypeScript && state.typeAnnotation) {
            emit("<" + *state.typeAnnotation + ">");
        }
        
        emit("(");
        if (state.initializer) {
            state.initializer->accept(*this);
        } else {
            emit("undefined");
        }
        emit(");");
        emitLine();
    }
    
    if (!node.stateVariables.empty()) emitLine();
    
    // Generate methods as functions
    for (const auto& method : node.methods) {
        emitLine("const " + method->name + " = ");
        
        // Check if method is async
        bool isAsync = std::find(method->modifiers.begin(), method->modifiers.end(), "@async") != method->modifiers.end();
        if (isAsync) emit("async ");
        
        emit("(");
        
        // Parameters
        for (size_t i = 0; i < method->parameters.size(); ++i) {
            if (i > 0) emit(", ");
            emit(method->parameters[i]);
            
            if (context.getConfig().target == CodeGenConfig::Target::TypeScript && 
                i < method->parameterTypes.size() && !method->parameterTypes[i].empty()) {
                emit(": " + method->parameterTypes[i]);
            }
        }
        
        emit(")");
        
        // Return type
        if (context.getConfig().target == CodeGenConfig::Target::TypeScript && method->returnType) {
            emit(": " + *method->returnType);
        }
        
        emit(" => ");
        method->body->accept(*this);
        emit(";");
        emitLine();
    }
    
    if (!node.methods.empty()) emitLine();
    
    // Generate render return
    emitLine("return (");
    context.pushScope("render");
    
    if (node.renderMethod) {
        node.renderMethod->accept(*this);
    } else {
        emitLine(context.getIndent() + "<div>No render method</div>");
    }
    
    context.popScope();
    emitLine(context.getIndent() + ");");
    
    context.popScope();
    emitLine("};");
}

// Async block generation - converts to async/await
void CodeGenerator::visit(AsyncBlock& node) {
    emitLine("(async () => {");
    context.pushScope("async");
    
    for (const auto& stmt : node.statements) {
        emit(context.getIndent());
        stmt->accept(*this);
        emitSemicolon();
        emitLine();
    }
    
    context.popScope();
    emit(context.getIndent() + "})()");
}

// ALTX/JSX generation - converts Alterion elements to JSX
void CodeGenerator::visit(Tag& node) {
    emit("<" + node.name);
    
    // Handle attributes
    for (const auto& attr : node.attributes) {
        emit(" " + attr->name);
        
        if (attr->value) {
            emit("=");
            
            // Check if it's an expression (starts with {)
            if (auto stringLit = dynamic_cast<StringLiteral*>(attr->value.get())) {
                // Regular string attribute
                emit("\"" + escapeLiteral(stringLit->value) + "\"");
            } else {
                // Expression attribute
                emit("{");
                attr->value->accept(*this);
                emit("}");
            }
        }
    }
    
    // Handle special Alterion attributes
    for (const auto& modifier : node.modifiers) {
        if (modifier == "center") {
            emit(" style={{display: 'flex', justifyContent: 'center', alignItems: 'center'}}");
        } else if (modifier == "hidden") {
            emit(" style={{display: 'none'}}");
        }
        // Add more modifiers as needed
    }
    
    // Handle styles
    if (!node.styles.empty()) {
        emit(" style={{");
        bool first = true;
        for (const auto& [property, value] : node.styles) {
            if (!first) emit(", ");
            
            // Convert CSS property names to camelCase
            std::string camelProperty = convertToCamelCase(property);
            emit(camelProperty + ": ");
            
            if (value->starts_with("{")) {
                // Expression value
                emit(value->substr(1, value->length() - 2));
            } else {
                emit("'" + *value + "'");
            }
            first = false;
        }
        emit("}}");
    }
    
    if (node.children.empty()) {
        emit(" />");
    } else {
        emit(">");
        
        // Handle children
        bool hasComplexChildren = false;
        for (const auto& child : node.children) {
            if (!dynamic_cast<TextContent*>(child.get())) {
                hasComplexChildren = true;
                break;
            }
        }
        
        if (hasComplexChildren) {
            emitLine();
            context.pushScope();
        }
        
        for (const auto& child : node.children) {
            if (hasComplexChildren) {
                emit(context.getIndent());
            }
            child->accept(*this);
            if (hasComplexChildren) {
                emitLine();
            }
        }
        
        if (hasComplexChildren) {
            context.popScope();
            emit(context.getIndent());
        }
        
        emit("</" + node.name + ">");
    }
}

void CodeGenerator::visit(TextContent& node) {
    // Handle text interpolation
    std::string text = node.content;
    std::regex interpolationRegex(R"(\{([^}]+)\})");
    std::smatch match;
    
    size_t lastPos = 0;
    bool hasInterpolation = false;
    
    while (std::regex_search(text.begin() + lastPos, text.end(), match, interpolationRegex)) {
        hasInterpolation = true;
        
        // Emit text before interpolation
        if (match.prefix().length() > 0) {
            emit(escapeLiteral(match.prefix().str()));
        }
        
        // Emit interpolated expression
        emit("{" + match[1].str() + "}");
        
        lastPos += match.prefix().length() + match.length();
    }
    
    // Emit remaining text
    if (lastPos < text.length()) {
        emit(escapeLiteral(text.substr(lastPos)));
    }
    
    if (!hasInterpolation) {
        emit(escapeLiteral(text));
    }
}

// Try-catch statement generation
void CodeGenerator::visit(TryStatement& node) {
    emit("try ");
    node.tryBlock->accept(*this);
    
    if (node.catchBlock) {
        emit(" catch");
        if (!node.catchParameter.empty()) {
            emit(" (" + node.catchParameter + ")");
        }
        emit(" ");
        node.catchBlock->accept(*this);
    }
    
    if (node.finallyBlock) {
        emit(" finally ");
        node.finallyBlock->accept(*this);
    }
}

void CodeGenerator::visit(ThrowStatement& node) {
    emit("throw ");
    node.expression->accept(*this);
    needsSemicolon = true;
}

// Utility functions
std::string CodeGenerator::capitalizeFirst(const std::string& str) {
    if (str.empty()) return str;
    std::string result = str;
    result[0] = std::toupper(result[0]);
    return result;
}

std::string CodeGenerator::convertToCamelCase(const std::string& kebabCase) {
    std::string result;
    bool nextUpper = false;
    
    for (char c : kebabCase) {
        if (c == '-') {
            nextUpper = true;
        } else if (nextUpper) {
            result += std::toupper(c);
            nextUpper = false;
        } else {
            result += c;
        }
    }
    
    return result;
}

// Enhanced code generation with optimization
class OptimizedCodeGenerator : public CodeGenerator {
private:
    bool eliminateDeadCode;
    bool inlineSmallFunctions;
    bool optimizeStateUpdates;
    
public:
    OptimizedCodeGenerator(const CodeGenConfig& config) 
        : CodeGenerator(config), eliminateDeadCode(true), 
          inlineSmallFunctions(true), optimizeStateUpdates(true) {}
    
    void visit(Component& node) override {
        // Add React performance optimizations
        if (shouldMemoizeComponent(node)) {
            emit("const " + node.name + " = React.memo((");
        } else {
            CodeGenerator::visit(node);
            return;
        }
        
        // Rest of component generation with memoization
        CodeGenerator::visit(node);
        
        if (shouldMemoizeComponent(node)) {
            emit(");");
        }
    }
    
private:
    bool shouldMemoizeComponent(const Component& node) {
        // Heuristics for when to use React.memo
        return node.stateVariables.size() > 2 || node.methods.size() > 3;
    }
};

// Specialized generators for different targets
class ReactNativeGenerator : public CodeGenerator {
public:
    ReactNativeGenerator(const CodeGenConfig& config) : CodeGenerator(config) {}
    
    void visit(Tag& node) override {
        // Map HTML tags to React Native components
        std::string mappedTag = mapHtmlToReactNative(node.name);
        
        // Create a modified node with the mapped tag
        Tag rnNode = node;
        rnNode.name = mappedTag;
        
        CodeGenerator::visit(rnNode);
    }
    
private:
    std::string mapHtmlToReactNative(const std::string& htmlTag) {
        static const std::unordered_map<std::string, std::string> tagMap = {
            {"div", "View"},
            {"span", "Text"},
            {"p", "Text"},
            {"h1", "Text"},
            {"h2", "Text"},
            {"h3", "Text"},
            {"button", "TouchableOpacity"},
            {"input", "TextInput"},
            {"img", "Image"},
            {"a", "Text"}  // Would need TouchableOpacity wrapper
        };
        
        auto it = tagMap.find(htmlTag);
        return it != tagMap.end() ? it->second : htmlTag;
    }
};

} // namespace Alterion
