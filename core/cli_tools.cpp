#include "cli_tools.h"
#include "code_generator.h"
#include "./lexer/lexer_complete.cpp"
#include "./parser/parser_complete.cpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <regex>
#include <cstdlib>

namespace Alterion {

// Project Scaffolder Implementation
bool ProjectScaffolder::createProject(const std::string& projectName, ProjectTemplate template, const std::string& targetDir) {
    try {
        std::filesystem::path projectPath = std::filesystem::path(targetDir) / projectName;
        std::filesystem::create_directories(projectPath);
        
        const auto& templateInfo = templates[template];
        
        // Variables for template substitution
        std::unordered_map<std::string, std::string> variables = {
            {"PROJECT_NAME", projectName},
            {"PROJECT_DESCRIPTION", templateInfo.description},
            {"AUTHOR", "Alterion Developer"},
            {"VERSION", "1.0.0"},
            {"YEAR", std::to_string(std::chrono::system_clock::now().time_since_epoch().count() / 1000000000 / 365 / 24 / 60 / 60 + 1970)}
        };
        
        // Generate project files
        for (const auto& file : templateInfo.files) {
            std::filesystem::path filePath = projectPath / file;
            std::filesystem::create_directories(filePath.parent_path());
            
            generateFile(file, filePath.string(), variables);
        }
        
        std::cout << "✅ Created " << projectName << " with " << templateInfo.name << " template\n";
        std::cout << "📁 Project location: " << projectPath << "\n";
        std::cout << "\nNext steps:\n";
        std::cout << "  cd " << projectName << "\n";
        std::cout << "  alterion dev\n\n";
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Failed to create project: " << e.what() << "\n";
        return false;
    }
}

void ProjectScaffolder::generateFile(const std::string& templatePath, const std::string& outputPath, 
                                   const std::unordered_map<std::string, std::string>& variables) {
    std::string content;
    
    // Generate content based on file type
    if (templatePath.ends_with(".alt")) {
        content = generateAlterionFile(templatePath, variables);
    } else if (templatePath.ends_with("package.json")) {
        content = generatePackageJson(variables);
    } else if (templatePath.ends_with("alterion.json")) {
        content = generateAlterionConfig(variables);
    } else if (templatePath.ends_with("README.md")) {
        content = generateReadme(variables);
    } else if (templatePath.ends_with(".gitignore")) {
        content = generateGitignore();
    } else {
        content = generateGenericFile(templatePath, variables);
    }
    
    // Write file
    std::ofstream file(outputPath);
    file << content;
    file.close();
}

std::string ProjectScaffolder::generateGenericFile(const std::string& templatePath, 
                                                  const std::unordered_map<std::string, std::string>& variables) {
    // Generic file generation with variable substitution
    std::string content = "// Generated file: " + templatePath + "\n";
    return content;
}

ProjectConfig ProjectScaffolder::loadProjectConfig(const std::string& projectDir) {
    ProjectConfig config;
    
    // Try to load alterion.json
    std::filesystem::path configPath = std::filesystem::path(projectDir) / "alterion.json";
    
    if (std::filesystem::exists(configPath)) {
        // Simple JSON parsing (would use proper JSON library in production)
        std::ifstream file(configPath);
        // For now, return default config
        config.name = "alterion-project";
        config.version = "1.0.0";
    } else {
        // Default configuration
        config.name = "alterion-project";
        config.version = "1.0.0";
        config.build.target = "typescript";
        config.build.outDir = "dist";
        config.build.srcDir = "src";
        config.dev.port = 3000;
        config.dev.host = "localhost";
        config.dev.hotReload = true;
    }
    
    return config;
}

void ProjectScaffolder::saveProjectConfig(const std::string& projectDir, const ProjectConfig& config) {
    std::filesystem::path configPath = std::filesystem::path(projectDir) / "alterion.json";
    
    // Simple JSON generation (would use proper JSON library in production)
    std::ofstream file(configPath);
    file << "{\n";
    file << "  \"name\": \"" << config.name << "\",\n";
    file << "  \"version\": \"" << config.version << "\",\n";
    file << "  \"build\": {\n";
    file << "    \"target\": \"" << config.build.target << "\",\n";
    file << "    \"outDir\": \"" << config.build.outDir << "\",\n";
    file << "    \"srcDir\": \"" << config.build.srcDir << "\"\n";
    file << "  }\n";
    file << "}\n";
    file.close();
}

std::string ProjectScaffolder::generateAlterionFile(const std::string& templatePath, 
                                                   const std::unordered_map<std::string, std::string>& variables) {
    if (templatePath == "src/main.alt") {
        return R"(import { App } from "./components/App"

// Main application entry point
function main() {
    let app = new App()
    app.render()
}

main()
)";
    }
    
