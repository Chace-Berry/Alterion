#include "../include/code_generator.h"
#include "../lexer/lexer_complete.cpp"
#include "../parser/parser_complete.cpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <chrono>

namespace Alterion {

struct CompilerOptions {
    std::string inputFile;
    std::string outputFile;
    std::string outputDir = "./dist";
    CodeGenConfig::Target target = CodeGenConfig::Target::TypeScript;
    CodeGenConfig::ModuleSystem moduleSystem = CodeGenConfig::ModuleSystem::ESModules;
    bool watch = false;
    bool minify = false;
    bool sourceMaps = true;
    bool verbose = false;
    bool optimize = false;
    std::vector<std::string> includePaths;
};

class AlterionCompiler {
private:
    CompilerOptions options;
    
    void printUsage() {
        std::cout << R"(
Alterion Compiler - Phase 2D Code Generation

USAGE:
    alterion [OPTIONS] <input-file>

OPTIONS:
    -o, --output <file>     Output file path
    -d, --output-dir <dir>  Output directory (default: ./dist)
    -t, --target <target>   Target language (js, ts, react, rn)
    -m, --module <system>   Module system (es6, cjs, amd, umd)
    --watch                 Watch for file changes
    --minify                Minify output
    --no-source-maps        Disable source maps
    --optimize              Enable optimizations
    -v, --verbose           Verbose output
    -h, --help              Show this help

TARGETS:
    js, javascript          Generate JavaScript (ES6+)
    ts, typescript          Generate TypeScript (default)
    react                   Generate React components
    rn, react-native        Generate React Native components

EXAMPLES:
    alterion app.alt                          # Compile to TypeScript
    alterion -t js -o app.js app.alt         # Compile to JavaScript
    alterion -t react --optimize app.alt     # Optimized React build
    alterion --watch src/                    # Watch directory for changes

)";
    }
    
    bool parseArguments(int argc, char* argv[]) {
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            
            if (arg == "-h" || arg == "--help") {
                printUsage();
                return false;
            }
            else if (arg == "-o" || arg == "--output") {
                if (++i >= argc) {
                    std::cerr << "Error: --output requires a file path\n";
                    return false;
                }
                options.outputFile = argv[i];
            }
            else if (arg == "-d" || arg == "--output-dir") {
                if (++i >= argc) {
                    std::cerr << "Error: --output-dir requires a directory path\n";
                    return false;
                }
                options.outputDir = argv[i];
            }
            else if (arg == "-t" || arg == "--target") {
                if (++i >= argc) {
                    std::cerr << "Error: --target requires a target type\n";
                    return false;
                }
                std::string target = argv[i];
                if (target == "js" || target == "javascript") {
                    options.target = CodeGenConfig::Target::JavaScript;
                } else if (target == "ts" || target == "typescript") {
                    options.target = CodeGenConfig::Target::TypeScript;
                } else if (target == "react") {
                    options.target = CodeGenConfig::Target::React;
                } else if (target == "rn" || target == "react-native") {
                    options.target = CodeGenConfig::Target::ReactNative;
                } else {
                    std::cerr << "Error: Unknown target '" << target << "'\n";
                    return false;
                }
            }
            else if (arg == "-m" || arg == "--module") {
                if (++i >= argc) {
                    std::cerr << "Error: --module requires a module system\n";
                    return false;
                }
                std::string module = argv[i];
                if (module == "es6" || module == "esm") {
                    options.moduleSystem = CodeGenConfig::ModuleSystem::ESModules;
                } else if (module == "cjs" || module == "commonjs") {
                    options.moduleSystem = CodeGenConfig::ModuleSystem::CommonJS;
                } else if (module == "amd") {
                    options.moduleSystem = CodeGenConfig::ModuleSystem::AMD;
                } else if (module == "umd") {
                    options.moduleSystem = CodeGenConfig::ModuleSystem::UMD;
                } else {
                    std::cerr << "Error: Unknown module system '" << module << "'\n";
                    return false;
                }
            }
            else if (arg == "--watch") {
                options.watch = true;
            }
            else if (arg == "--minify") {
                options.minify = true;
            }
            else if (arg == "--no-source-maps") {
                options.sourceMaps = false;
            }
            else if (arg == "--optimize") {
                options.optimize = true;
            }
            else if (arg == "-v" || arg == "--verbose") {
                options.verbose = true;
            }
            else if (arg.starts_with("-")) {
                std::cerr << "Error: Unknown option '" << arg << "'\n";
                return false;
            }
            else {
                // Input file
                options.inputFile = arg;
            }
        }
        
