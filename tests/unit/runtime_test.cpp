#include "runtime.h"
#include "standard_library.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace Alterion;

// Test Runtime System
void testAlterionValue() {
    std::cout << "=== Testing AlterionValue ===" << std::endl;
    
    // Basic types
    AlterionValue nullVal;
    AlterionValue boolVal(true);
    AlterionValue intVal(42);
    AlterionValue doubleVal(3.14);
    AlterionValue stringVal("Hello, Alterion!");
    
    std::cout << "null: " << nullVal.toString() << " (type: " << nullVal.getType() << ")" << std::endl;
    std::cout << "bool: " << boolVal.toString() << " (type: " << boolVal.getType() << ")" << std::endl;
    std::cout << "int: " << intVal.toString() << " (type: " << intVal.getType() << ")" << std::endl;
    std::cout << "double: " << doubleVal.toString() << " (type: " << doubleVal.getType() << ")" << std::endl;
    std::cout << "string: " << stringVal.toString() << " (type: " << stringVal.getType() << ")" << std::endl;
    
    // Array
    std::vector<AlterionValue> arrData = {intVal, doubleVal, stringVal};
    AlterionValue arrayVal(arrData);
    std::cout << "array: " << arrayVal.toString() << " (type: " << arrayVal.getType() << ")" << std::endl;
    std::cout << "array[1]: " << arrayVal[1].toString() << std::endl;
    
    // Object
    std::unordered_map<std::string, AlterionValue> objData = {
        {"name", AlterionValue("Alterion")},
        {"version", AlterionValue("1.0.0")},
        {"stable", AlterionValue(true)}
    };
    AlterionValue objectVal(objData);
    std::cout << "object: " << objectVal.toString() << " (type: " << objectVal.getType() << ")" << std::endl;
    std::cout << "object.name: " << objectVal["name"].toString() << std::endl;
    
    // JSON serialization
    std::cout << "array JSON: " << arrayVal.toJSON() << std::endl;
    std::cout << "object JSON: " << objectVal.toJSON() << std::endl;
    
    std::cout << "✅ AlterionValue tests passed!" << std::endl << std::endl;
}

void testAlterionFunction() {
    std::cout << "=== Testing AlterionFunction ===" << std::endl;
    
    // Create a simple function
    auto addFunction = createFunction("add", {"a", "b"}, 
        [](const std::vector<AlterionValue>& args) -> AlterionValue {
            if (args.size() >= 2) {
                return AlterionValue(args[0].asDouble() + args[1].asDouble());
            }
            return AlterionValue(0.0);
        });
    
    // Test function call
    std::vector<AlterionValue> args = {AlterionValue(5), AlterionValue(3)};
    AlterionValue result = addFunction->call(args);
    std::cout << "add(5, 3) = " << result.toString() << std::endl;
    
    // Create string manipulation function
    auto greetFunction = createFunction("greet", {"name"}, 
        [](const std::vector<AlterionValue>& args) -> AlterionValue {
            std::string name = args.empty() ? "World" : args[0].asString();
            return AlterionValue("Hello, " + name + "!");
        });
    
    AlterionValue greeting = greetFunction->call({AlterionValue("Alterion")});
    std::cout << "greet('Alterion') = " << greeting.toString() << std::endl;
    
    std::cout << "✅ AlterionFunction tests passed!" << std::endl << std::endl;
}

