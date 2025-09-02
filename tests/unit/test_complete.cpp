#include "include/token.h"
#include "include/lexer.h"
#include "include/parser.h"

// Include implementations since this is a single-file test
#include "lexer/lexer_complete.cpp"
#include "parser/parser_new.cpp"

#include <iostream>
#include <vector>

int main() {
    try {
        // Test lexer
        std::string code = R"(
component HelloWorld {
    name: "world"
    
    render: 
        <div class="greeting">
            Hello {!name}!
        </div>
}
)";

        std::cout << "Testing Alterion lexer and parser..." << std::endl;
        
        // Create lexer and tokenize
        Lexer lexer(code);
        std::vector<Token> tokens = lexer.tokenize();
        
        std::cout << "Lexer produced " << tokens.size() << " tokens" << std::endl;
        
        // Test parser
        Parser parser(std::move(tokens));
        auto ast = parser.parse();
        
        if (ast) {
            std::cout << "Parser succeeded! AST created." << std::endl;
            std::cout << "Program has " << ast->components.size() << " components" << std::endl;
        } else {
            std::cout << "Parser failed - no AST created" << std::endl;
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
