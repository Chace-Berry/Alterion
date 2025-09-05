# 📚 Alterion Language Guide
# ⚠️ **Note:** The features and syntax described below are future plans for Alterion and may not be fully implemented yet.

> Complete language reference for the Alterion Programming Language

## Table of Contents

1. [Language Overview](#language-overview)
2. [Lexical Structure](#lexical-structure)
3. [Grammar & Syntax](#grammar--syntax)
4. [Type System](#type-system)
5. [Memory Model & Ownership](#memory-model--ownership)
> Comprehensive reference for the Alterion Programming Language

## Table of Contents
1. [Introduction](#introduction)
2. [Syntax Overview](#syntax-overview)
3. [Components](#components)
4. [Variables & Types](#variables--types)
5. [Functions](#functions)
6. [Control Flow](#control-flow)
7. [Async Programming](#async-programming)
8. [Ownership & Borrowing](#ownership--borrowing)
9. [UI & JSX Syntax](#ui--jsx-syntax)
10. [Error Handling](#error-handling)
11. [Modules & Imports](#modules--imports)
12. [Type System](#type-system)
13. [Advanced Patterns](#advanced-patterns)

---

## Introduction

Alterion is a modern, component-first language designed for building cross-platform applications with memory safety, async support, and universal UI.

---

## Syntax Overview

Alterion uses clean, dynamic assignment and modern control flow:

```alterion
// Dynamic assignment
name = "Alice"
age = 25

// Static typing (optional)
age: Int = 25

// Arrays
numbers = [1, 2, 3]
```

---

## Components

Components are the building blocks of Alterion apps:

```alterion
component MyButton {
  label = "Click Me"
  onClick {
    print("Button clicked!")
  }
  render:
    <button onClick={onClick}>{label}</button>
}
```

---

## Variables & Types

Alterion supports both dynamic and static typing:

```alterion
// Dynamic
score = 42
// Static
score: Int = 42
// Type inference
isActive = true
// Arrays
items = ["a", "b", "c"]
```

---

## Functions

Functions can be declared with or without type hints:

```alterion
add(a, b) {
  return a + b
}

add(a: Int, b: Int): Int {
  return a + b
}
```

---

## Control Flow

### If/Else
```alterion
if (x > 0)[
  print("Positive")
][
  print("Non-positive")
]
```

### Loops
```alterion
for i (10)[
  print(i)
]

for item in items[
  print(item)
]
```

---

## Async Programming

Alterion supports async blocks and error handling:

```alterion
@async
fetchData(url) {
  async {
    [response = await httpGet(url)]
    [data = await response.json()]
    [return data]
    [catch(err) return null]
  }
}
```

---

## Ownership & Borrowing

Inspired by Rust, Alterion enforces memory safety:

```alterion
data = createData()
moved = data // ownership moves
data.cleanup() // error: 'data' moved

processData(&data) // borrow
modifyData(@mut data) // mutable borrow
```

---

## UI & JSX Syntax

Alterion uses JSX-like tags for UI:

```alterion
render:
  <div center>
    <h1>Hello, {name}!</h1>
    <button onClick={onClick}>Click</button>
  </div>
```

---

## Error Handling

Use async blocks for robust error handling:

```alterion
async {
  [result = await riskyOperation()]
  [catch(err) print("Error: " + err.message)]
  [finally print("Done!")]
}
```

---

## Modules & Imports

Import other files or libraries:

```alterion
import { MyButton, MyForm } from "./components"
import http from "std/http"
```

---

## Type System

Alterion supports:
- **Type inference**
- **Static types**
- **Union types**
- **Generics**

```alterion
// Union type
value: Int | String = "hello"

// Generic function
identity<T>(x: T): T {
  return x
}
```

---

## Advanced Patterns

### Ownership transfer
```alterion
data = createData()
moved = data // ownership moves
data.cleanup() // error: 'data' moved
```

### Borrowing
```alterion
processData(&data) // borrow
modifyData(@mut data) // mutable borrow
```

### Async with error handling
```alterion
async {
  [result = await fetchData()]
  [catch(err) print("Failed: " + err.message)]
  [finally print("Cleanup!")]
}
```

---

*For more examples, see the [API Reference](../api-reference/README.md) and [Migration Guide](../migration.md).*
// Booleans
let isActive: boolean = true
let isComplete: boolean = false

// Null and Undefined
let empty: null = null
let notSet: undefined = undefined
```

### Collection Types

```alterion
// Arrays
let numbers: array<number> = [1, 2, 3, 4, 5]
let names: array<string> = ["Alice", "Bob", "Charlie"]
let mixed: array = [1, "hello", true, null]

// Objects
let user: object = {
    name: "John",
    age: 30,
    isActive: true
}

// Maps
let colors: map<string, string> = {
    "red": "#FF0000",
    "green": "#00FF00",
    "blue": "#0000FF"
}

// Sets
let uniqueNumbers: set<number> = new Set([1, 2, 3, 2, 1])
```

## Variables and Constants

### Declaration

```alterion
// Type inference
let name = "Alterion"        // string
let count = 0                // number
let isReady = true          // boolean

// Explicit typing
let username: string = "user123"
let age: number = 25
let items: array<string> = []

// Constants
const PI: number = 3.14159
const APP_NAME: string = "My App"
```

### Scope Rules

```alterion
let globalVar = "I'm global"

function example() {
    let localVar = "I'm local"
    
    if (true) {
        let blockVar = "I'm block-scoped"
        // All three variables accessible here
    }
    
    // blockVar not accessible here
    // localVar and globalVar accessible
}

// Only globalVar accessible here
```

## Functions

### Basic Functions

```alterion
// Simple function
function greet(name: string) -> string {
    return `Hello, ${name}!`
}

// Function with multiple parameters
function add(a: number, b: number) -> number {
    return a + b
}

// Function with default parameters
function multiply(a: number, b: number = 1) -> number {
    return a * b
}

// Void function
function log(message: string) {
    console.log(message)
}
```

### Arrow Functions

```alterion
// Arrow function syntax
let square = (x: number) -> number => x * x

// Single expression
let double = (x: number) => x * 2

// Multiple parameters
let sum = (a: number, b: number) => a + b

// No parameters
let getRandom = () => Math.random()
```

### Higher-Order Functions

```alterion
// Function that takes a function as parameter
function apply(value: number, fn: (number) -> number) -> number {
    return fn(value)
}

// Usage
let result = apply(5, (x) => x * x)  // 25

// Function that returns a function
function multiplier(factor: number) -> (number) -> number {
    return (value: number) => value * factor
}

let double = multiplier(2)
let result = double(5)  // 10
```

## Components

### Basic Component

```alterion
component Counter {
    // State variables
    count: number = 0
    
    // Methods
    increment {
        count += 1
    }
    
    decrement {
        count -= 1
    }
    
    reset {
        count = 0
    }
    
    // Render method
    render:
        <div class="counter">
            <h2>Count: {count}</h2>
            <button onClick={increment}>+</button>
            <button onClick={decrement}>-</button>
            <button onClick={reset}>Reset</button>
        </div>
}
```

### Component with Props

```alterion
component UserCard {
    // Props (passed from parent)
    props: {
        name: string,
        email: string,
        avatar?: string
    }
    
    // Local state
    isExpanded: boolean = false
    
    toggle {
        isExpanded = !isExpanded
    }
    
    render:
        <div class="user-card">
            <img src={props.avatar || "/default-avatar.png"} />
            <h3>{props.name}</h3>
            <button onClick={toggle}>
                {isExpanded ? "Collapse" : "Expand"}
            </button>
            {isExpanded && 
                <div>
                    <p>Email: {props.email}</p>
                </div>
            }
        </div>
}

// Usage
<UserCard 
    name="John Doe" 
    email="john@example.com"
    avatar="/john-avatar.jpg"
/>
```

### Lifecycle Methods

```alterion
component DataFetcher {
    data: array = []
    loading: boolean = true
    
    // Called when component mounts
    onMount {
        fetchData()
    }
    
    // Called when component unmounts
    onUnmount {
        cleanup()
    }
    
    // Called when props or state change
    onUpdate(prevProps, prevState) {
        if (prevState.loading && !loading) {
            console.log("Data finished loading")
        }
    }
    
    @async
    fetchData {
        loading = true
        async {
            [data = await api.getData()]
            [catch(error) console.error(error)]
            [finally loading = false]
        }
    }
    
    cleanup {
        // Cleanup resources
    }
    
    render:
        loading ? 
            <div>Loading...</div> :
            <div>
                for item in data [
                    <p key={item.id}>{item.name}</p>
                ]
            </div>
}
```

## Control Flow

### Conditional Statements

```alterion
// If statement
if (age >= 18) {
    console.log("Adult")
} else {
    console.log("Minor")
}

// If-else chain
if (score >= 90) {
    grade = "A"
} else if (score >= 80) {
    grade = "B"
} else if (score >= 70) {
    grade = "C"
} else {
    grade = "F"
}

// Ternary operator
let status = age >= 18 ? "Adult" : "Minor"

// Null coalescing
let name = user.name ?? "Unknown"

// Optional chaining
let city = user?.address?.city
```

### Loops

```alterion
// For loop with range
for i (10) [
    console.log(`Number: ${i}`)
]

// For loop with range and step
for i (0, 100, 2) [
    console.log(`Even number: ${i}`)
]

// For-in loop
let fruits = ["apple", "banana", "orange"]
for fruit in fruits [
    console.log(fruit)
]

// For-in with index
for fruit, index in fruits [
    console.log(`${index}: ${fruit}`)
]

// While loop
let count = 0
while (count < 10) [
    console.log(count)
    count += 1
]

// Loop with break and continue
for i (100) [
    if (i % 2 == 0) continue
    if (i > 50) break
    console.log(i)
]
```

### Pattern Matching

```alterion
// Match statement
match value {
    0 => console.log("Zero"),
    1 | 2 | 3 => console.log("Small number"),
    x if x > 100 => console.log("Large number"),
    _ => console.log("Other number")
}

// Match with destructuring
match user {
    {name: "Admin", role: "admin"} => grantAdminAccess(),
    {role: "user", active: true} => grantUserAccess(),
    _ => denyAccess()
}
```

## Async Programming

### Async Functions

```alterion
@async
function fetchUser(id: number) -> Promise<User> {
    async {
        [
            let response = await fetch(`/api/users/${id}`)
            let user = await response.json()
            return user
        ]
        [catch(error)
            console.error("Failed to fetch user:", error)
            throw error
        ]
    }
}
```

### Async Blocks

```alterion
@async
function processData(items: array<Item>) {
    for item in items [
        async {
            [
                let result = await processItem(item)
                await saveResult(result)
            ]
            [catch(error)
                console.error(`Failed to process ${item.id}:`, error)
            ]
            [finally
                console.log(`Processed ${item.id}`)
            ]
        }
    ]
}
```

### Concurrent Operations

```alterion
@async
function fetchAllData() {
    // Run operations concurrently
    let [users, posts, comments] = await Promise.all([
        fetchUsers(),
        fetchPosts(),
        fetchComments()
    ])
    
    return { users, posts, comments }
}
```

## Error Handling

### Try-Catch in Async Blocks

```alterion
@async
function reliableOperation() {
    async {
        [
            let result = await riskyOperation()
            return result
        ]
        [catch(NetworkError error)
            console.log("Network error:", error.message)
            return await fallbackOperation()
        ]
        [catch(ValidationError error)
            console.log("Validation error:", error.message)
            throw error
        ]
        [catch(error)
            console.log("Unknown error:", error)
            return null
        ]
        [finally
            cleanup()
        ]
    }
}
```

### Result Type

```alterion
function divide(a: number, b: number) -> Result<number, string> {
    if (b == 0) {
        return Error("Division by zero")
    }
    return Ok(a / b)
}

// Usage
match divide(10, 2) {
    Ok(result) => console.log("Result:", result),
    Error(message) => console.error("Error:", message)
}
```

### Optional Type

```alterion
function findUser(id: number) -> Option<User> {
    let user = database.findById(id)
    return user ? Some(user) : None
}

// Usage
match findUser(123) {
    Some(user) => console.log("Found:", user.name),
    None => console.log("User not found")
}
```

## Type System

### Generic Types

```alterion
// Generic function
function identity<T>(value: T) -> T {
    return value
}

let number = identity<number>(42)
let text = identity<string>("hello")

// Generic component
component List<T> {
    props: {
        items: array<T>,
        renderItem: (T) -> JSX.Element
    }
    
    render:
        <div class="list">
            for item in props.items [
                props.renderItem(item)
            ]
        </div>
}
```

### Type Aliases

```alterion
type UserId = number
type UserName = string
type Timestamp = number

type User = {
    id: UserId,
    name: UserName,
    createdAt: Timestamp
}

type ApiResponse<T> = {
    data: T,
    status: number,
    message: string
}
```

### Union Types

```alterion
type Status = "loading" | "success" | "error"
type ID = number | string

function processStatus(status: Status) {
    match status {
        "loading" => showSpinner(),
        "success" => showSuccess(),
        "error" => showError()
    }
}
```

### Intersection Types

```alterion
type Nameable = { name: string }
type Ageable = { age: number }
type Person = Nameable & Ageable

let person: Person = {
    name: "John",
    age: 30
}
```

## Memory Management

### Ownership

```alterion
function processData(data: string) -> string {
    // 'data' is owned by this function
    let processed = data.toUpperCase()
    return processed
    // 'data' is automatically cleaned up
}

let original = "hello world"
let result = processData(original)  // 'original' is moved
// 'original' is no longer accessible here
```

### Borrowing

```alterion
function getLength(data: &string) -> number {
    // 'data' is borrowed (read-only reference)
    return data.length
}

let text = "hello world"
let length = getLength(&text)  // borrow 'text'
console.log(text)  // 'text' is still accessible
```

### Mutable Borrowing

```alterion
function appendText(data: &mut string, suffix: string) {
    // 'data' is mutably borrowed
    data.append(suffix)
}

let mut message = "Hello"
appendText(&mut message, " World")
console.log(message)  // "Hello World"
```

## Modules and Imports

### Module Definition

```alterion
// math.alt
export function add(a: number, b: number) -> number {
    return a + b
}

export function multiply(a: number, b: number) -> number {
    return a * b
}

export const PI = 3.14159

export component Calculator {
    // Component implementation
}
```

### Importing

```alterion
// Import specific items
import { add, multiply, PI } from "./math"

// Import all
import * as Math from "./math"

// Import with alias
import { Calculator as Calc } from "./math"

// Import default export
import DefaultComponent from "./component"
```

### Re-exports

```alterion
// index.alt
export { add, multiply } from "./math"
export { default as Button } from "./button"
export * from "./utils"
```

## Standard Library

### Console

```alterion
console.log("Info message")
console.error("Error message")
console.warn("Warning message")
console.debug("Debug message")
```

### Math

```alterion
Math.abs(-5)        // 5
Math.max(1, 2, 3)   // 3
Math.min(1, 2, 3)   // 1
Math.round(3.7)     // 4
Math.floor(3.7)     // 3
Math.ceil(3.2)      // 4
Math.random()       // 0.0 to 1.0
Math.sqrt(16)       // 4
Math.pow(2, 3)      // 8
```

### String Methods

```alterion
let text = "Hello, World!"

text.length              // 13
text.toUpperCase()       // "HELLO, WORLD!"
text.toLowerCase()       // "hello, world!"
text.substring(0, 5)     // "Hello"
text.indexOf("World")    // 7
text.replace("World", "Alterion")  // "Hello, Alterion!"
text.split(", ")         // ["Hello", "World!"]
text.trim()              // Remove whitespace
text.startsWith("Hello") // true
text.endsWith("!")       // true
```

### Array Methods

```alterion
let numbers = [1, 2, 3, 4, 5]

numbers.length           // 5
numbers.push(6)          // Add to end
numbers.pop()            // Remove from end
numbers.shift()          // Remove from start
numbers.unshift(0)       // Add to start
numbers.slice(1, 3)      // [2, 3]
numbers.indexOf(3)       // 2
numbers.includes(4)      // true
numbers.reverse()        // Reverse in place

// Higher-order methods
numbers.map(x => x * 2)           // [2, 4, 6, 8, 10]
numbers.filter(x => x % 2 == 0)   // [2, 4]
numbers.reduce((sum, x) => sum + x, 0)  // 15
numbers.forEach(x => console.log(x))
```

### Object Methods

```alterion
let obj = { a: 1, b: 2, c: 3 }

Object.keys(obj)         // ["a", "b", "c"]
Object.values(obj)       // [1, 2, 3]
Object.entries(obj)      // [["a", 1], ["b", 2], ["c", 3]]
Object.hasOwnProperty(obj, "a")  // true
Object.assign({}, obj, { d: 4 }) // { a: 1, b: 2, c: 3, d: 4 }
```

### Date and Time

```alterion
let now = Date.now()            // Current timestamp
let date = new Date()           // Current date
let specific = new Date("2023-12-25")

date.getFullYear()             // Year
date.getMonth()                // Month (0-11)
date.getDate()                 // Day of month
date.getHours()                // Hours
date.getMinutes()              // Minutes
date.getSeconds()              // Seconds
date.toISOString()             // ISO format string
```

---

This language guide covers the core features of Alterion. For more advanced topics and examples, see the [API Reference](../api-reference/) and [Examples](../../examples/) directories.
