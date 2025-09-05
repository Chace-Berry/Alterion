# 🎯 Getting Started with Alterion
# ⚠️ **Note:** The features and syntax described below are future plans for Alterion and may not be fully implemented yet.

> Beginner-friendly tutorial for the Alterion Programming Language

## Welcome to Alterion!
Alterion is a revolutionary programming language that combines:
> Beginner-friendly tutorial for the Alterion Programming Language

## Welcome to Alterion!

Alterion is a revolutionary programming language that combines:
- **Component-based architecture** like React
- **Memory safety** like Rust  
- **Native performance** like C++
- **Universal UI** that works everywhere

Let's build your first Alterion application!

## Installation

### Prerequisites
- **Windows**: MinGW-w64 or Visual Studio 2019+
- **macOS/Linux**: GCC 7+ or Clang 8+
- **Build Tools**: CMake 3.15+

### Install Alterion
```bash
git clone https://github.com/Chace-Berry/Alterion.git
cd alterion

# Windows
scripts\build.bat

# macOS/Linux
chmod +x scripts/build.sh
./scripts/build.sh
```

### Verify Installation
```bash
alterion --version
# Should output: Alterion 2.6.0
```

## Your First Program

### Hello World
Create a file called `hello.alt`:

```alterion
component HelloWorld {
  message = "Hello, Alterion!"
  render:
    <div center>
      <h1>{message}</h1>
    </div>
}

export default HelloWorld
```

Compile and run:
```bash
alterion build hello.alt --target native
./hello
```

**What's happening here?**
- We define a `component` called `HelloWorld`
- It has a `message` property (type inferred)
- The `render` block defines the UI using JSX-like syntax
- `<div center>` creates a centered div (no CSS needed!)
- `{message}` interpolates the variable into the UI

## Language Fundamentals

### Variables and Types

```alterion
// Dynamic typing (type inferred)
name = "Alice"
age = 25
isStudent = true

// Static typing (optional)
name: String = "Alice"
age: Int = 25
isStudent: Bool = true

// Arrays
numbers = [1, 2, 3, 4, 5]
names = ["Alice", "Bob", "Charlie"]
```

### Functions

```alterion
// Simple function
greet(name) {
  return "Hello, " + name + "!"
}

// With type hints (optional)
add(a: Int, b: Int): Int {
  return a + b
}

// Usage
result = add(5, 3)  // result = 8
greeting = greet("World")  // greeting = "Hello, World!"
```

### Control Flow

#### If Statements
```alterion
age = 18

if (age >= 18)[
  print("You can vote!")
][
  print("Too young to vote")
]
```

#### Loops
```alterion
// Count from 0 to 4
for i (5)[
  print("Count: " + i)
]

// Count from 1 to 10
for i (1, 11)[
  print("Number: " + i)
]

// Iterate over array
fruits = ["apple", "banana", "orange"]
for fruit in fruits[
  print("I like " + fruit)
]
```

## Building Interactive Components

### Counter Component
Let's build a classic counter:

```alterion
component Counter {
  count = 0
  increment {
    count = count + 1
  }
  decrement {
    count = count - 1
  }
  render:
    <div center>
      <h2>Count: {count}</h2>
      <div>
        <button onClick={increment}>+</button>
        <button onClick={decrement}>-</button>
      </div>
    </div>
}
```

### Todo List Component
A more complex example:

```alterion
component TodoList {
  todos = []
  newTodo = ""
  addTodo {
    if (newTodo.trim() != "") {
      todos.push(newTodo)
      newTodo = ""
    }
  }
  removeTodo(index) {
    todos.removeAt(index)
  }
  updateInput(value) {
    newTodo = value
  }
  render:
    <div>
      <h2>My Todo List</h2>
      <div>
        <input 
          value={newTodo}
          onChange={updateInput}
          placeholder="Add a new todo..."
        />
        <button onClick={addTodo}>Add</button>
      </div>
      <ul>
        for todo, index in todos[
          <li>
            {todo}
            <button onClick={() => removeTodo(index)}>
              Delete
            </button>
          </li>
        ]
      </ul>
    </div>
}
```

