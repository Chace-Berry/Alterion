// optimizer_test.cpp
// Test suite for Alterion optimizer
#include "optimizer.h"
#include "ast_complete.h"
#include <iostream>
#include <memory>

int main() {
    // Example: create a simple AST for testing
    auto root = std::make_shared<ASTNode>(ASTNodeType::Program);
    auto funcDecl = std::make_shared<ASTNode>(ASTNodeType::FunctionDeclaration);
    funcDecl->name = "add";
    root->children.push_back(funcDecl);

    // Run optimizer
    Optimizer opt(OptimizationLevel::Aggressive);
    opt.optimize(root);
    std::cout << "Optimization Report:\n" << opt.getReport() << std::endl;
    return 0;
}
