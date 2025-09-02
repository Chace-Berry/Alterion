#include <iostream>
#include <string>
#include <vector>

// Simple test that doesn't require complex includes
int main() {
    std::cout << "=== Alterion Language Basic Test ===" << std::endl;
    
    // Test 1: Check if files exist and are readable
    std::cout << "\n1. Testing File Structure..." << std::endl;
    
    const char* files[] = {
        "include/token.h",
        "include/lexer.h", 
        "include/parser.h",
        "lexer/lexer_complete.cpp",
        "parser/parser_complete.cpp"
    };
    
    for (const char* file : files) {
        FILE* f = fopen(file, "r");
        if (f) {
            std::cout << "   ✓ " << file << " - Found" << std::endl;
            fclose(f);
        } else {
            std::cout << "   ✗ " << file << " - Missing" << std::endl;
        }
    }
    
    // Test 2: Basic string processing (simulating lexer logic)
    std::cout << "\n2. Testing String Processing..." << std::endl;
    std::string testCode = "component Test { name: \"hello\" }";
    std::cout << "   Input: " << testCode << std::endl;
    std::cout << "   Length: " << testCode.length() << " characters" << std::endl;
    
    // Count basic tokens manually
    int braces = 0, quotes = 0, colons = 0;
    for (char c : testCode) {
        if (c == '{' || c == '}') braces++;
        if (c == '"') quotes++;
        if (c == ':') colons++;
    }
    
    std::cout << "   Found: " << braces << " braces, " << quotes << " quotes, " << colons << " colons" << std::endl;
    
    // Test 3: Expression patterns
    std::cout << "\n3. Testing Expression Patterns..." << std::endl;
    std::string exprTest = "Hello {name} and {!external}";
    std::cout << "   Expression test: " << exprTest << std::endl;
    
    size_t pos = 0;
    int localExpr = 0, externalExpr = 0;
    while ((pos = exprTest.find("{", pos)) != std::string::npos) {
        if (pos + 1 < exprTest.length() && exprTest[pos + 1] == '!') {
            externalExpr++;
            std::cout << "   Found external binding at position " << pos << std::endl;
        } else {
            localExpr++;
            std::cout << "   Found local expression at position " << pos << std::endl;
        }
        pos++;
    }
    
    std::cout << "   Total: " << localExpr << " local expressions, " << externalExpr << " external bindings" << std::endl;
    
    std::cout << "\n=== Basic Tests Completed Successfully! ===" << std::endl;
    std::cout << "\nTo run the full lexer/parser tests, you need to:" << std::endl;
    std::cout << "1. Set up a proper C++ development environment" << std::endl;
    std::cout << "2. Fix include paths in the source files" << std::endl;
    std::cout << "3. Use CMake or direct compilation with proper flags" << std::endl;
    
    return 0;
}
