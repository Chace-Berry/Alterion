#include "cli_tools.h"
#include <iostream>
#include <algorithm>

namespace Alterion {

AlterionCLI::AlterionCLI() {
    scaffolder = std::make_unique<ProjectScaffolder>();
    packageManager = std::make_unique<PackageManager>();
    devServer = std::make_unique<DevServer>();
    formatter = std::make_unique<CodeFormatter>();
}

AlterionCLI::~AlterionCLI() = default;

void AlterionCLI::printUsage() {
    std::cout << R"(
🌟 Alterion CLI v1.0.0
Modern UI development made simple

USAGE:
    alterion <command> [options]

COMMANDS:
    new <name>              Create a new Alterion project
    init                    Initialize Alterion in existing project
    dev                     Start development server with hot reload
    build                   Build project for production
    test                    Run tests
    add <package>           Add a dependency
    remove <package>        Remove a dependency  
    format                  Format code
    lint                    Lint code for errors and style issues
    publish                 Publish package to registry
    upgrade                 Upgrade Alterion and dependencies

PROJECT COMMANDS:
    new <name>              Create new project
      --template <type>     Project template (basic, react, react-native, library)
      --dir <directory>     Target directory (default: current)
      
    init                    Initialize in existing directory
      --template <type>     Initialize with template
      --force               Overwrite existing files

DEVELOPMENT COMMANDS:
    dev                     Start development server
      --port <number>       Server port (default: 3000)
      --host <address>      Server host (default: localhost)
      --no-hot             Disable hot reload
      --no-open            Don't open browser
      
    build                   Build for production
      --target <type>       Build target (js, ts, react, rn)
      --outdir <dir>        Output directory
      --minify             Minify output
      --no-source-maps     Disable source maps

PACKAGE COMMANDS:
    add <package>           Add dependency
      --dev                Add as dev dependency
      --exact              Install exact version
      
    remove <package>        Remove dependency
    
    publish                 Publish to registry
      --tag <name>         Publish with tag
      --access <level>     Set access level

CODE QUALITY:
    format                  Format code
      --check              Check if files are formatted
      --write              Write formatted files
      
    lint                    Lint code
      --fix                Auto-fix issues
      --report <format>    Report format (text, json)

EXAMPLES:
    alterion new my-app                    # Create new project
    alterion new mobile-app --template react-native
    alterion dev --port 8080              # Start dev server on port 8080
    alterion build --target react --minify # Production React build
    alterion add react-router-dom         # Add package
    alterion format --write               # Format all files
    alterion lint --fix                   # Auto-fix lint issues

HELP:
    alterion help <command>               # Get help for specific command
    alterion --version                    # Show version
    alterion --help                       # Show this help

DOCUMENTATION:
    https://docs.alterion.dev

)";
}

void AlterionCLI::printVersion() {
    std::cout << "Alterion CLI v1.0.0\n";
    std::cout << "Node.js runtime: " << "v18+" << "\n";
    std::cout << "Platform: " << 
#ifdef _WIN32
        "Windows"
#elif __APPLE__
        "macOS"  
#else
        "Linux"
#endif
    << "\n";
}

int AlterionCLI::run(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }
    
    std::string command = argv[1];
    std::vector<std::string> args;
    
    for (int i = 2; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }
    
    if (command == "help" || command == "--help" || command == "-h") {
        printUsage();
        return 0;
    }
    
    if (command == "version" || command == "--version" || command == "-v") {
        printVersion();
        return 0;
    }
    
    // Route to command implementations
    if (command == "new") {
        return cmdNew(args);
    } else if (command == "init") {
        return cmdInit(args);
    } else if (command == "dev") {
        return cmdDev(args);
    } else if (command == "build") {
        return cmdBuild(args);
    } else if (command == "test") {
        return cmdTest(args);
    } else if (command == "add") {
        return cmdAdd(args);
    } else if (command == "remove" || command == "rm") {
        return cmdRemove(args);
    } else if (command == "format" || command == "fmt") {
        return cmdFormat(args);
    } else if (command == "lint") {
        return cmdLint(args);
    } else if (command == "publish") {
        return cmdPublish(args);
    } else if (command == "upgrade") {
        return cmdUpgrade(args);
    } else {
        std::cerr << "❌ Unknown command: " << command << "\n";
        std::cerr << "Run 'alterion help' for usage information.\n";
        return 1;
    }
}

