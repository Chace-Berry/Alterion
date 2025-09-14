#include "../../core/include/ast.h"
#include "../../core/include/lexer.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <filesystem>


#include "../../core/include/parser.h"

int main(int argc, char** argv) {
    std::cerr << "[DEBUG] Program started\n";
    std::cerr << "[DEBUG] argc = " << argc << std::endl;
    
    std::string inputFile = argc > 1 ? argv[1] : "examples/lexer-app-test.alt";
    std::cerr << "[DEBUG] Input file: " << inputFile << std::endl;
    
    std::ifstream file(inputFile);
    if (!file) {
        std::cerr << "[ERROR] Failed to open " << inputFile << std::endl;
        std::cerr << "[DEBUG] Current working directory check..." << std::endl;
        try {
            std::cerr << "[DEBUG] Current path: " << std::filesystem::current_path() << std::endl;
        } catch (...) {
            std::cerr << "[DEBUG] Could not get current path" << std::endl;
        }
        return 1;
    }
    std::cerr << "[DEBUG] File opened successfully\n";
    
    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    std::cerr << "[DEBUG] Input size: " << source.size() << " bytes\n";

    if (source.empty()) {
        std::cerr << "[WARNING] Input file is empty!\n";
    } else {
        std::cerr << "[DEBUG] First 50 chars: " << source.substr(0, 50) << "...\n";
    }

    std::cerr << "[DEBUG] Creating lexer...\n";
    try {
        Lexer lexer(source);
        std::cerr << "[DEBUG] Lexer created, tokenizing...\n";
        std::vector<Token> tokens = lexer.tokenize();
        std::cerr << "[DEBUG] Token count: " << tokens.size() << "\n";

        if (tokens.empty()) {
            std::cerr << "[WARNING] No tokens generated!\n";
        } else {
            std::cerr << "[DEBUG] First token: type=" << (int)tokens[0].type << ", value=\"" << tokens[0].value << "\"\n";
        }

        
        std::cerr << "[DEBUG] Parsing tokens to AST using Parser...\n";
        Parser parser(tokens);
        auto ast = parser.parse();

        if (!ast) {
            std::cerr << "[ERROR] Parser returned null AST!\n";
            return 3;
        }

        std::cerr << "[DEBUG] AST root type: " << (int)ast->type << " (" << ast->value.value_or("") << ")\n";
        std::cerr << "[DEBUG] AST children count: " << ast->children.size() << "\n";

        
        std::cerr << "[DEBUG] First 3 AST nodes:\n";
        for (size_t i = 0; i < std::min((size_t)3, ast->children.size()); i++) {
            auto& child = ast->children[i];
            std::cerr << "  [" << i << "] Type: " << (int)child->type
                      << ", Value: \"" << child->value.value_or("")
                      << "\", Line: " << child->line
                      << ", Column: " << child->column << "\n";
        }

        
        const char* outPath = "results-dashboard/public/results/parser-results.json";
        std::cerr << "[DEBUG] Output path: " << outPath << "\n";
        std::filesystem::path outputPath(outPath);
        
        try {
            std::filesystem::create_directories(outputPath.parent_path());
            std::cerr << "[DEBUG] Output directory created/verified\n";
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] Could not create directory: " << e.what() << "\n";
            return 4;
        }
        
        std::ofstream out(outPath);
        if (!out) {
            std::cerr << "[ERROR] Could not open " << outPath << " for writing!\n";
            return 2;
        }
        std::cerr << "[DEBUG] Output file opened successfully\n";
        
        std::cerr << "[DEBUG] Calling astToJson...\n";
        try {
            std::string jsonOutput = astToJson(ast);
            std::cerr << "[DEBUG] Generated JSON length: " << jsonOutput.length() << " characters\n";
            
            if (jsonOutput.empty()) {
                std::cerr << "[WARNING] astToJson returned empty string!\n";
            } else {
                std::cerr << "[DEBUG] JSON preview (first 100 chars): " << jsonOutput.substr(0, 100);
                if (jsonOutput.length() > 100) std::cerr << "...";
                std::cerr << "\n";
            }
            
            
            out << jsonOutput;
            out.close();
            std::cerr << "[DEBUG] JSON written to file\n";
            
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] Exception in astToJson: " << e.what() << "\n";
            return 5;
        } catch (...) {
            std::cerr << "[ERROR] Unknown exception in astToJson\n";
            return 6;
        }
        
        std::cerr << "[SUCCESS] AST test complete. Output written to " << outPath << "\n";
        
        
        std::ifstream verify(outPath);
        if (verify) {
            verify.seekg(0, std::ios::end);
            size_t fileSize = verify.tellg();
            std::cerr << "[VERIFY] Output file size: " << fileSize << " bytes\n";
            verify.close();
        } else {
            std::cerr << "[WARNING] Could not verify output file\n";
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Exception: " << e.what() << std::endl;
        return 7;
    } catch (...) {
        std::cerr << "[ERROR] Unknown exception occurred\n";
        return 8;
    }
    
    std::cerr << "[DEBUG] Program completed successfully\n";
    return 0;
}