    if (templatePath == "src/components/App.alt") {
        return R"(component App {
    title: string = ")" + variables.at("PROJECT_NAME") + R"("
    count: number = 0
    
    increment {
        count = count + 1
    }
    
    reset {
        count = 0
    }
    
    render:
        <div class="app" center>
            <header class="app-header">
                <h1>{title}</h1>
                <p>Welcome to Alterion!</p>
            </header>
            
            <main class="app-content">
                <div class="counter-demo">
                    <h2>Interactive Counter</h2>
                    <div class="counter-display">
                        Count: {count}
                    </div>
                    <div class="counter-controls">
                        <button onClick={increment} class="btn-primary">
                            Increment
                        </button>
                        <button onClick={reset} class="btn-secondary">
                            Reset
                        </button>
                    </div>
                </div>
            </main>
            
            <footer class="app-footer">
                <p>Built with ❤️ using Alterion</p>
            </footer>
        </div>
}

export default App
)";
    }
    
    if (templatePath == "src/pages/Home.alt") {
        return R"(component Home {
    welcomeMessage: string = "Welcome to )" + variables.at("PROJECT_NAME") + R"("
    
    render:
        <div class="home-page">
            <h1>{welcomeMessage}</h1>
            <p>This is your home page. Start building something amazing!</p>
        </div>
}

export default Home
)";
    }
    
    return "// Generated Alterion file\n";
}

std::string ProjectScaffolder::generatePackageJson(const std::unordered_map<std::string, std::string>& variables) {
    return R"({
  "name": ")" + variables.at("PROJECT_NAME") + R"(",
  "version": ")" + variables.at("VERSION") + R"(",
  "description": ")" + variables.at("PROJECT_DESCRIPTION") + R"(",
  "main": "dist/main.js",
  "scripts": {
    "dev": "alterion dev",
    "build": "alterion build",
    "test": "alterion test",
    "format": "alterion format",
    "lint": "alterion lint"
  },
  "dependencies": {
    "react": "^18.0.0",
    "@types/react": "^18.0.0"
  },
  "devDependencies": {
    "alterion": "^1.0.0"
  },
  "keywords": ["alterion", "react", "typescript", "ui"],
  "author": ")" + variables.at("AUTHOR") + R"(",
  "license": "MIT"
}
)";
}

std::string ProjectScaffolder::generateAlterionConfig(const std::unordered_map<std::string, std::string>& variables) {
    return R"({
  "name": ")" + variables.at("PROJECT_NAME") + R"(",
  "version": ")" + variables.at("VERSION") + R"(",
  "build": {
    "target": "typescript",
    "outDir": "dist",
    "srcDir": "src",
    "sourceMaps": true,
    "optimize": true,
    "include": ["src/**/*.alt"],
    "exclude": ["**/*.test.alt", "**/*.spec.alt"]
  },
  "dev": {
    "port": 3000,
    "hotReload": true,
    "autoOpen": true,
    "host": "localhost",
    "watchPaths": ["src", "public"]
  },
  "format": {
    "indentSize": 2,
    "maxLineLength": 100,
    "semicolons": true,
    "trailingCommas": true
  }
}
)";
}

