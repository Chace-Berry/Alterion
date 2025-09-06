// codegen.h
// Alterion Language - Phase 4: Code Generation
// Reference implementation for multi-platform code generation

#pragma once
#include "ast_complete.h"
#include <string>
#include <vector>
#include <memory>

// Target platforms
enum class CodegenTarget {
    NativeUI,
    WDOM,
    Server,
    Custom
};

// Codegen context
struct CodegenContext {
    CodegenTarget target;
    std::string output;
    // Additional config: optimization level, platform, etc.
};

// Code generator interface
class CodeGenerator {
public:
    CodeGenerator(CodegenTarget target);
    void generate(std::shared_ptr<ASTNode> root);
    std::string getOutput() const;
private:
    CodegenContext ctx;
    void emitNode(std::shared_ptr<ASTNode> node);
    // Helpers for each target
    void emitNativeUI(std::shared_ptr<ASTNode> node);
    void emitWDOM(std::shared_ptr<ASTNode> node);
    void emitServer(std::shared_ptr<ASTNode> node);
    void emitCustom(std::shared_ptr<ASTNode> node);
};

// Usage example:
// CodeGenerator gen(CodegenTarget::NativeUI);
// gen.generate(astRoot);
// std::string code = gen.getOutput();
