// semantic_analysis_test.cpp
// Test suite for Alterion semantic analysis
#include "semantic_analysis.h"
#include "ast_complete.h"
#include <iostream>
#include <memory>

int main() {
    // Example: create a simple AST for testing
    auto root = std::make_shared<ASTNode>(ASTNodeType::Program);
    auto varDecl = std::make_shared<ASTNode>(ASTNodeType::VariableDeclaration);
    varDecl->name = "count";
    varDecl->typeName = "int";
    root->children.push_back(varDecl);

    // Run semantic analysis
    SemanticAnalyzer analyzer;
    analyzer.analyze(root);

    // Print errors
    if (!analyzer.errors.empty()) {
        std::cout << "Semantic errors found:\n";
        for (const auto& err : analyzer.errors) {
            std::cout << err << std::endl;
        }
    } else {
        std::cout << "No semantic errors detected." << std::endl;
    }
    return 0;
}
