#include "lexer.h"
#include <iostream>
#include <vector>

int main() {
    try {
        // Test lexer only
        std::string code = R"(
component HelloWorld {
    name: "world"
}
)";
        
        Lexer lexer(code);
        std::vector<Token> tokens = lexer.tokenize();
        
        std::cout << "Lexing successful! Generated " << tokens.size() << " tokens." << std::endl;
        
        // Print first few tokens to verify
        for (size_t i = 0; i < std::min(tokens.size(), size_t(10)); ++i) {
            std::cout << "Token " << i << ": " << tokens[i].value << " (" << static_cast<int>(tokens[i].type) << ")" << std::endl;
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
