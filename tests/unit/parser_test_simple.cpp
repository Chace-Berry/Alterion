#include "include/parser.h"
#include "include/token.h"
#include <iostream>
#include <vector>

int main() {
    std::vector<Token> tokens;
    tokens.emplace_back(TokenType::Identifier, "test", 1, 1);
    tokens.emplace_back(TokenType::EOFToken, "", 1, 5);
    
    try {
        Parser parser(std::move(tokens));
        std::cout << "Parser created successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }
}