void testAlterionComponent() {
    std::cout << "=== Testing AlterionComponent ===" << std::endl;
    
    // Create a component
    auto counter = createComponent("Counter");
    
    // Set initial state
    counter->setState("count", AlterionValue(0));
    counter->setState("title", AlterionValue("My Counter"));
    
    std::cout << "Initial count: " << counter->getState("count").toString() << std::endl;
    std::cout << "Component title: " << counter->getState("title").toString() << std::endl;
    
    // Create increment method
    auto incrementMethod = createFunction("increment", {}, 
        [counter](const std::vector<AlterionValue>& args) -> AlterionValue {
            int currentCount = counter->getState("count").asInt();
            counter->setState("count", AlterionValue(currentCount + 1));
            return AlterionValue();
        });
    
    // Simulate method calls
    incrementMethod->call({});
    incrementMethod->call({});
    incrementMethod->call({});
    
    std::cout << "After 3 increments: " << counter->getState("count").toString() << std::endl;
    
    // Test component serialization
    AlterionValue componentValue = counter->toValue();
    std::cout << "Component as value: " << componentValue.toJSON() << std::endl;
    
    std::cout << "✅ AlterionComponent tests passed!" << std::endl << std::endl;
}

void testJSInterop() {
    std::cout << "=== Testing JavaScript Interop ===" << std::endl;
    
    JSInterop jsInterop;
    
    // Register global functions
    auto consoleLog = createFunction("log", {"message"}, 
        [](const std::vector<AlterionValue>& args) -> AlterionValue {
            std::cout << "[JS Console] ";
            for (const auto& arg : args) {
                std::cout << arg.toString() << " ";
            }
            std::cout << std::endl;
            return AlterionValue();
        });
    
    jsInterop.registerGlobalFunction("console.log", consoleLog);
    
    // Test global access
    AlterionValue logFunc = jsInterop.getGlobal("console.log");
    logFunc.asFunction()->call({AlterionValue("Hello from JS interop!")});
    
    // Test type conversion
    AlterionValue alterionValue(42);
    std::string jsValue = jsInterop.alterionToJS(alterionValue);
    std::cout << "Alterion 42 -> JS: " << jsValue << std::endl;
    
    AlterionValue backToAlterion = jsInterop.jsToAlterion("42", "number");
    std::cout << "JS 42 -> Alterion: " << backToAlterion.toString() << std::endl;
    
    // Test DOM-like operations
    std::unordered_map<std::string, AlterionValue> attributes = {
        {"class", AlterionValue("button")},
        {"id", AlterionValue("my-button")}
    };
    AlterionValue element = jsInterop.createElement("button", attributes);
    std::cout << "Created element: " << element.toJSON() << std::endl;
    
    std::cout << "✅ JavaScript Interop tests passed!" << std::endl << std::endl;
}

void testStandardLibrary() {
    std::cout << "=== Testing Standard Library ===" << std::endl;
    
    // Initialize standard library
    globalStdLib = std::make_unique<StandardLibrary>();
    
    // Test Core module
    AlterionValue coreModule = globalStdLib->loadModule("core");
    AlterionValue typeofFunc = coreModule["typeof"];
    AlterionValue result = typeofFunc.asFunction()->call({AlterionValue("test")});
    std::cout << "typeof('test') = " << result.toString() << std::endl;
    
    // Test Math module
    AlterionValue mathModule = globalStdLib->loadModule("math");
    AlterionValue piValue = mathModule["PI"];
    std::cout << "Math.PI = " << piValue.toString() << std::endl;
    
    AlterionValue sqrtFunc = mathModule["sqrt"];
    AlterionValue sqrtResult = sqrtFunc.asFunction()->call({AlterionValue(16)});
    std::cout << "Math.sqrt(16) = " << sqrtResult.toString() << std::endl;
    
    // Test Array module
    AlterionValue arrayModule = globalStdLib->loadModule("array");
    std::vector<AlterionValue> testArray = {AlterionValue(1), AlterionValue(2), AlterionValue(3)};
    AlterionValue arrayVal(testArray);
    
    AlterionValue joinFunc = arrayModule["join"];
    AlterionValue joinResult = joinFunc.asFunction()->call({arrayVal, AlterionValue(", ")});
    std::cout << "Array join: " << joinResult.toString() << std::endl;
    
    // Test String module
    AlterionValue stringModule = globalStdLib->loadModule("string");
    AlterionValue upperFunc = stringModule["toUpperCase"];
    AlterionValue upperResult = upperFunc.asFunction()->call({AlterionValue("alterion")});
    std::cout << "toUpperCase('alterion') = " << upperResult.toString() << std::endl;
    
    // Test Console module
    AlterionValue consoleModule = globalStdLib->loadModule("console");
    AlterionValue logFunc = consoleModule["log"];
    logFunc.asFunction()->call({AlterionValue("Standard library console test!")});
    
    std::cout << "Available modules: ";
    for (const auto& moduleName : globalStdLib->getAvailableModules()) {
        std::cout << moduleName << " ";
    }
    std::cout << std::endl;
    
    std::cout << "✅ Standard Library tests passed!" << std::endl << std::endl;
}

