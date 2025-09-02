// semantic_analysis.h
// Alterion Language - Phase 3: Semantic Analysis & Type Checking
// Reference implementation for AST semantic analysis

#pragma once
#include "ast_complete.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

// Symbol kinds
enum class SymbolKind {
    Variable,
    Function,
    Component,
    Type,
    Parameter,
    Import
};

// Symbol table entry
struct Symbol {
    std::string name;
    SymbolKind kind;
    std::string typeName;
    int scopeLevel;
    // Additional info: location, value, etc.
};

// Symbol table (scoped)
class SymbolTable {
public:
    SymbolTable();
    void enterScope();
    void exitScope();
    bool declare(const Symbol& symbol);
    Symbol* lookup(const std::string& name);
    int currentScopeLevel() const;
private:
    std::vector<std::unordered_map<std::string, Symbol>> scopes;
};

// Type checker
class TypeChecker {
public:
    TypeChecker(SymbolTable& symbols);
    void check(std::shared_ptr<ASTNode> root);
    void reportError(const std::string& message, int line, int column);
    std::vector<std::string> errors;
private:
    SymbolTable& symbols;
    void checkNode(std::shared_ptr<ASTNode> node);
    // Add more helpers for expressions, statements, etc.
};

// Semantic analyzer
class SemanticAnalyzer {
public:
    SemanticAnalyzer();
    void analyze(std::shared_ptr<ASTNode> root);
    std::vector<std::string> errors;
    SymbolTable symbols;
    TypeChecker typeChecker;
};

// Usage example:
// SemanticAnalyzer analyzer;
// analyzer.analyze(astRoot);
// if (!analyzer.errors.empty()) { /* handle errors */ }