int AlterionCLI::cmdNew(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "❌ Project name required\n";
        std::cerr << "Usage: alterion new <project-name> [options]\n";
        return 1;
    }
    
    std::string projectName = args[0];
    ProjectTemplate template = ProjectTemplate::Basic;
    std::string targetDir = ".";
    
    // Parse options
    for (size_t i = 1; i < args.size(); ++i) {
        if (args[i] == "--template" && i + 1 < args.size()) {
            std::string templateName = args[++i];
            if (templateName == "basic") {
                template = ProjectTemplate::Basic;
            } else if (templateName == "react") {
                template = ProjectTemplate::React;
            } else if (templateName == "react-native" || templateName == "rn") {
                template = ProjectTemplate::ReactNative;
            } else if (templateName == "library" || templateName == "lib") {
                template = ProjectTemplate::Library;
            } else {
                std::cerr << "❌ Unknown template: " << templateName << "\n";
                return 1;
            }
        } else if (args[i] == "--dir" && i + 1 < args.size()) {
            targetDir = args[++i];
        }
    }
    
    std::cout << "🎨 Creating new Alterion project...\n";
    std::cout << "📛 Name: " << projectName << "\n";
    std::cout << "📋 Template: " << (template == ProjectTemplate::React ? "React" : 
                                    template == ProjectTemplate::ReactNative ? "React Native" :
                                    template == ProjectTemplate::Library ? "Library" : "Basic") << "\n";
    std::cout << "📁 Location: " << targetDir << "\n\n";
    
    bool success = scaffolder->createProject(projectName, template, targetDir);
    return success ? 0 : 1;
}

int AlterionCLI::cmdDev(const std::vector<std::string>& args) {
    // Load project config
    auto config = scaffolder->loadProjectConfig(".");
    
    int port = config.dev.port;
    std::string host = config.dev.host;
    bool hotReload = config.dev.hotReload;
    bool autoOpen = config.dev.autoOpen;
    
    // Parse options
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--port" && i + 1 < args.size()) {
            port = std::stoi(args[++i]);
        } else if (args[i] == "--host" && i + 1 < args.size()) {
            host = args[++i];
        } else if (args[i] == "--no-hot") {
            hotReload = false;
        } else if (args[i] == "--no-open") {
            autoOpen = false;
        }
    }
    
    std::cout << "🌟 Starting Alterion development server...\n\n";
    
    // Create dev server
    DevServer server(port, host);
    if (!hotReload) {
        server.disableHotReload();
    }
    
    // Open browser
    if (autoOpen) {
        std::string url = "http://" + host + ":" + std::to_string(port);
        std::cout << "🌐 Opening " << url << " in browser...\n";
        
#ifdef _WIN32
        std::system(("start " + url).c_str());
#elif __APPLE__
        std::system(("open " + url).c_str());
#else
        std::system(("xdg-open " + url).c_str());
#endif
    }
    
    // Start server (blocking)
    server.start(config);
    return 0;
}

int AlterionCLI::cmdBuild(const std::vector<std::string>& args) {
    std::cout << "🔨 Building Alterion project...\n\n";
    
    // Load project config
    auto config = scaffolder->loadProjectConfig(".");
    
    bool production = true;
    
    // Parse options
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--target" && i + 1 < args.size()) {
            config.build.target = args[++i];
        } else if (args[i] == "--outdir" && i + 1 < args.size()) {
            config.build.outDir = args[++i];
        } else if (args[i] == "--minify") {
            config.build.minify = true;
        } else if (args[i] == "--no-source-maps") {
            config.build.sourceMaps = false;
        } else if (args[i] == "--dev") {
            production = false;
        }
    }
    
    // Build project
    BuildSystem builder(config, ".");
    auto result = builder.build(production);
    
    if (result.success) {
        std::cout << "\n🎉 Build completed successfully!\n";
        std::cout << "📊 Generated " << result.generatedFiles.size() << " files\n";
        std::cout << "📏 Total size: " << (result.totalSize / 1024) << " KB\n";
        std::cout << "⏱️  Build time: " << result.buildTime << "ms\n";
        return 0;
    } else {
        std::cout << "\n❌ Build failed!\n";
        for (const auto& error : result.errors) {
            std::cerr << "  " << error << "\n";
        }
        return 1;
    }
}

