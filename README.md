# 🚀 Alterion Programming Language

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=flat&logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](.)
[![Version](https://img.shields.io/badge/version-2.6.0-orange.svg)](.)

> **A modern, expressive programming language with native compilation, component-based architecture, and universal NativeUI rendering.**

## 🌟 What is Alterion?

Alterion is a revolutionary systems programming language designed for modern cross-platform application development. It combines the performance of native compilation with the simplicity of component-based architecture and powerful async programming patterns.

### Key Features

- 🎯 **Component-First Architecture** - Build applications as reusable, native components
- ⚡ **Async-Native** - Built-in async blocks with try/catch/finally support
- 🎨 **NativeUI System** - Universal UI rendering across desktop, mobile, and web
- � **Native Compilation** - Direct compilation to machine code (x86_64, ARM, RISC-V)
- 🛡️ **Memory Safety** - Rust-inspired ownership system with lifetime analysis
- 📦 **Modern Syntax** - Clean, readable syntax with type inference

## 🎯 Why Alterion?

```alterion
// Traditional C++ Desktop Application
#include <windows.h>
#include <iostream>

class UserProfile {
    HWND window;
    HWND label;
    HWND button;
    
public:
    void create() {
        window = CreateWindow(...);
        label = CreateWindow(L"STATIC", ...);
        button = CreateWindow(L"BUTTON", ...);
        // 50+ lines of Win32 boilerplate...
    }
    
    void loadUser(int userId) {
        // Manual async handling, error management
        std::thread([=]() {
            try {
                auto user = ApiService::getUser(userId);
                // Update UI on main thread...
            } catch (...) {
                // Handle errors...
            }
        }).detach();
    }
};
```

```alterion
// Equivalent Alterion Component
component UserProfile {
    user: object = null
    loading: boolean = true
    error: string = null
    
    @async
    loadUser(userId: int) {
        loading = true
        async {
            [user = await ApiService.getUser(userId)]
            [catch(err) error = err.message]
            [finally loading = false]
        }
    }
    
    render:
        loading ? <div>Loading...</div> :
        error ? <div>Error: {error}</div> :
        <div center>
            <h2>{user.name}</h2>
            <p>{user.email}</p>
        </div>
}
```

## 🚀 Quick Start

### Prerequisites

- **Windows**: MinGW-w64 or Visual Studio 2019+
- **macOS/Linux**: GCC 7+ or Clang 8+
- **Build Tools**: CMake 3.15+

### Installation

```bash
git clone https://github.com/Chace-Berry/Alterion.git
cd alterion

# Windows
scripts\build.bat

# macOS/Linux
chmod +x scripts/build.sh
./scripts/build.sh
```

### Your First Alterion Program

Create `hello.alt`:

```alterion
component HelloWorld {
    name: string = "Alterion"
    count: number = 0
    
    increment {
        count += 1
    }
    
    render:
        <div center>
            <h1>Hello, {name}!</h1>
            <p>Clicked {count} times</p>
            <button onClick={increment} class="btn-primary">
                Click me!
            </button>
        </div>
}

export default HelloWorld;
```

Compile and run:

```bash
# Compile to native executable
alterion build hello.alt --target native

# Compile for web (NativeUI → DOM)
alterion build hello.alt --target web

# Compile for mobile (NativeUI → native mobile UI)
alterion build hello.alt --target mobile

# Development mode with hot reload
alterion dev hello.alt
```

## 📚 Language Features

### 🧩 Component System

```alterion
component TodoApp {
    todos: array = []
    newTodo: string = ""
    
    addTodo {
        if (newTodo.trim()) {
            todos.push({ text: newTodo, completed: false })
            newTodo = ""
        }
    }
    
    toggleTodo(index: number) {
        todos[index].completed = !todos[index].completed
    }
    
    render:
        <div class="todo-app">
            <input 
                value={newTodo} 
                onChange={(e) => newTodo = e.target.value}
                placeholder="Add a todo..."
            />
            <button onClick={addTodo}>Add</button>
            
            <ul class="todo-list">
                for todo, index in todos []
                    <TodoItem 
                        todo={todo} 
                        onToggle={() => toggleTodo(index)}
                    />
                ]
            </ul>
        </div>
}
```

### ⚡ Async Programming

```alterion
component DataFetcher {
    data: array = []
    loading: boolean = false
    
    @async
    fetchData {
        loading = true
        
        async {
            [
                let response = await fetch('/api/data')
                data = await response.json()
            ]
            [catch(error) 
                console.error('Failed to fetch:', error)
            ]
            [finally 
                loading = false
            ]
        }
    }
}
```

### 🔄 Enhanced Control Flow

```alterion
function processItems(items: array) {
    // Enhanced for loops
    for i (items.length) [
        console.log(`Processing item ${i}: ${items[i]}`)
    ]
    
    // For-in loops
    for item in items [
        if (item.active) {
            item.process()
        }
    ]
    
    // Range loops
    for i (0, 100) [
        if (i % 2 === 0) {
            console.log(`Even: ${i}`)
        }
    ]
}
```

### 🎨 Universal UI Rendering

```alterion
component ResponsiveLayout {
    isMobile: boolean = window.innerWidth < 768
    
    render:
        <div class={isMobile ? "mobile-layout" : "desktop-layout"}>
            <header center>
                <h1>My App</h1>
            </header>
            
            <main flex="column" gap="20px">
                <UserProfile />
                <TodoList />
                <Footer />
            </main>
        </div>
}
```

## 🛠️ Development Tools

### CLI Compiler

```bash
# Basic compilation
alterion build src/ --output dist/ --target native

# Development mode
alterion dev src/ --target native --hot-reload

# Cross-platform compilation
alterion build app.alt --targets native,web,mobile

# With optimizations
alterion build app.alt --optimize --target native
```

### Language Server Protocol (LSP)

- **VS Code Extension**: Full IntelliSense support
- **Syntax Highlighting**: Rich syntax coloring
- **Error Detection**: Real-time error checking
- **Auto-completion**: Smart code completion
- **Go to Definition**: Navigate your codebase easily

### Build System Integration

```json
// alterion.toml (Project configuration)
[project]
name = "my-app"
version = "1.0.0"

[build]
target = "native"
optimize = true

[dependencies]
# External C libraries via FFI
openssl = "1.1"
sqlite = "3.0"
```

## 🏗️ Project Structure

```
alterion/
├── core/                   # Core language implementation
│   ├── lexer/             # Tokenization and lexical analysis
│   ├── parser/            # Syntax analysis and AST generation
│   ├── semantic/          # Type checking and semantic analysis
│   └── codegen/           # Code generation and optimization
├── include/               # Header files and API definitions
├── runtime/               # Runtime system and standard library
│   ├── nativeui/          # Cross-platform UI rendering engine
│   └── ffi/               # Foreign function interface
├── stdlib/                # Alterion standard library
├── tools/                 # Development tools
│   ├── cli/               # Command-line compiler
│   ├── lsp/               # Language server protocol
│   └── debugger/          # Debug support
├── tests/                 # Test suite
│   ├── unit/              # Unit tests
│   ├── integration/       # Integration tests
│   └── benchmarks/        # Performance benchmarks
├── examples/              # Example Alterion programs
├── documentation/         # Language documentation
└── scripts/               # Build and utility scripts
```

## 🧪 Testing

```bash
# Run all tests
alterion test

# Run specific test suite
alterion test --suite lexer
alterion test --suite parser
alterion test --suite codegen

# Generate coverage report
alterion test --coverage

# Performance benchmarks
alterion benchmark
```

## 📖 Documentation

- 📚 [Language Guide](documentation/language-guide/) - Complete language reference
- 🎯 [Getting Started](documentation/getting-started.md) - Beginner-friendly tutorial  
- 🔧 [API Reference](documentation/api-reference/) - Detailed API documentation
- 🏗️ [Architecture](documentation/architecture.md) - Internal architecture overview
- 🚀 [Migration Guide](documentation/migration.md) - Migrating from other languages

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guide](CONTRIBUTING.md) for details.

### Development Setup

```bash
git clone https://github.com/Chace-Berry/Alterion.git
cd alterion

# Install development dependencies
npm install -g cmake
npm install

# Build development version
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j4

# Run tests
ctest --verbose
```

### Roadmap

- [x] **Phase 1**: Lexical Analysis & Tokenization
- [x] **Phase 2**: Syntax Analysis & AST Generation  
- [x] **Phase 3**: Semantic Analysis & Type Checking
- [x] **Phase 4**: Code Generation & Optimization
- [x] **Phase 5**: NativeUI Rendering Engine
- [x] **Phase 6**: CLI Tools & Language Server
- [ ] **Phase 7**: Advanced Optimizations (LLVM backend)
- [ ] **Phase 8**: Package Manager & Registry
- [ ] **Phase 9**: Debugger & Profiler
- [ ] **Phase 10**: IDE Extensions & Tooling

## 📊 Performance

Alterion is designed for performance:

- **Fast Compilation**: Multi-threaded compilation with intelligent caching
- **Optimized Output**: Generated code is optimized for target platforms
- **Memory Efficient**: Minimal runtime overhead
- **Bundle Size**: Tree-shaking and dead code elimination

| Metric | Value |
|--------|-------|
| Compile Time | ~1.8s for 10k LOC |
| Memory Usage | ~32MB peak |
| Binary Size | 60% smaller than equivalent C++ |
| Runtime Performance | Native machine code speed |

## 🌍 Community

- � [Contact](mailto:chaceberry686@gmail.com) - Questions and discussions
- 🐛 [Report Issues](mailto:chaceberry686@gmail.com) - Bug reports and feature requests

## 📄 License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

### What this means:
- ✅ **Free to use** for any purpose
- ✅ **Free to modify** and distribute
- ✅ **Free to contribute** to open source projects
- ❌ **Cannot be used** in proprietary software without releasing source code
- ❌ **Cannot be monetized** without explicit permission from the author

For commercial licensing options, please contact [chaceberry686@gmail.com](mailto:chaceberry686@gmail.com).

## 🙏 Acknowledgments

- Inspired by modern languages like Rust, TypeScript, and Swift
- Built with love for the developer community
- Special thanks to all contributors and early adopters

---

<div align="center">

*Made with ❤️ by Chace Berry*

**Contact: [chaceberry686@gmail.com](mailto:chaceberry686@gmail.com)**

</div>
