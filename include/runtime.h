#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <variant>
#include <any>
#include <future>
#include <exception>

namespace Alterion {

// Forward declarations
class AlterionValue;
class AlterionObject;
class AlterionFunction;
class AlterionComponent;
class JSInterop;

// Alterion Value System - represents any Alterion value at runtime
using AlterionValueVariant = std::variant<
    std::nullptr_t,
    bool,
    int64_t,
    double,
    std::string,
    std::shared_ptr<AlterionObject>,
    std::shared_ptr<AlterionFunction>,
    std::shared_ptr<AlterionComponent>,
    std::vector<AlterionValue>,
    std::unordered_map<std::string, AlterionValue>
>;

class AlterionValue {
private:
    AlterionValueVariant value;
    
public:
    // Constructors
    AlterionValue() : value(nullptr) {}
    AlterionValue(bool b) : value(b) {}
    AlterionValue(int i) : value(static_cast<int64_t>(i)) {}
    AlterionValue(int64_t i) : value(i) {}
    AlterionValue(double d) : value(d) {}
    AlterionValue(const std::string& s) : value(s) {}
    AlterionValue(const char* s) : value(std::string(s)) {}
    AlterionValue(std::shared_ptr<AlterionObject> obj) : value(obj) {}
    AlterionValue(std::shared_ptr<AlterionFunction> func) : value(func) {}
    AlterionValue(std::shared_ptr<AlterionComponent> comp) : value(comp) {}
    AlterionValue(const std::vector<AlterionValue>& arr) : value(arr) {}
    AlterionValue(const std::unordered_map<std::string, AlterionValue>& obj) : value(obj) {}
    
    // Type checking
    bool isNull() const { return std::holds_alternative<std::nullptr_t>(value); }
    bool isBool() const { return std::holds_alternative<bool>(value); }
    bool isNumber() const { return std::holds_alternative<int64_t>(value) || std::holds_alternative<double>(value); }
    bool isString() const { return std::holds_alternative<std::string>(value); }
    bool isArray() const { return std::holds_alternative<std::vector<AlterionValue>>(value); }
    bool isObject() const { return std::holds_alternative<std::unordered_map<std::string, AlterionValue>>(value) || 
                                  std::holds_alternative<std::shared_ptr<AlterionObject>>(value); }
    bool isFunction() const { return std::holds_alternative<std::shared_ptr<AlterionFunction>>(value); }
    bool isComponent() const { return std::holds_alternative<std::shared_ptr<AlterionComponent>>(value); }
    
    // Value access
    bool asBool() const;
    int64_t asInt() const;
    double asDouble() const;
    std::string asString() const;
    std::vector<AlterionValue>& asArray();
    const std::vector<AlterionValue>& asArray() const;
    std::unordered_map<std::string, AlterionValue>& asObjectMap();
    const std::unordered_map<std::string, AlterionValue>& asObjectMap() const;
    std::shared_ptr<AlterionObject> asObject() const;
    std::shared_ptr<AlterionFunction> asFunction() const;
    std::shared_ptr<AlterionComponent> asComponent() const;
    
    // Operators
    AlterionValue operator[](const std::string& key) const;
    AlterionValue& operator[](const std::string& key);
    AlterionValue operator[](size_t index) const;
    AlterionValue& operator[](size_t index);
    
    // JavaScript conversion
    std::string toJSON() const;
    static AlterionValue fromJSON(const std::string& json);
    
    // String representation
    std::string toString() const;
    std::string getType() const;
};

// Alterion Object - base class for all Alterion objects
class AlterionObject {
protected:
    std::unordered_map<std::string, AlterionValue> properties;
    std::unordered_map<std::string, std::shared_ptr<AlterionFunction>> methods;
    
public:
    virtual ~AlterionObject() = default;
    
    // Property access
    virtual AlterionValue getProperty(const std::string& name) const;
    virtual void setProperty(const std::string& name, const AlterionValue& value);
    virtual bool hasProperty(const std::string& name) const;
    virtual std::vector<std::string> getPropertyNames() const;
    
    // Method access
    virtual AlterionValue callMethod(const std::string& name, const std::vector<AlterionValue>& args);
    virtual bool hasMethod(const std::string& name) const;
    virtual std::vector<std::string> getMethodNames() const;
    
    // Serialization
    virtual std::string toString() const;
    virtual AlterionValue toValue() const;
    