int AlterionCLI::cmdAdd(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "❌ Package name required\n";
        std::cerr << "Usage: alterion add <package> [version]\n";
        return 1;
    }
    
    std::string packageName = args[0];
    std::string version = "latest";
    
    if (args.size() > 1) {
        version = args[1];
    }
    
    bool success = packageManager->addPackage(packageName, version);
    return success ? 0 : 1;
}

int AlterionCLI::cmdRemove(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "❌ Package name required\n";
        std::cerr << "Usage: alterion remove <package>\n";
        return 1;
    }
    
    std::string packageName = args[0];
    bool success = packageManager->removePackage(packageName);
    return success ? 0 : 1;
}

int AlterionCLI::cmdFormat(const std::vector<std::string>& args) {
    std::cout << "✨ Formatting Alterion code...\n";
    
    bool checkOnly = false;
    bool writeFiles = false;
    
    // Parse options
    for (const auto& arg : args) {
        if (arg == "--check") {
            checkOnly = true;
        } else if (arg == "--write") {
            writeFiles = true;
        }
    }
    
    if (!checkOnly && !writeFiles) {
        writeFiles = true; // Default to writing
    }
    
    CodeFormatter::FormatOptions options;
    formatter->formatProject("src", options);
    
    std::cout << "✅ Code formatting complete!\n";
    return 0;
}

int AlterionCLI::cmdLint(const std::vector<std::string>& args) {
    std::cout << "🔍 Linting Alterion code...\n";
    
    auto result = formatter->lintProject("src");
    
    if (!result.errors.empty()) {
        std::cout << "\n❌ Errors found:\n";
        for (const auto& error : result.errors) {
            std::cerr << "  " << error << "\n";
        }
    }
    
    if (!result.warnings.empty()) {
        std::cout << "\n⚠️  Warnings:\n";
        for (const auto& warning : result.warnings) {
            std::cout << "  " << warning << "\n";
        }
    }
    
    if (result.errors.empty() && result.warnings.empty()) {
        std::cout << "✅ No issues found!\n";
    }
    
    return result.errors.empty() ? 0 : 1;
}

int AlterionCLI::cmdTest(const std::vector<std::string>& args) {
    std::cout << "🧪 Running tests...\n";
    
    TestRunner runner;
    auto result = runner.runTests();
    
    std::cout << "\n📊 Test Results:\n";
    std::cout << "  Total: " << result.totalTests << "\n";
    std::cout << "  Passed: " << result.passedTests << "\n";
    std::cout << "  Failed: " << result.failedTests << "\n";
    std::cout << "  Skipped: " << result.skippedTests << "\n";
    std::cout << "  Time: " << result.executionTime << "ms\n";
    
    if (result.failedTests == 0) {
        std::cout << "\n✅ All tests passed!\n";
        return 0;
    } else {
        std::cout << "\n❌ " << result.failedTests << " test(s) failed\n";
        return 1;
    }
}

int AlterionCLI::cmdPublish(const std::vector<std::string>& args) {
    std::cout << "📦 Publishing package...\n";
    
    bool success = packageManager->publishPackage(".");
    
    if (success) {
        std::cout << "✅ Package published successfully!\n";
        return 0;
    } else {
        std::cout << "❌ Failed to publish package\n";
        return 1;
    }
}

int AlterionCLI::cmdInit(const std::vector<std::string>& args) {
    std::cout << "🎯 Initializing Alterion project...\n";
    
    // Create basic project structure in current directory
    bool success = scaffolder->createProject(".", ProjectTemplate::Basic, ".");
    
    if (success) {
        std::cout << "✅ Project initialized!\n";
        std::cout << "Run 'alterion dev' to start development\n";
        return 0;
    } else {
        return 1;
    }
}

int AlterionCLI::cmdUpgrade(const std::vector<std::string>& args) {
    std::cout << "⬆️  Upgrading Alterion...\n";
    
    // Update Alterion CLI and project dependencies
    int result = std::system("npm update alterion");
    
    if (result == 0) {
        std::cout << "✅ Alterion upgraded successfully!\n";
        return 0;
    } else {
        std::cout << "❌ Failed to upgrade Alterion\n";
        return 1;
    }
}

} // namespace Alterion

// Main entry point
int main(int argc, char* argv[]) {
    Alterion::AlterionCLI cli;
    return cli.run(argc, argv);
}