## Async Programming Made Easy

### Async Functions
```alterion
@async
fetchUserData(userId) {
  async {
    [response = await httpGet("/api/users/" + userId)]
    [user = await response.json()]
    [return user]
    [catch(err) return null]
  }
}
```

### Async Blocks with Error Handling
```alterion
component UserProfile {
  user = null
  loading = false
  error = null
  @async
  loadUser(id) {
    loading = true
    error = null
    async {
      [response = await fetchUserData(id)]
      [user = response.user]
      [catch(err) error = "Failed to load user: " + err.message]
      [finally loading = false]
    }
  }
  render:
    loading ? 
      <div center>Loading...</div> :
    error ? 
      <div center>Error: {error}</div> :
    user ?
      <div center>
        <h1>{user.name}</h1>
        <p>{user.email}</p>
      </div> :
      <div center>No user loaded</div>
}
```

## Memory Safety Without Garbage Collection

Alterion uses a Rust-inspired ownership system:

### Ownership
```alterion
// Each value has exactly one owner
data = createData()  // data owns the value
moved = data        // ownership moves to 'moved'
// 'data' is no longer valid

processData(moved)  // ownership moves to function
// 'moved' is no longer valid
```

### Borrowing
```alterion
// Borrow without taking ownership
data = createData()
processData(&data)  // borrow immutably
data.cleanup()      // 'data' still valid

// Mutable borrowing
modifyData(@mut data)  // borrow mutably
```

### Automatic Cleanup
```alterion
// Resources automatically cleaned up
{
  file = openFile("data.txt")
  content = file.readAll()
  // file automatically closed here
}
```

## Cross-Platform UI

### Universal Components
Alterion UI tags are **universal UI elements**—they automatically render natively on mobile, macOS, Windows, Linux, and web. You never need to recode for specific platforms or devices; your components work everywhere by default.

The same component works on all platforms:

```alterion
component ResponsiveApp {
  isMobile = screenWidth < 768
  render:
    <div class={isMobile ? "mobile" : "desktop"}>
      <header center>
        <h1>My App</h1>
      </header>
      <main flex="column" gap="20px">
        <UserProfile />
        <TodoList />
      </main>
      <footer center>
        <p>© 2025 My Company</p>
      </footer>
    </div>
}
```

### Building for Different Targets
```bash
# Native desktop app
alterion build app.alt --target native

# Web application (compiles to WebAssembly)
alterion build app.alt --target web

# Mobile app (native mobile UI)
alterion build app.alt --target mobile

# Development with hot reload
alterion dev app.alt --hot-reload
```

## Development Tools

### CLI Commands
```bash
# Create new project
alterion new my-project

# Build project
alterion build src/ --output dist/

# Run with hot reload
alterion dev src/ --port 3000

# Format code
alterion fmt src/

# Run tests
alterion test

# Type checking
alterion check src/
```

### IDE Support
- **VS Code Extension**: Full IntelliSense, syntax highlighting, debugging
- **Language Server**: Works with any LSP-compatible editor
- **Real-time Error Checking**: Catch errors as you type

## Next Steps

Congratulations! You've learned the basics of Alterion. Here's what to explore next:

1. **[Language Guide](language-guide/)** - Complete language reference
2. **[API Reference](api-reference/)** - Built-in functions and libraries  
3. **[Architecture Guide](architecture.md)** - How Alterion works internally
4. **[Examples](../examples/)** - More complex applications

### Example Projects to Try
- **Calculator App** - Practice with state and events
- **Weather App** - Learn async programming and API calls
- **Chat Application** - Explore real-time features
- **Game** - Build interactive experiences

### Community
- 📧 [Get Help](mailto:chaceberry686@gmail.com) - Questions and support
- 🐛 [Report Bugs](mailto:chaceberry686@gmail.com) - Help improve Alterion
- � [Feature Requests](mailto:chaceberry686@gmail.com) - Suggest new features