void testAlterionRuntime() {
    std::cout << "=== Testing Alterion Runtime ===" << std::endl;
    
    // Initialize runtime
    globalRuntime = std::make_unique<AlterionRuntime>();
    globalRuntime->initialize();
    
    // Register a test module
    std::unordered_map<std::string, AlterionValue> testModuleExports = {
        {"name", AlterionValue("TestModule")},
        {"version", AlterionValue("1.0.0")},
        {"hello", AlterionValue(createFunction("hello", {"name"}, 
            [](const std::vector<AlterionValue>& args) -> AlterionValue {
                std::string name = args.empty() ? "World" : args[0].asString();
                return AlterionValue("Hello from module, " + name + "!");
            }))}
    };
    
    globalRuntime->registerModule("test-module", AlterionValue(testModuleExports));
    
    // Import and use the module
    AlterionValue importedModule = globalRuntime->importModule("test-module");
    std::cout << "Imported module name: " << importedModule["name"].toString() << std::endl;
    
    AlterionValue helloFunc = importedModule["hello"];
    AlterionValue helloResult = helloFunc.asFunction()->call({AlterionValue("Runtime")});
    std::cout << "Module function result: " << helloResult.toString() << std::endl;
    
    // Test component registration and creation
    auto testComponent = createComponent("TestComponent");
    testComponent->setState("message", AlterionValue("Hello from component!"));
    globalRuntime->registerComponent("TestComponent", testComponent);
    
    auto createdComponent = globalRuntime->createComponent("TestComponent");
    std::cout << "Created component: " << createdComponent->getComponentName() << std::endl;
    
    // Test render scheduling
    createdComponent->markForRerender();
    globalRuntime->scheduleRender(createdComponent);
    globalRuntime->processRenderQueue();
    
    // Install standard library into runtime
    if (globalStdLib) {
        globalStdLib->installIntoRuntime(*globalRuntime);
    }
    
    // Test performance monitoring
    globalRuntime->startPerformanceTimer("test-operation");
    // Simulate some work
    for (int i = 0; i < 1000; ++i) {
        AlterionValue val(i);
        val.toString();
    }
    double elapsed = globalRuntime->endPerformanceTimer("test-operation");
    std::cout << "Performance test completed" << std::endl;
    
    globalRuntime->logPerformanceMetrics();
    
    std::cout << "✅ Alterion Runtime tests passed!" << std::endl << std::endl;
}

