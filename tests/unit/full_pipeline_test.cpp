// full_pipeline_test.cpp
// Comprehensive test for Alterion language pipeline: lexer, parser, semantic analysis, codegen, optimizer, LSP
#include "lexer.h"
#include "parser_complete.h"
#include "ast_complete.h"
#include "../semantic_analysis.h"
#include "../codegen.h"
#include "../optimizer.h"
#include "../lsp_server.h"
#include <iostream>
#include <memory>

int main() {
    std::string sourceCode = R"(
        component Counter {
            count = 0
            increment {
                count = count + 1
            }
            render:
                <button onClick={increment}>
                    Count: {count}
                </button>
        }
    )";

    // Lexer
    Lexer lexer(sourceCode);
    auto tokens = lexer.tokenize();
    std::cout << "Lexer output: " << tokens.size() << " tokens\n";

    // Parser
    Parser parser(std::move(tokens));
    auto programUnique = parser.parse();
    std::cout << "Parser output: AST generated\n";

    // Use shared_ptr for the pipeline
    std::shared_ptr<Program> program = std::move(programUnique);

    // Semantic Analysis
    SemanticAnalyzer analyzer;
    analyzer.analyze(program);
    if (!analyzer.errors.empty()) {
        std::cout << "Semantic errors found:\n";
        for (const auto& err : analyzer.errors) std::cout << err << std::endl;
    } else {
        std::cout << "Semantic analysis: No errors\n";
    }

    // Optimization
    Optimizer optimizer(OptimizationLevel::Aggressive);
    optimizer.optimize(program);
    std::cout << "Optimizer report:\n" << optimizer.getReport();

    // Code Generation
    CodeGenerator codegen(CodegenTarget::NativeUI);
    codegen.generate(program);
    std::cout << "Codegen output (NativeUI):\n" << codegen.getOutput();

    // LSP Server simulation
    LSPServer lsp;
    lsp.start(8080);
    lsp.handleRequest("textDocument/didOpen");
    std::cout << "LSP Response: " << lsp.getResponse() << std::endl;

    std::cout << "\nFull pipeline test complete!\n";
    return 0;
}