---

*Ready to build amazing applications with Alterion? Start coding and see how easy modern programming can be!*
- **Universal UI** that works everywhere

Let's build your first Alterion application!

## Installation

### Prerequisites
- **Windows**: MinGW-w64 or Visual Studio 2019+
- **macOS/Linux**: GCC 7+ or Clang 8+
- **Build Tools**: CMake 3.15+

### Install Alterion
```bash
git clone https://github.com/Chace-Berry/Alterion.git
cd alterion

# Windows
scripts\build.bat

# macOS/Linux
chmod +x scripts/build.sh
./scripts/build.sh
```

### Verify Installation
```bash
alterion --version
```

## Your First Program

### Hello World
Create a file called `hello.alt`:

```alterion
component HelloWorld {
  message: String = "Hello, Alterion!"
  
  render:
    <div center>
      <h1>{message}</h1>
    </div>
}

export default HelloWorld
```

Compile and run:
```bash
alterion build hello.alt --target native
./hello
```

**What's happening here?**
- We define a `component` called `HelloWorld`
- It has a `message` property with type `String`
- The `render` block defines the UI using JSX-like syntax
- `<div center>` creates a centered div (no CSS needed!)
- `{message}` interpolates the variable into the UI

## Language Fundamentals

### Variables and Types

```alterion
// Dynamic typing (type inferred)
name = "Alice"
age = 25
isStudent = true

// Static typing (explicit types)
name: String = "Alice"
age: Int = 25
isStudent: Bool = true

// Arrays
numbers = [1, 2, 3, 4, 5]
names: [String] = ["Alice", "Bob", "Charlie"]
```

### Functions

```alterion
// Simple function
greet(name) {
  return "Hello, " + name + "!"
}

// With type hints
add(a: Int, b: Int): Int {
  return a + b
}

// Usage
result = add(5, 3)  // result = 8
greeting = greet("World")  // greeting = "Hello, World!"
```

### Control Flow

#### If Statements
```alterion
age = 18

if (age >= 18)[
  print("You can vote!")
][
  print("Too young to vote")
]
```

#### Loops
```alterion
// Count from 0 to 4
for i (5)[
  print("Count: " + i)
]

// Count from 1 to 10
for i (1, 11)[
  print("Number: " + i)
]

// Iterate over array
fruits = ["apple", "banana", "orange"]
for fruit in fruits[
  print("I like " + fruit)
]
```

## Building Interactive Components

### Counter Component
Let's build a classic counter:

```alterion
component Counter {
  count: Int = 0
  
  increment {
    count = count + 1
  }
  
  decrement {
    count = count - 1
  }
  
  render:
    <div center>
      <h2>Count: {count}</h2>
      <div>
        <button onClick={increment}>+</button>
        <button onClick={decrement}>-</button>
      </div>
    </div>
}
```

**Key concepts:**
- Component state with `count: Int = 0`
- Event handlers like `increment` and `decrement`
- UI event binding with `onClick={increment}`
- Automatic re-rendering when state changes

### Todo List Component
A more complex example:

```alterion
component TodoList {
  todos: [String] = []
  newTodo: String = ""
  
  addTodo {
    if (newTodo.trim() != "") {
      todos.push(newTodo)
      newTodo = ""
    }
  }
  
  removeTodo(index: Int) {
    todos.removeAt(index)
  }
  
  updateInput(value: String) {
    newTodo = value
  }
  
  render:
    <div>
      <h2>My Todo List</h2>
      
      <div>
        <input 
          value={newTodo}
          onChange={updateInput}
          placeholder="Add a new todo..."
        />
        <button onClick={addTodo}>Add</button>
      </div>
      
      <ul>
        for todo, index in todos[
          <li>
            {todo}
            <button onClick={() => removeTodo(index)}>
              Delete
            </button>
          </li>
        ]
      </ul>
    </div>
}
```

