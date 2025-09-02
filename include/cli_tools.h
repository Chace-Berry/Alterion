#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <memory>

namespace Alterion {

// Phase 2E: CLI Tools and Development Environment
// Complete toolchain for Alterion development

struct ProjectConfig {
    std::string name;
    std::string version;
    std::string description;
    std::string main;
    std::vector<std::string> scripts;
    std::unordered_map<std::string, std::string> dependencies;
    std::unordered_map<std::string, std::string> devDependencies;
    
    // Alterion-specific config
    struct BuildConfig {
        std::string target = "typescript";        // js, ts, react, react-native
        std::string outDir = "dist";
        std::string srcDir = "src";
        bool sourceMaps = true;
        bool minify = false;
        bool optimize = true;
        std::vector<std::string> include;
        std::vector<std::string> exclude;
    } build;
    
    struct DevConfig {
        int port = 3000;
        bool hotReload = true;
        bool autoOpen = true;
        std::string host = "localhost";
        std::vector<std::string> watchPaths;
    } dev;
};

// Project template system
enum class ProjectTemplate {
    Basic,              // Simple Alterion project
    React,              // React web application
    ReactNative,        // React Native mobile app
    Library,            // Reusable component library
    Fullstack,          // Full-stack application with API
    Desktop,            // Desktop application (Electron)
    Component           // Single component project
};

struct TemplateInfo {
    std::string name;
    std::string description;
    std::vector<std::string> files;
    std::unordered_map<std::string, std::string> dependencies;
    std::string readme;
};

// Package management
struct PackageInfo {
    std::string name;
    std::string version;
    std::string description;
    std::string author;
    std::string license;
    std::vector<std::string> keywords;
    std::unordered_map<std::string, std::string> dependencies;
    std::string repository;
    std::string homepage;
};

class ProjectScaffolder {
private:
    std::unordered_map<ProjectTemplate, TemplateInfo> templates;
    
    void initializeTemplates() {
        // Basic project template
        templates[ProjectTemplate::Basic] = {
            "basic",
            "Basic Alterion project with essential structure",
            {
                "src/main.alt",
                "src/components/App.alt",
                "alterion.json",
                "package.json",
                "README.md",
                ".gitignore"
            },
            {
                {"react", "^18.0.0"},
                {"@types/react", "^18.0.0"}
            },
            "# Basic Alterion Project\n\nA simple Alterion application."
        };
        
        // React web app template
        templates[ProjectTemplate::React] = {
            "react",
            "React web application with routing and state management",
            {
                "src/main.alt",
                "src/components/App.alt",
                "src/components/Header.alt",
                "src/components/Footer.alt",
                "src/pages/Home.alt",
                "src/pages/About.alt",
                "src/styles/global.css",
                "public/index.html",
                "alterion.json",
                "package.json",
                "README.md",
                ".gitignore"
            },
            {
                {"react", "^18.0.0"},
                {"react-dom", "^18.0.0"},
                {"react-router-dom", "^6.0.0"},
                {"@types/react", "^18.0.0"},
                {"@types/react-dom", "^18.0.0"}
            },
            "# React Alterion App\n\nA modern React application built with Alterion."
        };
        
        // React Native template
        templates[ProjectTemplate::ReactNative] = {
            "react-native",
            "Cross-platform mobile application",
            {
                "src/App.alt",
                "src/components/Header.alt",
                "src/screens/Home.alt",
                "src/screens/Profile.alt",
                "src/navigation/AppNavigator.alt",
                "alterion.json",
                "package.json",
                "README.md",
                ".gitignore",
                "metro.config.js",
                "babel.config.js"
            },
            {
                {"react-native", "^0.72.0"},
                {"@react-navigation/native", "^6.0.0"},
                {"@react-navigation/stack", "^6.0.0"},
                {"react-native-screens", "^3.0.0"},
                {"react-native-safe-area-context", "^4.0.0"}
            },
            "# React Native Alterion App\n\nCross-platform mobile app built with Alterion."
        };
    }
    
public:
    ProjectScaffolder() {
        initializeTemplates();
    }
    
    bool createProject(const std::string& projectName, ProjectTemplate template, const std::string& targetDir);
    void generateFile(const std::string& templatePath, const std::string& outputPath, const std::unordered_map<std::string, std::string>& variables);
    ProjectConfig loadProjectConfig(const std::string& projectDir);
    void saveProjectConfig(const std::string& projectDir, const ProjectConfig& config);
    
private:
    std::string generateAlterionFile(const std::string& templatePath, const std::unordered_map<std::string, std::string>& variables);
    std::string generatePackageJson(const std::unordered_map<std::string, std::string>& variables);
    std::string generateAlterionConfig(const std::unordered_map<std::string, std::string>& variables);
    std::string generateReadme(const std::unordered_map<std::string, std::string>& variables);
    std::string generateGitignore();
    std::string generateGenericFile(const std::string& templatePath, const std::unordered_map<std::string, std::string>& variables);
};

// Package manager
class PackageManager {
private:
    std::string registry = "https://registry.alterion.dev";
    std::string cacheDir;
    
public:
    PackageManager();
    
