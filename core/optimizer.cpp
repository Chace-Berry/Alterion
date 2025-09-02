// optimizer.cpp
// Alterion Language - Phase 5: Optimization
#include "optimizer.h"
#include <iostream>

Optimizer::Optimizer(OptimizationLevel level) : level(level) {}

void Optimizer::optimize(std::shared_ptr<ASTNode> root) {
    optimizeNode(root);
}

std::string Optimizer::getReport() const {
    std::string out;
    for (const auto& line : report) out += line + "\n";
    return out;
}

void Optimizer::optimizeNode(std::shared_ptr<ASTNode> node) {
    if (!node) return;
    // Example: dead code elimination, constant folding, etc.
    // For demo, just log node type
    report.push_back("Optimized node: " + node->toString());
    for (const auto& child : node->children) {
        optimizeNode(child);
    }
}
