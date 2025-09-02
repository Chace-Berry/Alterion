// codegen.cpp
// Alterion Language - Phase 4: Code Generation
#include "codegen.h"
#include <iostream>

CodeGenerator::CodeGenerator(CodegenTarget target) {
    ctx.target = target;
    ctx.output = "";
}

void CodeGenerator::generate(std::shared_ptr<ASTNode> root) {
    emitNode(root);
}

std::string CodeGenerator::getOutput() const {
    return ctx.output;
}

void CodeGenerator::emitNode(std::shared_ptr<ASTNode> node) {
    if (!node) return;
    switch (ctx.target) {
        case CodegenTarget::NativeUI:
            emitNativeUI(node);
            break;
        case CodegenTarget::WDOM:
            emitWDOM(node);
            break;
        case CodegenTarget::Server:
            emitServer(node);
            break;
        case CodegenTarget::Custom:
            emitCustom(node);
            break;
    }
    // Recurse children
    for (const auto& child : node->children) {
        emitNode(child);
    }
}

void CodeGenerator::emitNativeUI(std::shared_ptr<ASTNode> node) {
    // Example: emit C++/Swift/Kotlin code for native UI
    ctx.output += "// NativeUI code for node: " + node->toString() + "\n";
}
void CodeGenerator::emitWDOM(std::shared_ptr<ASTNode> node) {
    // Example: emit JavaScript/TypeScript for web
    ctx.output += "// WDOM code for node: " + node->toString() + "\n";
}
void CodeGenerator::emitServer(std::shared_ptr<ASTNode> node) {
    // Example: emit Node.js/JavaScript for server
    ctx.output += "// Server code for node: " + node->toString() + "\n";
}
void CodeGenerator::emitCustom(std::shared_ptr<ASTNode> node) {
    // Example: emit custom target code
    ctx.output += "// Custom code for node: " + node->toString() + "\n";
}
