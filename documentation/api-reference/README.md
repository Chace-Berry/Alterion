# 🔧 Alterion API Reference
# ⚠️ **Note:** The features and syntax described below are future plans for Alterion and may not be fully implemented yet.

> Detailed API documentation for the Alterion Programming Language

## Built-in Types

### Primitive Types

#### Int
Integer numbers (64-bit signed)
```alterion
age = 25
count = 0
```

#### Float  
Floating-point numbers (64-bit double precision)
```alterion
pi = 3.14159
temperature = 98.6
```

#### Bool
Boolean values
```alterion
isActive = true
isDone = false
```

#### String
Text strings (UTF-8 encoded)
```alterion
name = "Alice"
message = "Hello, Alterion!"

// String methods
length = name.length()        // 5
upper = name.toUpperCase()    // "ALICE"
lower = name.toLowerCase()    // "alice"
trimmed = message.trim()      // removes whitespace
```

#### Void
Represents no return value
```alterion
printMessage() {
  print("Hello")
}
```

#### None
Null/undefined value
```alterion
user = none
if (user == none) {
  print("No user found")
}
```

### Collection Types

#### Arrays
Ordered collections of elements
```alterion
numbers = [1, 2, 3, 4, 5]
names = ["Alice", "Bob", "Charlie"]

// Array methods
length = numbers.length()     // 5
numbers.push(6)              // [1, 2, 3, 4, 5, 6]
last = numbers.pop()         // 6, array becomes [1, 2, 3, 4, 5]
first = numbers.shift()      // 1, array becomes [2, 3, 4, 5]
numbers.unshift(0)           // [0, 2, 3, 4, 5]
numbers.insert(1, 1)         // [0, 1, 2, 3, 4, 5]
numbers.removeAt(0)          // [1, 2, 3, 4, 5]
```

#### Objects
Key-value collections
```alterion
user = {
  name: "Alice",
  age: 30,
  email: "alice@example.com"
}

// Access properties
userName = user.name         // "Alice"
user.age = 31               // Update value
```


Key-value collections, used for structured data and component props.
```alterion
user = {
  name: "Alice",
  age: 30,
  email: "alice@example.com"
}
```

## Built-in Functions
Array of any type, e.g. `[Int]`, `[String]`, `[Object]`.
```alterion
numbers = [1, 2, 3]
names = ["Alice", "Bob"]
```
### Console Output
```alterion
print(message) {
  // Outputs message to console
}

print("Hello, World!")
print("Number: " + 42.toString())
```

### String Functions
```alterion
// String manipulation
concat(str1, str2)
substring(str, start, end)
replace(str, search, replace)
split(str, delimiter)

// Usage examples
full = concat("Hello", " World")     // "Hello World"
sub = substring("Hello", 1, 3)       // "el"  
replaced = replace("Hello", "l", "x") // "Hexxo"
parts = split("a,b,c", ",")          // ["a", "b", "c"]
```

### Numeric Functions
```alterion
// Math operations
x = 3.4
base = 1.2

abs(x)          // Absolute value
sqrt(x)         // Square root
pow(base, exp)  // Exponentiation
floor(x)        // Round down
ceil(x)         // Round up
round(x)        // Round to nearest

// Usage
result1 = abs(-5.5)          // 5.5
result2 = sqrt(16.0)         // 4.0
result3 = pow(2.0, 3.0)      // 8.0
```

### Type Conversion
age = 25
```alterion
// To String
toString(value)

age = 25
message = "I am " + age.toString() + " years old"

// To Numbers
parseInt(value)
parseFloat(value)

number = parseInt("123")      // 123
decimal = parseFloat("3.14")  // 3.14
```

## Component API

### Component Lifecycle
```alterion
component MyComponent {
  // Constructor (implicit)
  // Called when component is created
  
  onCreate() {
    // Called after component creation
    print("Component created")
  }
  
  onMount() {
    // Called when component is mounted to UI
    print("Component mounted")
  }
  
  onUpdate(oldProps: Object, newProps: Object) {
    // Called when properties change
    print("Component updated")
  }
  
  onUnmount() {
    // Called when component is removed
    print("Component unmounted")
  }
}
```

### State Management
```alterion
component StatefulComponent {
  // State variables automatically trigger re-render
  count = 0
  name = "Default"
  
  // Method to update state
  updateCount(newCount: Int) {
    count = newCount  // Triggers re-render
  }
  
  // Computed properties
  doubleCount: Int {
    get {
      return count * 2
    }
  }
}
```

## Async API

### Async Functions
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

@async
loadUser(userId) {
  loading = true
  async {
    [user = await fetchData("/api/user/" + userId)]
    [catch(err) error = err.message]
    [finally loading = false]
  }
}
```

### HTTP Client
```alterion
// HTTP methods
@async httpGet(url)
@async httpPost(url, data)
@async httpPut(url, data)
@async httpDelete(url)

// Response object
struct Response {
  status
  headers
  @async json()
  @async text()
  @async bytes()
}

@async
apiExample() {
  async {
    [response = await httpGet("/api/users")]
    [users = await response.json()]
    [newUser = { name: "Alice", email: "alice@example.com" }]
    [createResponse = await httpPost("/api/users", newUser)]
    [if (createResponse.status == 201) print("User created successfully")]
    [catch(err) print("API error: " + err.message)]
  }
}
```

### Timers and Delays
```alterion
@async delay(ms)
setTimeout(callback, ms)
clearTimeout(id)
setInterval(callback, ms)
clearInterval(id)