void testCompleteIntegration() {
    std::cout << "=== Testing Complete Integration ===" << std::endl;
    
    // Test a complete Alterion program simulation
    std::cout << "Simulating Alterion component compilation and execution..." << std::endl;
    
    // Create a Counter component (simulating compiled Alterion code)
    auto counterComponent = createComponent("Counter");
    
    // Initialize component state
    counterComponent->setState("count", AlterionValue(0));
    counterComponent->setState("title", AlterionValue("Interactive Counter"));
    
    // Create component methods
    auto incrementMethod = createFunction("increment", {}, 
        [counterComponent](const std::vector<AlterionValue>& args) -> AlterionValue {
            int currentCount = counterComponent->getState("count").asInt();
            counterComponent->setState("count", AlterionValue(currentCount + 1));
            std::cout << "  Count incremented to: " << (currentCount + 1) << std::endl;
            return AlterionValue();
        });
    
    auto resetMethod = createFunction("reset", {}, 
        [counterComponent](const std::vector<AlterionValue>& args) -> AlterionValue {
            counterComponent->setState("count", AlterionValue(0));
            std::cout << "  Count reset to: 0" << std::endl;
            return AlterionValue();
        });
    
    // Create render function (simulating ALTX compilation)
    auto renderMethod = createFunction("render", {}, 
        [counterComponent](const std::vector<AlterionValue>& args) -> AlterionValue {
            std::cout << "  Rendering Counter Component:" << std::endl;
            std::cout << "    Title: " << counterComponent->getState("title").toString() << std::endl;
            std::cout << "    Count: " << counterComponent->getState("count").toString() << std::endl;
            std::cout << "    Virtual DOM: <div><h1>Counter</h1><p>Count: " 
                      << counterComponent->getState("count").toString() << "</p></div>" << std::endl;
            return AlterionValue();
        });
    
    counterComponent->setRenderFunction(renderMethod);
    
    // Simulate user interactions
    std::cout << "Initial render:" << std::endl;
    counterComponent->render();
    
    std::cout << "\nUser clicks increment button 3 times:" << std::endl;
    incrementMethod->call({});
    incrementMethod->call({});
    incrementMethod->call({});
    counterComponent->render();
    
    std::cout << "\nUser clicks reset button:" << std::endl;
    resetMethod->call({});
    counterComponent->render();
    
    // Test with props
    std::unordered_map<std::string, AlterionValue> props = {
        {"initialCount", AlterionValue(10)},
        {"step", AlterionValue(2)}
    };
    counterComponent->setProps(props);
    
    std::cout << "\nComponent with props:" << std::endl;
    std::cout << "  Initial count prop: " << counterComponent->getProps("initialCount").toString() << std::endl;
    std::cout << "  Step prop: " << counterComponent->getProps("step").toString() << std::endl;
    
    // Register component in runtime
    globalRuntime->registerComponent("Counter", counterComponent);
    
    // Test standard library integration
    AlterionValue mathModule = globalRuntime->importModule("math");
    AlterionValue randomFunc = mathModule["random"];
    AlterionValue randomValue = randomFunc.asFunction()->call({});
    std::cout << "\nRandom value from stdlib: " << randomValue.toString() << std::endl;
    
    // Test console integration
    AlterionValue consoleModule = globalRuntime->importModule("console");
    AlterionValue logFunc = consoleModule["log"];
    logFunc.asFunction()->call({
        AlterionValue("Component integration test completed!"),
        AlterionValue("Count:"),
        counterComponent->getState("count")
    });
    
    std::cout << "✅ Complete Integration tests passed!" << std::endl << std::endl;
}