        if (options.inputFile.empty()) {
            std::cerr << "Error: No input file specified\n";
            printUsage();
            return false;
        }
        
        return true;
    }
    
    std::string readFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filename);
        }
        
        return std::string(std::istreambuf_iterator<char>(file),
                          std::istreambuf_iterator<char>());
    }
    
    void writeFile(const std::string& filename, const std::string& content) {
        // Create directory if it doesn't exist
        std::filesystem::path filepath(filename);
        std::filesystem::create_directories(filepath.parent_path());
        
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot write to file: " + filename);
        }
        
        file << content;
        file.close();
        
        if (options.verbose) {
            std::cout << "Generated: " << filename << " (" << content.length() << " bytes)\n";
        }
    }
    
    std::string getOutputExtension() {
        switch (options.target) {
            case CodeGenConfig::Target::JavaScript:
            case CodeGenConfig::Target::React:
                return ".js";
            case CodeGenConfig::Target::TypeScript:
                return ".ts";
            case CodeGenConfig::Target::ReactNative:
                return ".tsx";
            default:
                return ".js";
        }
    }
    
    std::string determineOutputFile() {
        if (!options.outputFile.empty()) {
            return options.outputFile;
        }
        
        std::filesystem::path inputPath(options.inputFile);
        std::string baseName = inputPath.stem().string();
        std::string extension = getOutputExtension();
        
        return options.outputDir + "/" + baseName + extension;
    }
    
    CompilationResult compileFile(const std::string& inputFile) {
        auto startTime = std::chrono::high_resolution_clock::now();
        
        CompilationResult result;
        result.inputFile = inputFile;
        result.success = false;
        
        try {
            // Read source file
            std::string sourceCode = readFile(inputFile);
            
            if (options.verbose) {
                std::cout << "Compiling: " << inputFile << " (" << sourceCode.length() << " bytes)\n";
            }
            
            // Tokenize
            Lexer lexer(sourceCode);
            auto tokens = lexer.tokenize();
            
            // Check for lexer errors
            for (const auto& token : tokens) {
                if (token.type == TokenType::Error) {
                    result.errors.push_back("Lexer error: " + token.errorMessage);
                }
            }
            
            if (!result.errors.empty()) {
                return result;
            }
            
            // Parse
            Parser parser(std::move(tokens));
            auto ast = parser.parse();
            
            if (!ast) {
                result.errors.push_back("Failed to parse input file");
                return result;
            }
            
            // Generate code
            CodeGenConfig config;
            config.target = options.target;
            config.moduleSystem = options.moduleSystem;
            config.minify = options.minify;
            config.generateSourceMaps = options.sourceMaps;
            
            std::unique_ptr<CodeGenerator> generator;
            
            if (options.optimize) {
                generator = std::make_unique<OptimizedCodeGenerator>(config);
            } else if (options.target == CodeGenConfig::Target::ReactNative) {
                generator = std::make_unique<ReactNativeGenerator>(config);
            } else {
                generator = std::make_unique<CodeGenerator>(config);
            }
            
            auto generatedCode = generator->generate(*ast);
            
            // Write output
            result.outputFile = determineOutputFile();
            writeFile(result.outputFile, generatedCode.code);
            
            // Write source map if enabled
            if (options.sourceMaps && !generatedCode.sourceMaps.empty()) {
                std::string sourceMapFile = result.outputFile + ".map";
                // Generate source map content (simplified)
                std::string sourceMapContent = generateSourceMap(generatedCode, inputFile);
                writeFile(sourceMapFile, sourceMapContent);
            }
            
            result.success = true;
            
        } catch (const std::exception& e) {
            result.errors.push_back(std::string("Compilation error: ") + e.what());
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        result.compilationTime = duration.count();
        
        return result;
    }
    
    std::string generateSourceMap(const GeneratedCode& code, const std::string& sourceFile) {
        // Simplified source map generation
        return R"({
  "version": 3,
  "sources": [")" + sourceFile + R"("],
  "names": [],
  "mappings": "",
  "file": ")" + determineOutputFile() + R"("
})";
    }
    