@async
example() {
  print("Starting...")
  await delay(1000)  // Wait 1 second
  print("Done!")
}

timerId = setTimeout(() => {
  print("Timer fired!")
}, 2000)

clearTimeout(timerId)
```

## UI API

### Event Handling
```alterion
// Mouse events
onClick: Function
onDoubleClick: Function
onMouseDown: Function
onMouseUp: Function
onMouseMove: Function
onMouseEnter: Function
onMouseLeave: Function

// Keyboard events  
onKeyDown: Function
onKeyUp: Function
onKeyPress: Function

// Form events
onChange: Function
onSubmit: Function
onFocus: Function
onBlur: Function

// Usage example
component Button {
  handleClick(event: MouseEvent) {
    print("Button clicked!")
    print("Mouse position: " + event.x + ", " + event.y)
  }
  
  render:
    <button onClick={handleClick}>
      Click me
    </button>
}
```

### Event Objects
```alterion
struct MouseEvent {
  x: Int          // Mouse X coordinate
  y: Int          // Mouse Y coordinate  
  button: Int     // Which mouse button (0=left, 1=middle, 2=right)
  ctrlKey: Bool   // Was Ctrl key pressed
  shiftKey: Bool  // Was Shift key pressed
  altKey: Bool    // Was Alt key pressed
}

struct KeyboardEvent {
  key: String     // Key name ("a", "Enter", "Escape", etc.)
  keyCode: Int    // Numeric key code
  ctrlKey: Bool   // Was Ctrl key pressed
  shiftKey: Bool  // Was Shift key pressed
  altKey: Bool    // Was Alt key pressed
}

struct FormEvent {
  target: Element // The form element
  value: String   // Current value of input
}
```

### Element References
```alterion
component FormExample {
  inputRef: ElementRef = createRef()
  
  focusInput() {
    inputRef.current.focus()
  }
  
  render:
    <div>
      <input ref={inputRef} />
      <button onClick={focusInput}>Focus Input</button>
    </div>
}
```

## File System API

### File Operations
```alterion
// File reading
@async readFile(path: String): Future<String>
@async readBytes(path: String): Future<[Int]>

// File writing
@async writeFile(path: String, content: String): Future<Void>
@async writeBytes(path: String, data: [Int]): Future<Void>

// File info
@async fileExists(path: String): Future<Bool>
@async fileSize(path: String): Future<Int>
@async isDirectory(path: String): Future<Bool>

// Directory operations
@async listDirectory(path: String): Future<[String]>
@async createDirectory(path: String): Future<Void>
@async removeDirectory(path: String): Future<Void>

// Usage example
@async
fileExample() {
  // Read file
  content = await readFile("data.txt")
  print("File content: " + content)
  
  // Write file
  await writeFile("output.txt", "Hello, Alterion!")
  
  // Check if file exists
  exists = await fileExists("config.json")
  if (exists) {
    configData = await readFile("config.json")
  }
}
```

## Foreign Function Interface (FFI)

### C Library Integration
```alterion
// Declare external C functions
extern sqrt(x: Float): Float from "libm.so"
extern malloc(size: Int): Pointer from "libc.so"
extern free(ptr: Pointer): Void from "libc.so"

// String functions
extern strlen(str: CString): Int from "libc.so"
extern strcpy(dest: CString, src: CString): CString from "libc.so"

// Usage
result = sqrt(16.0)  // 4.0

// Memory management
ptr = malloc(1024)   // Allocate 1KB
// ... use memory ...
free(ptr)           // Free memory
```

### Type Mappings
```alterion
// Alterion Type -> C Type
Int             -> int64_t
Float           -> double  
Bool            -> bool
String          -> char* (UTF-8)
CString         -> char* (null-terminated)
Pointer         -> void*
[Int]           -> int64_t*
Function        -> function pointer
```

## Error Handling API

### Exception Types
```alterion
struct Error {
  message: String
  code: Int
  stack: String
}

struct NetworkError : Error {
  statusCode: Int
  url: String
}

struct ValidationError : Error {
  field: String
  value: Any
}

struct FileError : Error {
  path: String
  operation: String
}
```

### Throwing and Catching
```alterion
// Throwing exceptions
validateAge(age: Int) {
  if (age < 0) {
    throw ValidationError {
      message: "Age cannot be negative",
      field: "age", 
      value: age
    }
  }
}

// Catching specific exception types
@async
handleErrors() {
  async{[
    data = await fetchData("/api/users")
    validateAge(data.age)
  ][
    catch(NetworkError err) {
      print("Network error: " + err.message)
      print("Status code: " + err.statusCode)
    }
    catch(ValidationError err) {
      print("Validation error: " + err.message)
      print("Invalid field: " + err.field)
    }
    catch(Error err) {
      print("General error: " + err.message)
    }
  ][
    finally {
      print("Cleanup completed")
    }
  ]}
}
```

## Testing API

### Test Functions
```alterion
// Assert functions
assert(condition: Bool): Void
assertEqual(actual: Any, expected: Any): Void
assertNotEqual(actual: Any, expected: Any): Void
assertNull(value: Any): Void
assertNotNull(value: Any): Void

// Test decorators
@test
testAddition() {
  result = add(2, 3)
  assertEqual(result, 5)
}

@test
@async
testAsyncFunction() {
  result = await fetchData("/test-data")
  assertNotNull(result)
  assertEqual(result.status, "success")
}
```

---

*This API reference covers all built-in functions and types available in Alterion. For more advanced usage and examples, see the [Language Guide](language-guide/) and [Getting Started](getting-started.md) documentation.*