    bool addPackage(const std::string& packageName, const std::string& version = "latest");
    bool removePackage(const std::string& packageName);
    bool updatePackage(const std::string& packageName);
    bool installDependencies();
    
    std::vector<PackageInfo> searchPackages(const std::string& query);
    PackageInfo getPackageInfo(const std::string& packageName);
    
    bool publishPackage(const std::string& packageDir);
    bool unpublishPackage(const std::string& packageName, const std::string& version);
};

// Development server
class DevServer {
private:
    int port;
    std::string host;
    bool hotReload;
    std::vector<std::string> watchPaths;
    std::unordered_map<std::string, std::string> middleware;
    
    class FileWatcher {
    private:
        std::vector<std::string> watchedPaths;
        std::unordered_map<std::string, std::filesystem::file_time_type> lastModified;
        
    public:
        void addPath(const std::string& path);
        void removePath(const std::string& path);
        std::vector<std::string> getChangedFiles();
        void startWatching();
        void stopWatching();
    };
    
    FileWatcher watcher;
    
public:
    DevServer(int port = 3000, const std::string& host = "localhost");
    
    void start(const ProjectConfig& config);
    void stop();
    
    void enableHotReload();
    void disableHotReload();
    
    void addMiddleware(const std::string& path, const std::string& handler);
    void removeMiddleware(const std::string& path);
    
    void serveStatic(const std::string& path, const std::string& directory);
    void proxy(const std::string& path, const std::string& target);
};

// Build system
class BuildSystem {
private:
    ProjectConfig config;
    std::string workingDir;
    
public:
    BuildSystem(const ProjectConfig& cfg, const std::string& dir) 
        : config(cfg), workingDir(dir) {}
    
    struct BuildResult {
        bool success;
        std::vector<std::string> generatedFiles;
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
        long long buildTime; // milliseconds
        size_t totalSize;    // bytes
    };
    
    BuildResult build(bool production = false);
    BuildResult watch();
    
    void clean();
    void optimize();
    
    // Asset processing
    void processStyles();
    void processAssets();
    void generateManifest();
    
    // Bundle analysis
    void analyzeBundles();
    void generateReport();
};

// Testing framework integration
class TestRunner {
private:
    std::string testDir;
    std::vector<std::string> testFiles;
    
public:
    struct TestResult {
        int totalTests;
        int passedTests;
        int failedTests;
        int skippedTests;
        std::vector<std::string> errors;
        long long executionTime;
    };
    
    TestResult runTests(const std::vector<std::string>& testPaths = {});
    TestResult runTestsWatch();
    
    void generateCoverageReport();
    void setupTestEnvironment();
};

// Code formatting and linting
class CodeFormatter {
public:
    struct FormatOptions {
        int indentSize = 2;
        bool useTabs = false;
        int maxLineLength = 100;
        bool semicolons = true;
        bool trailingCommas = true;
        std::string quoteStyle = "double"; // "double", "single"
    };
    
    std::string formatCode(const std::string& code, const FormatOptions& options = {});
    std::string formatFile(const std::string& filePath, const FormatOptions& options = {});
    
    void formatProject(const std::string& projectDir, const FormatOptions& options = {});
    
    struct LintResult {
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
        std::vector<std::string> suggestions;
    };
    
    LintResult lintCode(const std::string& code);
    LintResult lintFile(const std::string& filePath);
    LintResult lintProject(const std::string& projectDir);
};

// Main CLI application
class AlterionCLI {
private:
    std::unique_ptr<ProjectScaffolder> scaffolder;
    std::unique_ptr<PackageManager> packageManager;
    std::unique_ptr<DevServer> devServer;
    std::unique_ptr<CodeFormatter> formatter;
    
    void printUsage();
    void printVersion();
    
public:
    AlterionCLI();
    ~AlterionCLI();
    
    int run(int argc, char* argv[]);
    
    // Command implementations
    int cmdNew(const std::vector<std::string>& args);
    int cmdDev(const std::vector<std::string>& args);
    int cmdBuild(const std::vector<std::string>& args);
    int cmdTest(const std::vector<std::string>& args);
    int cmdAdd(const std::vector<std::string>& args);
    int cmdRemove(const std::vector<std::string>& args);
    int cmdFormat(const std::vector<std::string>& args);
    int cmdLint(const std::vector<std::string>& args);
    int cmdPublish(const std::vector<std::string>& args);
    int cmdInit(const std::vector<std::string>& args);
    int cmdUpgrade(const std::vector<std::string>& args);
};

} // namespace Alterion
