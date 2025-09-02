#include "include/lexer.h"
#include "include/parser.h"
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
            Hello {name}!
        </div>
}
)";
        
        Lexer lexer(code);
        std::vector<Token> tokens = lexer.tokenize();
        
        std::cout << "Lexing successful! Generated " << tokens.size() << " tokens." << std::endl;
        
        // Test parser
        Parser parser(tokens);
        auto program = parser.parse();
        
        std::cout << "Parsing successful! Generated AST with " << program->components.size() << " components." << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