public:
    struct CompilationResult {
        std::string inputFile;
        std::string outputFile;
        bool success;
        std::vector<std::string> errors;
        long long compilationTime; // milliseconds
    };
    
    int run(int argc, char* argv[]) {
        if (!parseArguments(argc, argv)) {
            return 1;
        }
        
        if (options.verbose) {
            std::cout << "Alterion Compiler v2.0 - Phase 2D Code Generation\n";
            std::cout << "Target: " << getTargetName() << "\n";
            std::cout << "Module: " << getModuleName() << "\n";
            std::cout << "Output: " << determineOutputFile() << "\n\n";
        }
        
        if (options.watch) {
            return runWatchMode();
        } else {
            return runSingleCompilation();
        }
    }
    
private:
    std::string getTargetName() {
        switch (options.target) {
            case CodeGenConfig::Target::JavaScript: return "JavaScript";
            case CodeGenConfig::Target::TypeScript: return "TypeScript";
            case CodeGenConfig::Target::React: return "React";
            case CodeGenConfig::Target::ReactNative: return "React Native";
            default: return "Unknown";
        }
    }
    
    std::string getModuleName() {
        switch (options.moduleSystem) {
            case CodeGenConfig::ModuleSystem::ESModules: return "ES Modules";
            case CodeGenConfig::ModuleSystem::CommonJS: return "CommonJS";
            case CodeGenConfig::ModuleSystem::AMD: return "AMD";
            case CodeGenConfig::ModuleSystem::UMD: return "UMD";
            default: return "Unknown";
        }
    }
    
    int runSingleCompilation() {
        auto result = compileFile(options.inputFile);
        
        if (result.success) {
            std::cout << "✅ Compilation successful!\n";
            std::cout << "Output: " << result.outputFile << "\n";
            std::cout << "Time: " << result.compilationTime << "ms\n";
            return 0;
        } else {
            std::cout << "❌ Compilation failed!\n";
            for (const auto& error : result.errors) {
                std::cerr << "Error: " << error << "\n";
            }
            return 1;
        }
    }
    
    int runWatchMode() {
        std::cout << "👀 Watching for changes in: " << options.inputFile << "\n";
        std::cout << "Press Ctrl+C to stop...\n\n";
        
        auto lastWriteTime = std::filesystem::last_write_time(options.inputFile);
        
        while (true) {
            try {
                auto currentWriteTime = std::filesystem::last_write_time(options.inputFile);
                
                if (currentWriteTime != lastWriteTime) {
                    std::cout << "🔄 File changed, recompiling...\n";
                    auto result = compileFile(options.inputFile);
                    
                    if (result.success) {
                        std::cout << "✅ Recompiled successfully (" << result.compilationTime << "ms)\n\n";
                    } else {
                        std::cout << "❌ Recompilation failed:\n";
                        for (const auto& error : result.errors) {
                            std::cerr << "  " << error << "\n";
                        }
                        std::cout << "\n";
                    }
                    
                    lastWriteTime = currentWriteTime;
                }
                
                // Sleep for 100ms
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                
            } catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "File system error: " << e.what() << "\n";
                break;
            }
        }
        
        return 0;
    }
};

} // namespace Alterion

int main(int argc, char* argv[]) {
    Alterion::AlterionCompiler compiler;
    return compiler.run(argc, argv);
}
