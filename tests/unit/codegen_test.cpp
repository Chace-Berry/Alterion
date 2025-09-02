// codegen_test.cpp
// Test suite for Alterion code generation
#include "codegen.h"
#include "ast_complete.h"
#include <iostream>
#include <memory>

int main() {
    // Example: create a simple AST for testing
    auto root = std::make_shared<ASTNode>(ASTNodeType::Program);
    auto compDecl = std::make_shared<ASTNode>(ASTNodeType::ComponentDeclaration);
    compDecl->name = "Counter";
    root->children.push_back(compDecl);

    // Generate code for NativeUI
    CodeGenerator genNative(CodegenTarget::NativeUI);
    genNative.generate(root);
    std::cout << "NativeUI Output:\n" << genNative.getOutput() << std::endl;

    // Generate code for WDOM
    CodeGenerator genWDOM(CodegenTarget::WDOM);
    genWDOM.generate(root);
    std::cout << "WDOM Output:\n" << genWDOM.getOutput() << std::endl;

    // Generate code for Server
    CodeGenerator genServer(CodegenTarget::Server);
    genServer.generate(root);
    std::cout << "Server Output:\n" << genServer.getOutput() << std::endl;

    return 0;
}