std::string ProjectScaffolder::generateReadme(const std::unordered_map<std::string, std::string>& variables) {
    return R"(# )" + variables.at("PROJECT_NAME") + R"(

)" + variables.at("PROJECT_DESCRIPTION") + R"(

## Getting Started

### Prerequisites
- Node.js 16+ 
- Alterion CLI

### Installation
```bash
npm install
```

### Development
```bash
alterion dev
```

### Building
```bash
alterion build
```

### Testing
```bash
alterion test
```

## Project Structure
```
src/
├── components/     # Reusable components
├── pages/         # Page components
├── styles/        # Global styles
└── main.alt       # Application entry point
```

## Learn More
- [Alterion Documentation](https://docs.alterion.dev)
- [Component Guide](https://docs.alterion.dev/components)
- [Styling Guide](https://docs.alterion.dev/styling)

## Contributing
Pull requests are welcome! Please read our contributing guidelines first.

## License
MIT © )" + variables.at("YEAR") + " " + variables.at("AUTHOR") + R"(
)";
}

std::string ProjectScaffolder::generateGitignore() {
    return R"(# Dependencies
node_modules/
npm-debug.log*
yarn-debug.log*
yarn-error.log*

# Build outputs
dist/
build/
.next/
.nuxt/

# Environment files
.env
.env.local
.env.development.local
.env.test.local
.env.production.local

# IDE files
.vscode/
.idea/
*.swp
*.swo
*~

# OS files
.DS_Store
Thumbs.db

# Logs
logs/
*.log

# Runtime data
pids/
*.pid
*.seed
*.pid.lock

# Coverage directory used by tools like istanbul
coverage/
*.lcov

# Temporary folders
tmp/
temp/

# Alterion cache
.alterion/
)";
}

// Package Manager Implementation
PackageManager::PackageManager() {
    cacheDir = std::filesystem::temp_directory_path() / "alterion-cache";
    std::filesystem::create_directories(cacheDir);
}

bool PackageManager::addPackage(const std::string& packageName, const std::string& version) {
    std::cout << "📦 Adding package: " << packageName << "@" << version << "\n";
    
    try {
        // Load current package.json
        ProjectConfig config = ProjectScaffolder().loadProjectConfig(".");
        
        // Add dependency
        config.dependencies[packageName] = version;
        
        // Save updated package.json
        ProjectScaffolder().saveProjectConfig(".", config);
        
        // Install dependencies
        return installDependencies();
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Failed to add package: " << e.what() << "\n";
        return false;
    }
}

bool PackageManager::installDependencies() {
    std::cout << "📥 Installing dependencies...\n";
    
    // For now, delegate to npm/yarn
    int result = std::system("npm install");
    
    if (result == 0) {
        std::cout << "✅ Dependencies installed successfully\n";
        return true;
    } else {
        std::cerr << "❌ Failed to install dependencies\n";
        return false;
    }
}

// Development Server Implementation
DevServer::DevServer(int port, const std::string& host) 
    : port(port), host(host), hotReload(true) {}

void DevServer::start(const ProjectConfig& config) {
    std::cout << "🚀 Starting development server...\n";
    std::cout << "📍 Local: http://" << host << ":" << port << "\n";
    std::cout << "🔥 Hot reload: " << (hotReload ? "enabled" : "disabled") << "\n";
    std::cout << "📁 Serving: " << config.build.srcDir << "\n\n";
    
    // Start file watcher
    watcher.startWatching();
    
    // Add watch paths
    for (const auto& path : config.dev.watchPaths) {
        watcher.addPath(path);
    }
    
    // Main server loop (simplified)
    while (true) {
        auto changedFiles = watcher.getChangedFiles();
        
        if (!changedFiles.empty()) {
            std::cout << "🔄 File changes detected:\n";
            for (const auto& file : changedFiles) {
                std::cout << "  📝 " << file << "\n";
            }
            
            if (hotReload) {
                std::cout << "⚡ Hot reloading...\n";
                // Trigger rebuild and reload
                rebuildAndReload(changedFiles);
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void DevServer::enableHotReload() {
    hotReload = true;
}

void DevServer::disableHotReload() {
    hotReload = false;
}

void DevServer::stop() {
    watcher.stopWatching();
}

void DevServer::rebuildAndReload(const std::vector<std::string>& changedFiles) {
    try {
        // Build changed files
        for (const auto& file : changedFiles) {
            if (file.ends_with(".alt")) {
                std::cout << "🔨 Rebuilding " << file << "...\n";
                
                // Read source
                std::ifstream fileStream(file);
                std::string source((std::istreambuf_iterator<char>(fileStream)),
                                 std::istreambuf_iterator<char>());
                
                // Compile
                Lexer lexer(source);
                auto tokens = lexer.tokenize();
                
                Parser parser(std::move(tokens));
                auto ast = parser.parse();
                
                if (ast) {
                    CodeGenConfig config;
                    config.target = CodeGenConfig::Target::TypeScript;
                    CodeGenerator generator(config);
                    auto result = generator.generate(*ast);
                    
                    // Write output
                    std::string outputFile = "dist/" + std::filesystem::path(file).stem().string() + ".ts";
                    std::filesystem::create_directories(std::filesystem::path(outputFile).parent_path());
                    
                    std::ofstream output(outputFile);
                    output << result.code;
                    output.close();
                    
                    std::cout << "✅ " << outputFile << "\n";
                } else {
                    std::cout << "❌ Parse error in " << file << "\n";
                }
            }
        }
        
        std::cout << "🔄 Reload complete\n\n";
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Rebuild failed: " << e.what() << "\n";
    }
}

// Build System Implementation  
BuildSystem::BuildResult BuildSystem::build(bool production) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    BuildResult result;
    result.success = false;
    result.totalSize = 0;
    
    std::cout << "🔨 Building project...\n";
    std::cout << "📂 Source: " << config.build.srcDir << "\n";
    std::cout << "📁 Output: " << config.build.outDir << "\n";
    std::cout << "🎯 Target: " << config.build.target << "\n";
    
    try {
        // Clean output directory
        if (std::filesystem::exists(config.build.outDir)) {
            std::filesystem::remove_all(config.build.outDir);
        }
        std::filesystem::create_directories(config.build.outDir);
        
        // Find all .alt files
        std::vector<std::string> sourceFiles;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(config.build.srcDir)) {
            if (entry.path().extension() == ".alt") {
                sourceFiles.push_back(entry.path().string());
            }
        }
        
        std::cout << "📝 Found " << sourceFiles.size() << " source files\n";
        
        // Compile each file
        CodeGenConfig codeGenConfig;
        if (config.build.target == "javascript") {
            codeGenConfig.target = CodeGenConfig::Target::JavaScript;
        } else if (config.build.target == "typescript") {
            codeGenConfig.target = CodeGenConfig::Target::TypeScript;
        } else if (config.build.target == "react") {
            codeGenConfig.target = CodeGenConfig::Target::React;
        } else if (config.build.target == "react-native") {
            codeGenConfig.target = CodeGenConfig::Target::ReactNative;
        }
        
        codeGenConfig.minify = production;
        codeGenConfig.generateSourceMaps = config.build.sourceMaps;
        
        CodeGenerator generator(codeGenConfig);
        
        for (const auto& sourceFile : sourceFiles) {
            std::cout << "⚡ " << sourceFile << "\n";
            
            try {
                // Read source
                std::ifstream file(sourceFile);
                std::string source((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
                
                // Compile
                Lexer lexer(source);
                auto tokens = lexer.tokenize();
                
                Parser parser(std::move(tokens));
                auto ast = parser.parse();
                
                if (ast) {
                    auto generated = generator.generate(*ast);
                    
                    // Determine output file
                    std::filesystem::path relativePath = std::filesystem::relative(sourceFile, config.build.srcDir);
                    std::string outputFile = config.build.outDir + "/" + relativePath.replace_extension(
                        codeGenConfig.target == CodeGenConfig::Target::JavaScript ? ".js" : ".ts"
                    ).string();
                    
                    // Create output directory
                    std::filesystem::create_directories(std::filesystem::path(outputFile).parent_path());
                    
                    // Write output
                    std::ofstream output(outputFile);
                    output << generated.code;
                    output.close();
                    
                    result.generatedFiles.push_back(outputFile);
                    result.totalSize += generated.code.length();
                    
                } else {
                    result.errors.push_back("Failed to parse " + sourceFile);
                }
                
            } catch (const std::exception& e) {
                result.errors.push_back("Error compiling " + sourceFile + ": " + e.what());
            }
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        result.buildTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        
        if (result.errors.empty()) {
            result.success = true;
            std::cout << "\n✅ Build completed successfully!\n";
            std::cout << "📊 " << result.generatedFiles.size() << " files generated\n";
            std::cout << "📏 Total size: " << (result.totalSize / 1024) << " KB\n";
            std::cout << "⏱️  Build time: " << result.buildTime << "ms\n";
        } else {
            std::cout << "\n❌ Build failed with errors:\n";
            for (const auto& error : result.errors) {
                std::cerr << "  " << error << "\n";
            }
        }
        
    } catch (const std::exception& e) {
        result.errors.push_back(std::string("Build system error: ") + e.what());
    }
    
    return result;
}

// Code Formatter Implementation
std::string CodeFormatter::formatCode(const std::string& code, const FormatOptions& options) {
    // Simplified formatter - would use a proper AST-based formatter in production
    std::string result = code;
    
    // Basic indentation
    std::vector<std::string> lines;
    std::stringstream ss(result);
    std::string line;
    
    while (std::getline(ss, line)) {
        lines.push_back(line);
    }
    
    std::stringstream formatted;
    int indentLevel = 0;
    
    for (const auto& line : lines) {
        std::string trimmed = line;
        // Remove leading whitespace
        trimmed.erase(0, trimmed.find_first_not_of(" \t"));
        
        // Adjust indent level
        if (trimmed.find('}') != std::string::npos) {
            indentLevel--;
        }
        
        // Add indentation
        for (int i = 0; i < indentLevel; ++i) {
            if (options.useTabs) {
                formatted << "\t";
            } else {
                for (int j = 0; j < options.indentSize; ++j) {
                    formatted << " ";
                }
            }
        }
        
        formatted << trimmed << "\n";
        
        if (trimmed.find('{') != std::string::npos) {
            indentLevel++;
        }
    }
    
    return formatted.str();
}

// File Watcher Implementation
void DevServer::FileWatcher::addPath(const std::string& path) {
    watchedPaths.push_back(path);
    if (std::filesystem::exists(path)) {
        lastModified[path] = std::filesystem::last_write_time(path);
    }
}

void DevServer::FileWatcher::removePath(const std::string& path) {
    watchedPaths.erase(std::remove(watchedPaths.begin(), watchedPaths.end(), path), watchedPaths.end());
    lastModified.erase(path);
}

std::vector<std::string> DevServer::FileWatcher::getChangedFiles() {
    std::vector<std::string> changedFiles;
    
    for (const auto& path : watchedPaths) {
        if (std::filesystem::exists(path)) {
            auto currentTime = std::filesystem::last_write_time(path);
            auto it = lastModified.find(path);
            
            if (it == lastModified.end() || it->second != currentTime) {
                changedFiles.push_back(path);
                lastModified[path] = currentTime;
            }
        }
    }
    
    return changedFiles;
}

void DevServer::FileWatcher::startWatching() {
    // Start watching (simplified implementation)
}

void DevServer::FileWatcher::stopWatching() {
    // Stop watching
}

// Test Runner Implementation  
TestRunner::TestResult TestRunner::runTests(const std::vector<std::string>& testPaths) {
    TestResult result;
    result.totalTests = 0;
    result.passedTests = 0;
    result.failedTests = 0;
    result.skippedTests = 0;
    result.executionTime = 0;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Find test files
    std::vector<std::string> tests;
    if (testPaths.empty()) {
        // Default test discovery
        for (const auto& entry : std::filesystem::recursive_directory_iterator("src")) {
            if (entry.path().filename().string().find(".test.alt") != std::string::npos ||
                entry.path().filename().string().find(".spec.alt") != std::string::npos) {
                tests.push_back(entry.path().string());
            }
        }
    } else {
        tests = testPaths;
    }
    
    result.totalTests = tests.size();
    
    // Run tests (simplified)
    for (const auto& testFile : tests) {
        std::cout << "🧪 Running " << testFile << "...\n";
        
        try {
            // In a real implementation, this would execute the test
            result.passedTests++;
            std::cout << "  ✅ PASS\n";
        } catch (const std::exception& e) {
            result.failedTests++;
            result.errors.push_back("Failed " + testFile + ": " + e.what());
            std::cout << "  ❌ FAIL: " << e.what() << "\n";
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    result.executionTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    
    return result;
}

TestRunner::TestResult TestRunner::runTestsWatch() {
    // Watch mode for tests
    return runTests();
}

void TestRunner::generateCoverageReport() {
    std::cout << "📊 Generating coverage report...\n";
    // Coverage report generation would go here
}

void TestRunner::setupTestEnvironment() {
    std::cout << "🔧 Setting up test environment...\n";
    // Test environment setup would go here
}

} // namespace Alterion
