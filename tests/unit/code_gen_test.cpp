#include "../include/code_generator.h"
#include "../include/ast_complete.h"
#include "../lexer/lexer_complete.cpp"
#include "../parser/parser_complete.cpp"
#include <iostream>
#include <cassert>
#include <string>

namespace Alterion {

class CodeGenTest {
private:
    int testCount = 0;
    int passedTests = 0;
    
    void assert_equal(const std::string& expected, const std::string& actual, const std::string& testName) {
        testCount++;
        if (expected == actual) {
            passedTests++;
            std::cout << "✅ " << testName << " PASSED\n";
        } else {
            std::cout << "❌ " << testName << " FAILED\n";
            std::cout << "Expected:\n" << expected << "\n";
            std::cout << "Actual:\n" << actual << "\n";
            std::cout << "---\n";
        }
    }
    
    void assert_contains(const std::string& expected, const std::string& actual, const std::string& testName) {
        testCount++;
        if (actual.find(expected) != std::string::npos) {
            passedTests++;
            std::cout << "✅ " << testName << " PASSED\n";
        } else {
            std::cout << "❌ " << testName << " FAILED\n";
            std::cout << "Expected to contain: " << expected << "\n";
            std::cout << "Actual: " << actual << "\n";
            std::cout << "---\n";
        }
    }
    
    std::unique_ptr<Program> parseAlterion(const std::string& code) {
        try {
            Lexer lexer(code);
            auto tokens = lexer.tokenize();
            Parser parser(std::move(tokens));
            return parser.parse();
        } catch (const std::exception& e) {
            std::cerr << "Parse error: " << e.what() << std::endl;
            return nullptr;
        }
    }
    
public:
    void runAllTests() {
        std::cout << "🚀 Running Code Generation Tests...\n\n";
        
        testBasicExpressions();
        testSimpleComponent();
        testComponentWithState();
        testComponentWithMethods();
        testALTXGeneration();
        testAsyncBlocks();
        testImportsExports();
        testTypeScriptGeneration();
        testReactNativeGeneration();
        testOptimizedGeneration();
        
        std::cout << "\n📊 Test Results: " << passedTests << "/" << testCount << " tests passed\n";
        
        if (passedTests == testCount) {
            std::cout << "🎉 All tests passed! Code generation is working perfectly!\n";
        } else {
            std::cout << "⚠️  Some tests failed. Please review the implementation.\n";
        }
    }
    
    void testBasicExpressions() {
        std::cout << "Testing basic expressions...\n";
        
        CodeGenConfig config;
        config.target = CodeGenConfig::Target::JavaScript;
        CodeGenerator generator(config);
        
        // Test string literal
        auto program = parseAlterion(R"(
            function test() {
                return "hello world";
            }
        )");
        
        if (program) {
            auto result = generator.generate(*program);
            assert_contains("\"hello world\"", result.code, "String literal generation");
            assert_contains("function test()", result.code, "Function generation");
        }
    }
    
    void testSimpleComponent() {
        std::cout << "\nTesting simple component generation...\n";
        
        CodeGenConfig config;
        config.target = CodeGenConfig::Target::TypeScript;
        CodeGenerator generator(config);
        
        auto program = parseAlterion(R"(
            component HelloWorld {
                render:
                    <div>Hello, World!</div>
            }
        )");
        
        if (program) {
            auto result = generator.generate(*program);
            
            // Check for React component structure
            assert_contains("const HelloWorld", result.code, "Component declaration");
            assert_contains("React.FC", result.code, "TypeScript React.FC type");
            assert_contains("return (", result.code, "Return statement");
            assert_contains("<div>Hello, World!</div>", result.code, "JSX content");
        }
    }
    
    void testComponentWithState() {
        std::cout << "\nTesting component with state...\n";
        
        CodeGenConfig config;
        config.target = CodeGenConfig::Target::TypeScript;
        CodeGenerator generator(config);
        
        auto program = parseAlterion(R"(
            component Counter {
                count = 0
                
                render:
                    <div>Count: {count}</div>
            }
        )");
        
        if (program) {
            auto result = generator.generate(*program);
            
            assert_contains("React.useState", result.code, "useState hook");
            assert_contains("const [count, setCount]", result.code, "State variable declaration");
            assert_contains("Count: {count}", result.code, "State interpolation in JSX");
        }
    }
    
    void testComponentWithMethods() {
        std::cout << "\nTesting component with methods...\n";
        
        CodeGenConfig config;
        config.target = CodeGenConfig::Target::TypeScript;
        CodeGenerator generator(config);
        
        auto program = parseAlterion(R"(
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
        )");
        
        if (program) {
            auto result = generator.generate(*program);
            
            assert_contains("const increment = ", result.code, "Method as function");
            assert_contains("onClick={increment}", result.code, "Method reference in JSX");
            assert_contains("setCount", result.code, "State setter in method");
        }
    }
    