    // Type information
    virtual std::string getClassName() const { return "AlterionObject"; }
};

// Alterion Function - represents callable functions
class AlterionFunction {
public:
    using NativeFunction = std::function<AlterionValue(const std::vector<AlterionValue>&)>;
    using AsyncFunction = std::function<std::future<AlterionValue>(const std::vector<AlterionValue>&)>;
    
private:
    std::string name;
    std::vector<std::string> parameters;
    NativeFunction nativeImpl;
    AsyncFunction asyncImpl;
    bool isAsync;
    std::shared_ptr<AlterionObject> boundObject;
    
public:
    // Constructors
    AlterionFunction(const std::string& name, const std::vector<std::string>& params, 
                    NativeFunction impl);
    AlterionFunction(const std::string& name, const std::vector<std::string>& params, 
                    AsyncFunction impl);
    
    // Function call
    AlterionValue call(const std::vector<AlterionValue>& args);
    std::future<AlterionValue> callAsync(const std::vector<AlterionValue>& args);
    
    // Binding
    std::shared_ptr<AlterionFunction> bind(std::shared_ptr<AlterionObject> obj);
    
    // Properties
    const std::string& getName() const { return name; }
    const std::vector<std::string>& getParameters() const { return parameters; }
    bool getIsAsync() const { return isAsync; }
    size_t getArity() const { return parameters.size(); }
    
    // String representation
    std::string toString() const;
};

// Alterion Component - represents UI components
class AlterionComponent : public AlterionObject {
private:
    std::string componentName;
    std::unordered_map<std::string, AlterionValue> state;
    std::unordered_map<std::string, AlterionValue> props;
    std::shared_ptr<AlterionFunction> renderFunction;
    std::vector<std::shared_ptr<AlterionComponent>> children;
    std::weak_ptr<AlterionComponent> parent;
    bool needsRerender;
    
public:
    AlterionComponent(const std::string& name);
    
    // State management
    AlterionValue getState(const std::string& key) const;
    void setState(const std::string& key, const AlterionValue& value);
    void setState(const std::unordered_map<std::string, AlterionValue>& newState);
    const std::unordered_map<std::string, AlterionValue>& getAllState() const { return state; }
    
    // Props management
    AlterionValue getProps(const std::string& key) const;
    void setProps(const std::unordered_map<std::string, AlterionValue>& newProps);
    const std::unordered_map<std::string, AlterionValue>& getAllProps() const { return props; }
    
    // Render function
    void setRenderFunction(std::shared_ptr<AlterionFunction> func);
    AlterionValue render();
    
    // Component tree
    void addChild(std::shared_ptr<AlterionComponent> child);
    void removeChild(std::shared_ptr<AlterionComponent> child);
    const std::vector<std::shared_ptr<AlterionComponent>>& getChildren() const { return children; }
    std::shared_ptr<AlterionComponent> getParent() const { return parent.lock(); }
    
    // Lifecycle
    virtual void componentDidMount() {}
    virtual void componentWillUnmount() {}
    virtual void componentDidUpdate() {}
    
    // Re-rendering
    void markForRerender() { needsRerender = true; }
    bool getNeedsRerender() const { return needsRerender; }
    void clearRerenderFlag() { needsRerender = false; }
    
    // Component information
    const std::string& getComponentName() const { return componentName; }
    std::string getClassName() const override { return "AlterionComponent"; }
    
    // Serialization
    std::string toString() const override;
    AlterionValue toValue() const override;
};

// JavaScript Interop Layer
class JSInterop {
private:
    std::unordered_map<std::string, AlterionValue> globalObjects;
    std::unordered_map<std::string, std::shared_ptr<AlterionFunction>> globalFunctions;
    
public:
    // Global object management
    void registerGlobalObject(const std::string& name, const AlterionValue& object);
    void registerGlobalFunction(const std::string& name, std::shared_ptr<AlterionFunction> function);
    AlterionValue getGlobal(const std::string& name) const;
    bool hasGlobal(const std::string& name) const;
    
    // JavaScript execution
    AlterionValue executeJS(const std::string& jsCode);
    std::future<AlterionValue> executeJSAsync(const std::string& jsCode);
    
    // Type conversion
    AlterionValue jsToAlterion(const std::string& jsValue, const std::string& jsType);
    std::string alterionToJS(const AlterionValue& value);
    
