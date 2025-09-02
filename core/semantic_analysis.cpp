// semantic_analysis.cpp
// Alterion Language - Phase 3: Semantic Analysis & Type Checking
#include "semantic_analysis.h"
#include <iostream>

SymbolTable::SymbolTable() {
    enterScope();
}
void SymbolTable::enterScope() {
    scopes.push_back({});
}
void SymbolTable::exitScope() {
    if (!scopes.empty()) scopes.pop_back();
}
bool SymbolTable::declare(const Symbol& symbol) {
    auto& current = scopes.back();
    if (current.count(symbol.name)) return false;
    current[symbol.name] = symbol;
    return true;
}
Symbol* SymbolTable::lookup(const std::string& name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) return &found->second;
    }
    return nullptr;
}
int SymbolTable::currentScopeLevel() const {
    return (int)scopes.size() - 1;
}

TypeChecker::TypeChecker(SymbolTable& symbols) : symbols(symbols) {}
void TypeChecker::check(std::shared_ptr<ASTNode> root) {
    checkNode(root);
}
void TypeChecker::reportError(const std::string& message, int line, int column) {
    errors.push_back("Type error at " + std::to_string(line) + ":" + std::to_string(column) + " - " + message);
}
void TypeChecker::checkNode(std::shared_ptr<ASTNode> node) {
    // Example: check variable declarations
    if (!node) return;
    // ... implement type checking for all node types ...
    // For demo, just recurse children
    for (const auto& child : node->children) {
        checkNode(child);
    }
}

SemanticAnalyzer::SemanticAnalyzer() : typeChecker(symbols) {}
void SemanticAnalyzer::analyze(std::shared_ptr<ASTNode> root) {
    symbols.enterScope();
    typeChecker.check(root);
    errors = typeChecker.errors;
    symbols.exitScope();
}
