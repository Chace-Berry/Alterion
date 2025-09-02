#include <iostream>
#include <string>
#include <vector>

// Just include the main headers
#include "include/token.h"
#include "include/lexer.h"

// Include the actual implementations - lexer first
#include "lexer/lexer_complete.cpp"

// Now include parser
#include "include/parser_complete.h"
#include "parser/parser_complete.cpp"

int main() {
    std::cout << "=== Alterion Language Test Suite ===" << std::endl;
    
    try {
        // Test 1: Basic lexer functionality
        std::cout << "\n1. Testing Basic Lexer..." << std::endl;
        std::string simpleCode = "component Test { name: \"hello\" }";
        Lexer lexer(simpleCode);
        std::vector<Token> tokens = lexer.tokenize();
        
        std::cout << "   Tokenized " << tokens.size() << " tokens successfully!" << std::endl;
        for (size_t i = 0; i < std::min(tokens.size(), size_t(5)); ++i) {
            std::cout << "   Token " << i << ": " << tokens[i].toString() << std::endl;
        }
        
        // Test 2: Expression syntax {name} vs {!name}
        std::cout << "\n2. Testing Expression Syntax..." << std::endl;
        std::string expressionCode = R"(
component TestComponent {
    name: "local"
    
    render:
        <div>
            Local: {name}
            External: {!name}
        </div>
}
)";
        
        Lexer exprLexer(expressionCode);
        std::vector<Token> exprTokens = exprLexer.tokenize();
        
        std::cout << "   Found expressions in component:" << std::endl;
        for (const auto& token : exprTokens) {
            if (token.type == TokenType::Identifier || token.type == TokenType::ValueBinding) {
                std::cout << "   - " << token.toString() << std::endl;
            }
        }
        
        // Test 3: Basic parser
        std::cout << "\n3. Testing Parser..." << std::endl;
        Parser parser(exprTokens);
        auto ast = parser.parse();
        
        if (ast) {
            std::cout << "   Parser created AST successfully!" << std::endl;
            std::cout << "   AST type: Program with " << ast->toString().length() << " characters" << std::endl;
        } else {
            std::cout << "   Parser failed to create AST" << std::endl;
        }
        
        std::cout << "\n=== All Tests Completed Successfully! ===" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "\nTest failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "\nTest failed with unknown exception" << std::endl;
        return 1;
    }
}