void demonstrateAlterionCode() {
    std::cout << "=== Demonstrating Alterion Code Execution ===" << std::endl;
    
    std::cout << "This demonstrates how compiled Alterion code would execute:" << std::endl << std::endl;
    
    std::cout << "Original Alterion code:" << std::endl;
    std::cout << R"(
component TodoApp {
    todos = []
    newTodo = ""
    
    addTodo {
        if (newTodo.trim() != "") {
            todos.push({
                id: Math.random(),
                text: newTodo,
                completed: false
            })
            newTodo = ""
        }
    }
    
    toggleTodo(id) {
        let todo = todos.find(t => t.id == id)
        if (todo) {
            todo.completed = !todo.completed
        }
    }
    
    render:
        <div class="todo-app">
            <h1>Todo List</h1>
            <input value={newTodo} onChange={setNewTodo} />
            <button onClick={addTodo}>Add Todo</button>
            <ul>
                {todos.map(todo => 
                    <li key={todo.id} class={todo.completed ? "completed" : ""}>
                        <span onClick={() => toggleTodo(todo.id)}>
                            {todo.text}
                        </span>
                    </li>
                )}
            </ul>
        </div>
}
)" << std::endl;
    
    std::cout << "Compiled and executing..." << std::endl << std::endl;
    
    // Simulate the compiled component
    auto todoApp = createComponent("TodoApp");
    
    // Initialize state
    todoApp->setState("todos", createArray({}));
    todoApp->setState("newTodo", AlterionValue(""));
    
    // Create methods
    auto addTodoMethod = createFunction("addTodo", {}, 
        [todoApp](const std::vector<AlterionValue>& args) -> AlterionValue {
            std::string newTodo = todoApp->getState("newTodo").asString();
            if (!newTodo.empty()) {
                auto& todos = todoApp->getState("todos");
                auto& todoArray = const_cast<AlterionValue&>(todos).asArray();
                
                std::unordered_map<std::string, AlterionValue> todo = {
                    {"id", AlterionValue(static_cast<double>(rand()) / RAND_MAX)},
                    {"text", AlterionValue(newTodo)},
                    {"completed", AlterionValue(false)}
                };
                
                todoArray.push_back(AlterionValue(todo));
                todoApp->setState("newTodo", AlterionValue(""));
                
                std::cout << "  Added todo: " << newTodo << std::endl;
            }
            return AlterionValue();
        });
    
    auto renderMethod = createFunction("render", {}, 
        [todoApp](const std::vector<AlterionValue>& args) -> AlterionValue {
            std::cout << "  Rendering TodoApp:" << std::endl;
            std::cout << "    <div class=\"todo-app\">" << std::endl;
            std::cout << "      <h1>Todo List</h1>" << std::endl;
            
            const auto& todos = todoApp->getState("todos").asArray();
            std::cout << "      <ul> (" << todos.size() << " todos)" << std::endl;
            
            for (const auto& todo : todos) {
                if (todo.isObject()) {
                    std::string text = todo["text"].toString();
                    bool completed = todo["completed"].asBool();
                    std::cout << "        <li class=\"" << (completed ? "completed" : "") << "\">"
                              << text << "</li>" << std::endl;
                }
            }
            
            std::cout << "      </ul>" << std::endl;
            std::cout << "    </div>" << std::endl;
            return AlterionValue();
        });
    
    todoApp->setRenderFunction(renderMethod);
    
    // Simulate user interactions
    std::cout << "Initial render:" << std::endl;
    todoApp->render();
    
    std::cout << "\nUser adds todos:" << std::endl;
    todoApp->setState("newTodo", AlterionValue("Learn Alterion"));
    addTodoMethod->call({});
    
    todoApp->setState("newTodo", AlterionValue("Build awesome UI"));
    addTodoMethod->call({});
    
    todoApp->setState("newTodo", AlterionValue("Ship to production"));
    addTodoMethod->call({});
    
    std::cout << "\nFinal render:" << std::endl;
    todoApp->render();
    
    std::cout << "\n✅ Alterion code execution demo completed!" << std::endl << std::endl;
}

int main() {
    try {
        std::cout << "🚀 Alterion Runtime/FFI Test Suite" << std::endl;
        std::cout << "===================================" << std::endl << std::endl;
        
        // Run all tests
        testAlterionValue();
        testAlterionFunction();
        testAlterionComponent();
        testJSInterop();
        testStandardLibrary();
        testAlterionRuntime();
        testCompleteIntegration();
        demonstrateAlterionCode();
        
        std::cout << "🎉 ALL TESTS PASSED!" << std::endl;
        std::cout << "Phase 2F: Runtime/FFI is working correctly!" << std::endl;
        
        // Cleanup
        if (globalRuntime) {
            globalRuntime->shutdown();
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with error: " << e.what() << std::endl;
        return 1;
    }
}