    // Promise handling
    std::future<AlterionValue> createPromise(std::function<AlterionValue()> executor);
    AlterionValue awaitPromise(std::future<AlterionValue>& future);
    
    // Event system
    void addEventListener(const std::string& event, std::shared_ptr<AlterionFunction> handler);
    void removeEventListener(const std::string& event, std::shared_ptr<AlterionFunction> handler);
    void dispatchEvent(const std::string& event, const AlterionValue& data);
    
    // DOM integration (for web targets)
    AlterionValue createElement(const std::string& tagName, 
                               const std::unordered_map<std::string, AlterionValue>& attributes);
    void appendChild(const AlterionValue& parent, const AlterionValue& child);
    void setAttribute(const AlterionValue& element, const std::string& attr, const AlterionValue& value);
    
    // Console integration
    void log(const std::vector<AlterionValue>& args);
    void warn(const std::vector<AlterionValue>& args);
    void error(const std::vector<AlterionValue>& args);
};

// Runtime Environment
class AlterionRuntime {
private:
    std::unique_ptr<JSInterop> jsInterop;
    std::unordered_map<std::string, AlterionValue> modules;
    std::unordered_map<std::string, std::shared_ptr<AlterionComponent>> components;
    std::vector<std::shared_ptr<AlterionComponent>> renderQueue;
    
public:
    AlterionRuntime();
    ~AlterionRuntime();
    
    // Runtime initialization
    void initialize();
    void shutdown();
    
    // Module system
    void registerModule(const std::string& name, const AlterionValue& moduleExports);
    AlterionValue importModule(const std::string& name);
    bool hasModule(const std::string& name) const;
    
    // Component registration
    void registerComponent(const std::string& name, std::shared_ptr<AlterionComponent> component);
    std::shared_ptr<AlterionComponent> createComponent(const std::string& name);
    
    // Rendering system
    void scheduleRender(std::shared_ptr<AlterionComponent> component);
    void processRenderQueue();
    void renderComponent(std::shared_ptr<AlterionComponent> component);
    
    // JavaScript interop
    JSInterop& getJSInterop() { return *jsInterop; }
    
    // Standard library integration
    void loadStandardLibrary();
    void loadDOMLibrary();      // For web targets
    void loadNodeLibrary();     // For Node.js targets
    void loadReactNativeLibrary(); // For React Native targets
    
    // Error handling
    void handleError(const std::exception& error);
    void setErrorHandler(std::function<void(const std::exception&)> handler);
    
    // Performance monitoring
    void startPerformanceTimer(const std::string& name);
    double endPerformanceTimer(const std::string& name);
    void logPerformanceMetrics();
};

// Runtime Exceptions
class AlterionRuntimeError : public std::exception {
private:
    std::string message;
    std::string location;
    
public:
    AlterionRuntimeError(const std::string& msg, const std::string& loc = "")
        : message(msg), location(loc) {}
    
    const char* what() const noexcept override { return message.c_str(); }
    const std::string& getLocation() const { return location; }
};

class AlterionTypeError : public AlterionRuntimeError {
public:
    AlterionTypeError(const std::string& expected, const std::string& actual, const std::string& loc = "")
        : AlterionRuntimeError("Type error: expected " + expected + ", got " + actual, loc) {}
};

class AlterionReferenceError : public AlterionRuntimeError {
public:
    AlterionReferenceError(const std::string& name, const std::string& loc = "")
        : AlterionRuntimeError("Reference error: " + name + " is not defined", loc) {}
};

// Global runtime instance
extern std::unique_ptr<AlterionRuntime> globalRuntime;

// Utility functions
AlterionValue createArray(const std::vector<AlterionValue>& elements);
AlterionValue createObject(const std::unordered_map<std::string, AlterionValue>& properties);
std::shared_ptr<AlterionFunction> createFunction(const std::string& name,
                                                const std::vector<std::string>& params,
                                                AlterionFunction::NativeFunction impl);
std::shared_ptr<AlterionComponent> createComponent(const std::string& name);

// Type conversion helpers
template<typename T>
T alterionCast(const AlterionValue& value);

template<>
bool alterionCast<bool>(const AlterionValue& value);

template<>
int alterionCast<int>(const AlterionValue& value);

template<>
double alterionCast<double>(const AlterionValue& value);

template<>
std::string alterionCast<std::string>(const AlterionValue& value);

} // namespace Alterion