    void testALTXGeneration() {
        std::cout << "\nTesting ALTX/JSX generation...\n";
        
        CodeGenConfig config;
        config.target = CodeGenConfig::Target::TypeScript;
        CodeGenerator generator(config);
        
        auto program = parseAlterion(R"(
            component App {
                render:
                    <div class="container" center>
                        <h1 style="color: blue">Title</h1>
                        <button onClick={handleClick}>Click me</button>
                        <img src={imageUrl} alt="test" />
                    </div>
            }
        )");
        
        if (program) {
            auto result = generator.generate(*program);
            
            assert_contains("className=\"container\"", result.code, "Class attribute conversion");
            assert_contains("style={{color: 'blue'}}", result.code, "Style attribute conversion");
            assert_contains("onClick={handleClick}", result.code, "Event handler");
            assert_contains("src={imageUrl}", result.code, "Expression attribute");
            assert_contains("style={{display: 'flex'", result.code, "Center modifier conversion");
        }
    }
    
    void testAsyncBlocks() {
        std::cout << "\nTesting async block generation...\n";
        
        CodeGenConfig config;
        config.target = CodeGenConfig::Target::JavaScript;
        CodeGenerator generator(config);
        
        auto program = parseAlterion(R"(
            component DataLoader {
                loadData {
                    async {
                        let response = await fetch("/api/data");
                        let data = await response.json();
                        return data;
                    }
                }
            }
        )");
        
        if (program) {
            auto result = generator.generate(*program);
            
            assert_contains("(async () => {", result.code, "Async IIFE wrapper");
            assert_contains("await fetch", result.code, "Await expression");
            assert_contains("})();", result.code, "IIFE closing");
        }
    }
    
    void testImportsExports() {
        std::cout << "\nTesting import/export generation...\n";
        
        CodeGenConfig config;
        config.target = CodeGenConfig::Target::TypeScript;
        config.moduleSystem = CodeGenConfig::ModuleSystem::ESModules;
        CodeGenerator generator(config);
        
        auto program = parseAlterion(R"(
            import { useState, useEffect } from "react";
            
            export default component MyComponent {
                render: <div>Test</div>
            }
        )");
        
        if (program) {
            auto result = generator.generate(*program);
            
            assert_contains("import { useState, useEffect } from \"react\"", result.code, "ES6 import");
            assert_contains("export default", result.code, "Default export");
        }
        
        // Test CommonJS
        config.moduleSystem = CodeGenConfig::ModuleSystem::CommonJS;
        CodeGenerator generatorCJS(config);
        
        if (program) {
            auto result = generatorCJS.generate(*program);
            
            assert_contains("require(\"react\")", result.code, "CommonJS require");
            assert_contains("module.exports", result.code, "CommonJS export");
        }
    }
    
    void testTypeScriptGeneration() {
        std::cout << "\nTesting TypeScript-specific generation...\n";
        
        CodeGenConfig config;
        config.target = CodeGenConfig::Target::TypeScript;
        config.generateTypeDefinitions = true;
        CodeGenerator generator(config);
        
        auto program = parseAlterion(R"(
            component UserProfile {
                name: string = ""
                age: number = 0
                
                updateName(newName: string) {
                    name = newName
                }
                
                render:
                    <div>
                        <h1>{name}</h1>
                        <p>Age: {age}</p>
                    </div>
            }
        )");
        
        if (program) {
            auto result = generator.generate(*program);
            
            assert_contains("interface UserProfileProps", result.code, "Props interface");
            assert_contains("React.FC<UserProfileProps>", result.code, "Typed component");
            assert_contains("newName: string", result.code, "Parameter types");
            assert_contains("React.useState<string>", result.code, "Typed useState");
        }
    }
    
    void testReactNativeGeneration() {
        std::cout << "\nTesting React Native generation...\n";
        
        CodeGenConfig config;
        config.target = CodeGenConfig::Target::ReactNative;
        ReactNativeGenerator generator(config);
        
        auto program = parseAlterion(R"(
            component MobileApp {
                render:
                    <div>
                        <h1>Title</h1>
                        <button>Click me</button>
                        <img src="image.png" />
                    </div>
            }
        )");
        
        if (program) {
            auto result = generator.generate(*program);
            
            assert_contains("<View>", result.code, "div -> View mapping");
            assert_contains("<Text>Title</Text>", result.code, "h1 -> Text mapping");
            assert_contains("<TouchableOpacity>", result.code, "button -> TouchableOpacity mapping");
            assert_contains("<Image", result.code, "img -> Image mapping");
        }
    }
    
    void testOptimizedGeneration() {
        std::cout << "\nTesting optimized generation...\n";
        
        CodeGenConfig config;
        config.target = CodeGenConfig::Target::TypeScript;
        OptimizedCodeGenerator generator(config);
        
        auto program = parseAlterion(R"(
            component ExpensiveComponent {
                data = []
                loading = false
                error = null
                
                fetchData { /* complex method */ }
                processData { /* complex method */ }
                validateData { /* complex method */ }
                renderData { /* complex method */ }
                
                render:
                    <div>Complex component</div>
            }
        )");
        
        if (program) {
            auto result = generator.generate(*program);
            
            assert_contains("React.memo", result.code, "Component memoization");
        }
    }
};

} // namespace Alterion

int main() {
    Alterion::CodeGenTest test;
    test.runAllTests();
    return 0;
}