## Async Programming Made Easy

### Async Functions
```alterion
@async
fetchUserData(userId: String) {
  response = await httpGet("/api/users/" + userId)
  return await response.json()
}
```

### Async Blocks with Error Handling
```alterion
component UserProfile {
  user: Object = null
  loading: Bool = false
  error: String = null
  
  @async
  loadUser(id: String) {
    loading = true
    error = null
    
    async{[
      // Try block
      response = await fetchUserData(id)
      user = response.user
    ][
      // Catch block
      catch(err) {
        error = "Failed to load user: " + err.message
      }
    ][
      // Finally block
      finally {
        loading = false
      }
    ]}
  }
  
  render:
    loading ? 
      <div center>Loading...</div> :
    error ? 
      <div center>Error: {error}</div> :
    user ?
      <div center>
        <h1>{user.name}</h1>
        <p>{user.email}</p>
      </div> :
      <div center>No user loaded</div>
}
```

## Memory Safety Without Garbage Collection

Alterion uses a Rust-inspired ownership system:

### Ownership
```alterion
// Each value has exactly one owner
data = createData()  // data owns the value
moved = data        // ownership moves to 'moved'
// 'data' is no longer valid

processData(moved)  // ownership moves to function
// 'moved' is no longer valid
```

### Borrowing
```alterion
// Borrow without taking ownership
data = createData()
processData(&data)  // borrow immutably
data.cleanup()      // 'data' still valid

// Mutable borrowing
modifyData(@mut data)  // borrow mutably
```

### Automatic Cleanup
```alterion
// Resources automatically cleaned up
{
  file = openFile("data.txt")
  content = file.readAll()
  // file automatically closed here
}
```

## Cross-Platform UI

### Universal Components
The same component works on all platforms:

```alterion
component ResponsiveApp {
  isMobile: Bool = screenWidth < 768
  
  render:
    <div class={isMobile ? "mobile" : "desktop"}>
      <header center>
        <h1>My App</h1>
      </header>
      
      <main flex="column" gap="20px">
        <UserProfile />
        <TodoList />
      </main>
      
      <footer center>
        <p>© 2025 My Company</p>
      </footer>
    </div>
}
```

### Building for Different Targets
```bash
# Native desktop app
alterion build app.alt --target native

# Web application (compiles to WebAssembly)
alterion build app.alt --target web

# Mobile app (native mobile UI)
alterion build app.alt --target mobile

# Development with hot reload
alterion dev app.alt --hot-reload
```

## Development Tools

### CLI Commands
```bash
# Create new project
alterion new my-project

# Build project
alterion build src/ --output dist/

# Run with hot reload
alterion dev src/ --port 3000

# Format code
alterion fmt src/

# Run tests
alterion test

# Type checking
alterion check src/
```

### IDE Support
- **VS Code Extension**: Full IntelliSense, syntax highlighting, debugging
- **Language Server**: Works with any LSP-compatible editor
- **Real-time Error Checking**: Catch errors as you type

## Next Steps

Congratulations! You've learned the basics of Alterion. Here's what to explore next:

1. **[Language Guide](language-guide/)** - Complete language reference
2. **[API Reference](api-reference/)** - Built-in functions and libraries  
3. **[Architecture Guide](architecture.md)** - How Alterion works internally
4. **[Examples](../examples/)** - More complex applications

### Example Projects to Try
- **Calculator App** - Practice with state and events
- **Weather App** - Learn async programming and API calls
- **Chat Application** - Explore real-time features
- **Game** - Build interactive experiences

### Community
- 📧 [Get Help](mailto:chaceberry686@gmail.com) - Questions and support
- 🐛 [Report Bugs](mailto:chaceberry686@gmail.com) - Help improve Alterion
- � [Feature Requests](mailto:chaceberry686@gmail.com) - Suggest new features

---

*Ready to build amazing applications with Alterion? Start coding and see how easy modern programming can be!*
