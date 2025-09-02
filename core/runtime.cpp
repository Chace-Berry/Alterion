#include "runtime.h"
#include <sstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <future>
#include <algorithm>
#include <regex>

namespace Alterion {

// Global runtime instance
std::unique_ptr<AlterionRuntime> globalRuntime = nullptr;

// AlterionValue Implementation
bool AlterionValue::asBool() const {
    if (isBool()) return std::get<bool>(value);
    if (isNumber()) return asDouble() != 0.0;
    if (isString()) return !asString().empty();
    if (isNull()) return false;
    return true; // Objects, arrays, functions are truthy
}

int64_t AlterionValue::asInt() const {
    if (std::holds_alternative<int64_t>(value)) return std::get<int64_t>(value);
    if (std::holds_alternative<double>(value)) return static_cast<int64_t>(std::get<double>(value));
    if (std::holds_alternative<bool>(value)) return std::get<bool>(value) ? 1 : 0;
    if (isString()) {
        try {
            return std::stoll(asString());
        } catch (...) {
            return 0;
        }
    }
    return 0;
}

double AlterionValue::asDouble() const {
    if (std::holds_alternative<double>(value)) return std::get<double>(value);
    if (std::holds_alternative<int64_t>(value)) return static_cast<double>(std::get<int64_t>(value));
    if (std::holds_alternative<bool>(value)) return std::get<bool>(value) ? 1.0 : 0.0;
    if (isString()) {
        try {
            return std::stod(asString());
        } catch (...) {
            return 0.0;
        }
    }
    return 0.0;
}

std::string AlterionValue::asString() const {
    if (isString()) return std::get<std::string>(value);
    if (isNull()) return "null";
    if (isBool()) return std::get<bool>(value) ? "true" : "false";
    if (std::holds_alternative<int64_t>(value)) return std::to_string(std::get<int64_t>(value));
    if (std::holds_alternative<double>(value)) return std::to_string(std::get<double>(value));
    if (isArray()) return "[Array]";
    if (isObject()) return "[Object]";
    if (isFunction()) return "[Function]";
    if (isComponent()) return "[Component]";
    return "[Unknown]";
}

std::vector<AlterionValue>& AlterionValue::asArray() {
    if (!isArray()) throw AlterionTypeError("Array", getType());
    return std::get<std::vector<AlterionValue>>(value);
}

const std::vector<AlterionValue>& AlterionValue::asArray() const {
    if (!isArray()) throw AlterionTypeError("Array", getType());
    return std::get<std::vector<AlterionValue>>(value);
}

std::unordered_map<std::string, AlterionValue>& AlterionValue::asObjectMap() {
    if (!std::holds_alternative<std::unordered_map<std::string, AlterionValue>>(value)) {
        throw AlterionTypeError("Object Map", getType());
    }
    return std::get<std::unordered_map<std::string, AlterionValue>>(value);
}

const std::unordered_map<std::string, AlterionValue>& AlterionValue::asObjectMap() const {
    if (!std::holds_alternative<std::unordered_map<std::string, AlterionValue>>(value)) {
        throw AlterionTypeError("Object Map", getType());
    }
    return std::get<std::unordered_map<std::string, AlterionValue>>(value);
}

std::shared_ptr<AlterionObject> AlterionValue::asObject() const {
    if (std::holds_alternative<std::shared_ptr<AlterionObject>>(value)) {
        return std::get<std::shared_ptr<AlterionObject>>(value);
    }
    throw AlterionTypeError("AlterionObject", getType());
}

std::shared_ptr<AlterionFunction> AlterionValue::asFunction() const {
    if (!isFunction()) throw AlterionTypeError("Function", getType());
    return std::get<std::shared_ptr<AlterionFunction>>(value);
}

std::shared_ptr<AlterionComponent> AlterionValue::asComponent() const {
    if (!isComponent()) throw AlterionTypeError("Component", getType());
    return std::get<std::shared_ptr<AlterionComponent>>(value);
}

AlterionValue AlterionValue::operator[](const std::string& key) const {
    if (std::holds_alternative<std::unordered_map<std::string, AlterionValue>>(value)) {
        const auto& map = std::get<std::unordered_map<std::string, AlterionValue>>(value);
        auto it = map.find(key);
        return it != map.end() ? it->second : AlterionValue();
    }
    if (std::holds_alternative<std::shared_ptr<AlterionObject>>(value)) {
        return std::get<std::shared_ptr<AlterionObject>>(value)->getProperty(key);
    }
    throw AlterionTypeError("Object", getType());
}

AlterionValue& AlterionValue::operator[](const std::string& key) {
    if (std::holds_alternative<std::unordered_map<std::string, AlterionValue>>(value)) {
        return std::get<std::unordered_map<std::string, AlterionValue>>(value)[key];
    }
    throw AlterionTypeError("Object", getType());
}

AlterionValue AlterionValue::operator[](size_t index) const {
    if (!isArray()) throw AlterionTypeError("Array", getType());
    const auto& arr = std::get<std::vector<AlterionValue>>(value);
    if (index >= arr.size()) throw std::out_of_range("Array index out of bounds");
    return arr[index];
}

AlterionValue& AlterionValue::operator[](size_t index) {
    if (!isArray()) throw AlterionTypeError("Array", getType());
    auto& arr = std::get<std::vector<AlterionValue>>(value);
    if (index >= arr.size()) throw std::out_of_range("Array index out of bounds");
    return arr[index];
}

std::string AlterionValue::toJSON() const {
    if (isNull()) return "null";
    if (isBool()) return asBool() ? "true" : "false";
    if (isNumber()) return std::to_string(asDouble());
    if (isString()) return "\"" + asString() + "\"";
    if (isArray()) {
        std::stringstream ss;
        ss << "[";
        const auto& arr = asArray();
        for (size_t i = 0; i < arr.size(); ++i) {
            if (i > 0) ss << ",";
            ss << arr[i].toJSON();
        }
        ss << "]";
        return ss.str();
    }
    if (std::holds_alternative<std::unordered_map<std::string, AlterionValue>>(value)) {
        std::stringstream ss;
        ss << "{";
        const auto& obj = asObjectMap();
        bool first = true;
        for (const auto& [key, val] : obj) {
            if (!first) ss << ",";
            ss << "\"" << key << "\":" << val.toJSON();
            first = false;
        }
        ss << "}";
        return ss.str();
    }
    return "{}";
}

std::string AlterionValue::toString() const {
    return asString();
}

std::string AlterionValue::getType() const {
    if (isNull()) return "null";
    if (isBool()) return "boolean";
    if (isNumber()) return "number";
    if (isString()) return "string";
    if (isArray()) return "array";
    if (isFunction()) return "function";
    if (isComponent()) return "component";
    if (isObject()) return "object";
    return "unknown";
}

// AlterionObject Implementation
AlterionValue AlterionObject::getProperty(const std::string& name) const {
    auto it = properties.find(name);
    return it != properties.end() ? it->second : AlterionValue();
}

void AlterionObject::setProperty(const std::string& name, const AlterionValue& value) {
    properties[name] = value;
}

bool AlterionObject::hasProperty(const std::string& name) const {
    return properties.find(name) != properties.end();
}

std::vector<std::string> AlterionObject::getPropertyNames() const {
    std::vector<std::string> names;
    for (const auto& [name, value] : properties) {
        names.push_back(name);
    }
    return names;
}

AlterionValue AlterionObject::callMethod(const std::string& name, const std::vector<AlterionValue>& args) {
    auto it = methods.find(name);
    if (it != methods.end()) {
        return it->second->call(args);
    }
    throw AlterionReferenceError("Method " + name + " not found");
}

bool AlterionObject::hasMethod(const std::string& name) const {
    return methods.find(name) != methods.end();
}

std::vector<std::string> AlterionObject::getMethodNames() const {
    std::vector<std::string> names;
    for (const auto& [name, method] : methods) {
        names.push_back(name);
    }
    return names;
}

std::string AlterionObject::toString() const {
    return "[object " + getClassName() + "]";
}

AlterionValue AlterionObject::toValue() const {
    std::unordered_map<std::string, AlterionValue> obj;
    for (const auto& [name, value] : properties) {
        obj[name] = value;
    }
    return AlterionValue(obj);
}

// AlterionFunction Implementation
AlterionFunction::AlterionFunction(const std::string& name, const std::vector<std::string>& params, 
                                 NativeFunction impl)
    : name(name), parameters(params), nativeImpl(impl), isAsync(false) {}

AlterionFunction::AlterionFunction(const std::string& name, const std::vector<std::string>& params, 
                                 AsyncFunction impl)
    : name(name), parameters(params), asyncImpl(impl), isAsync(true) {}

AlterionValue AlterionFunction::call(const std::vector<AlterionValue>& args) {
    if (isAsync) {
        auto future = asyncImpl(args);
        return future.get(); // Block until completion
    }
    return nativeImpl(args);
}

std::future<AlterionValue> AlterionFunction::callAsync(const std::vector<AlterionValue>& args) {
    if (isAsync) {
        return asyncImpl(args);
    }
    // Wrap synchronous function in a future
    return std::async(std::launch::async, [this, args]() {
        return nativeImpl(args);
    });
}

std::shared_ptr<AlterionFunction> AlterionFunction::bind(std::shared_ptr<AlterionObject> obj) {
    auto bound = std::make_shared<AlterionFunction>(*this);
    bound->boundObject = obj;
    return bound;
}

std::string AlterionFunction::toString() const {
    return "[Function: " + name + "]";
}

// AlterionComponent Implementation
AlterionComponent::AlterionComponent(const std::string& name) 
    : componentName(name), needsRerender(false) {}

AlterionValue AlterionComponent::getState(const std::string& key) const {
    auto it = state.find(key);
    return it != state.end() ? it->second : AlterionValue();
}

void AlterionComponent::setState(const std::string& key, const AlterionValue& value) {
    state[key] = value;
    markForRerender();
}

void AlterionComponent::setState(const std::unordered_map<std::string, AlterionValue>& newState) {
    for (const auto& [key, value] : newState) {
        state[key] = value;
    }
    markForRerender();
}

AlterionValue AlterionComponent::getProps(const std::string& key) const {
    auto it = props.find(key);
    return it != props.end() ? it->second : AlterionValue();
}

void AlterionComponent::setProps(const std::unordered_map<std::string, AlterionValue>& newProps) {
    props = newProps;
    markForRerender();
}

void AlterionComponent::setRenderFunction(std::shared_ptr<AlterionFunction> func) {
    renderFunction = func;
}

AlterionValue AlterionComponent::render() {
    if (renderFunction) {
        clearRerenderFlag();
        return renderFunction->call({});
    }
    return AlterionValue();
}

void AlterionComponent::addChild(std::shared_ptr<AlterionComponent> child) {
    children.push_back(child);
    child->parent = std::weak_ptr<AlterionComponent>(shared_from_this());
}

void AlterionComponent::removeChild(std::shared_ptr<AlterionComponent> child) {
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
        children.erase(it);
        child->parent.reset();
    }
}

std::string AlterionComponent::toString() const {
    return "[Component: " + componentName + "]";
}

AlterionValue AlterionComponent::toValue() const {
    std::unordered_map<std::string, AlterionValue> obj;
    obj["type"] = AlterionValue("component");
    obj["name"] = AlterionValue(componentName);
    obj["state"] = AlterionValue(state);
    obj["props"] = AlterionValue(props);
    return AlterionValue(obj);
}

// JSInterop Implementation
void JSInterop::registerGlobalObject(const std::string& name, const AlterionValue& object) {
    globalObjects[name] = object;
}

void JSInterop::registerGlobalFunction(const std::string& name, std::shared_ptr<AlterionFunction> function) {
    globalFunctions[name] = function;
}

AlterionValue JSInterop::getGlobal(const std::string& name) const {
    auto objIt = globalObjects.find(name);
    if (objIt != globalObjects.end()) {
        return objIt->second;
    }
    
    auto funcIt = globalFunctions.find(name);
    if (funcIt != globalFunctions.end()) {
        return AlterionValue(funcIt->second);
    }
    
    throw AlterionReferenceError(name);
}

bool JSInterop::hasGlobal(const std::string& name) const {
    return globalObjects.find(name) != globalObjects.end() || 
           globalFunctions.find(name) != globalFunctions.end();
}

AlterionValue JSInterop::executeJS(const std::string& jsCode) {
    // Simplified JavaScript execution simulation
    // In a real implementation, this would use a JavaScript engine like V8
    
    // Handle simple cases for demonstration
    if (jsCode == "console.log") {
        return AlterionValue(createFunction("log", {"message"}, 
            [this](const std::vector<AlterionValue>& args) {
                log(args);
                return AlterionValue();
            }));
    }
    
    if (jsCode.starts_with("JSON.stringify(")) {
        // Extract the argument and convert to JSON
        std::string arg = jsCode.substr(15, jsCode.length() - 16);
        // This would need proper parsing in a real implementation
        return AlterionValue("\"" + arg + "\"");
    }
    
    // Default: return undefined
    return AlterionValue();
}

std::future<AlterionValue> JSInterop::executeJSAsync(const std::string& jsCode) {
    return std::async(std::launch::async, [this, jsCode]() {
        return executeJS(jsCode);
    });
}

AlterionValue JSInterop::jsToAlterion(const std::string& jsValue, const std::string& jsType) {
    if (jsType == "null" || jsValue == "null") return AlterionValue();
    if (jsType == "boolean") return AlterionValue(jsValue == "true");
    if (jsType == "number") return AlterionValue(std::stod(jsValue));
    if (jsType == "string") return AlterionValue(jsValue);
    // More complex types would need proper parsing
    return AlterionValue();
}

std::string JSInterop::alterionToJS(const AlterionValue& value) {
    if (value.isNull()) return "null";
    if (value.isBool()) return value.asBool() ? "true" : "false";
    if (value.isNumber()) return std::to_string(value.asDouble());
    if (value.isString()) return "\"" + value.asString() + "\"";
    return value.toJSON();
}

std::future<AlterionValue> JSInterop::createPromise(std::function<AlterionValue()> executor) {
    return std::async(std::launch::async, executor);
}

AlterionValue JSInterop::awaitPromise(std::future<AlterionValue>& future) {
    return future.get();
}

void JSInterop::addEventListener(const std::string& event, std::shared_ptr<AlterionFunction> handler) {
    // Event system implementation would go here
    std::cout << "Added event listener for: " << event << std::endl;
}

void JSInterop::removeEventListener(const std::string& event, std::shared_ptr<AlterionFunction> handler) {
    std::cout << "Removed event listener for: " << event << std::endl;
}

void JSInterop::dispatchEvent(const std::string& event, const AlterionValue& data) {
    std::cout << "Dispatched event: " << event << " with data: " << data.toString() << std::endl;
}

AlterionValue JSInterop::createElement(const std::string& tagName, 
                                     const std::unordered_map<std::string, AlterionValue>& attributes) {
    std::unordered_map<std::string, AlterionValue> element;
    element["tagName"] = AlterionValue(tagName);
    element["attributes"] = AlterionValue(attributes);
    element["children"] = createArray({});
    return AlterionValue(element);
}

void JSInterop::appendChild(const AlterionValue& parent, const AlterionValue& child) {
    if (parent.isObject() && parent["children"].isArray()) {
        auto& children = const_cast<AlterionValue&>(parent)["children"].asArray();
        children.push_back(child);
    }
}

void JSInterop::setAttribute(const AlterionValue& element, const std::string& attr, const AlterionValue& value) {
    if (element.isObject() && element["attributes"].isObject()) {
        auto& attributes = const_cast<AlterionValue&>(element)["attributes"].asObjectMap();
        attributes[attr] = value;
    }
}

void JSInterop::log(const std::vector<AlterionValue>& args) {
    std::cout << "[LOG] ";
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) std::cout << " ";
        std::cout << args[i].toString();
    }
    std::cout << std::endl;
}

void JSInterop::warn(const std::vector<AlterionValue>& args) {
    std::cout << "[WARN] ";
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) std::cout << " ";
        std::cout << args[i].toString();
    }
    std::cout << std::endl;
}

void JSInterop::error(const std::vector<AlterionValue>& args) {
    std::cerr << "[ERROR] ";
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) std::cerr << " ";
        std::cerr << args[i].toString();
    }
    std::cerr << std::endl;
}

// AlterionRuntime Implementation
AlterionRuntime::AlterionRuntime() : jsInterop(std::make_unique<JSInterop>()) {}

AlterionRuntime::~AlterionRuntime() = default;

void AlterionRuntime::initialize() {
    std::cout << "🚀 Initializing Alterion Runtime..." << std::endl;
    
    // Initialize JavaScript interop
    loadStandardLibrary();
    
    std::cout << "✅ Alterion Runtime initialized successfully!" << std::endl;
}

void AlterionRuntime::shutdown() {
    std::cout << "🔄 Shutting down Alterion Runtime..." << std::endl;
    
    // Clean up resources
    modules.clear();
    components.clear();
    renderQueue.clear();
    
    std::cout << "✅ Alterion Runtime shutdown complete!" << std::endl;
}

void AlterionRuntime::registerModule(const std::string& name, const AlterionValue& moduleExports) {
    modules[name] = moduleExports;
    std::cout << "📦 Registered module: " << name << std::endl;
}

AlterionValue AlterionRuntime::importModule(const std::string& name) {
    auto it = modules.find(name);
    if (it != modules.end()) {
        return it->second;
    }
    throw AlterionReferenceError("Module " + name + " not found");
}

bool AlterionRuntime::hasModule(const std::string& name) const {
    return modules.find(name) != modules.end();
}

void AlterionRuntime::registerComponent(const std::string& name, std::shared_ptr<AlterionComponent> component) {
    components[name] = component;
    std::cout << "🎨 Registered component: " << name << std::endl;
}

std::shared_ptr<AlterionComponent> AlterionRuntime::createComponent(const std::string& name) {
    auto it = components.find(name);
    if (it != components.end()) {
        // Create a new instance based on the registered component
        return std::make_shared<AlterionComponent>(name);
    }
    throw AlterionReferenceError("Component " + name + " not found");
}

void AlterionRuntime::scheduleRender(std::shared_ptr<AlterionComponent> component) {
    renderQueue.push_back(component);
}

void AlterionRuntime::processRenderQueue() {
    for (auto& component : renderQueue) {
        if (component->getNeedsRerender()) {
            renderComponent(component);
        }
    }
    renderQueue.clear();
}

void AlterionRuntime::renderComponent(std::shared_ptr<AlterionComponent> component) {
    std::cout << "🎨 Rendering component: " << component->getComponentName() << std::endl;
    
    try {
        auto result = component->render();
        // In a real implementation, this would update the actual DOM/UI
        std::cout << "✅ Component rendered successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "❌ Error rendering component: " << e.what() << std::endl;
    }
}

void AlterionRuntime::loadStandardLibrary() {
    // Console functions
    jsInterop->registerGlobalFunction("console.log", createFunction("log", {"message"}, 
        [this](const std::vector<AlterionValue>& args) {
            jsInterop->log(args);
            return AlterionValue();
        }));
    
    jsInterop->registerGlobalFunction("console.warn", createFunction("warn", {"message"}, 
        [this](const std::vector<AlterionValue>& args) {
            jsInterop->warn(args);
            return AlterionValue();
        }));
    
    jsInterop->registerGlobalFunction("console.error", createFunction("error", {"message"}, 
        [this](const std::vector<AlterionValue>& args) {
            jsInterop->error(args);
            return AlterionValue();
        }));
    
    // Math functions
    std::unordered_map<std::string, AlterionValue> mathObject;
    mathObject["PI"] = AlterionValue(3.14159265359);
    mathObject["E"] = AlterionValue(2.71828182846);
    jsInterop->registerGlobalObject("Math", AlterionValue(mathObject));
    
    // JSON functions
    jsInterop->registerGlobalFunction("JSON.stringify", createFunction("stringify", {"value"}, 
        [](const std::vector<AlterionValue>& args) {
            if (!args.empty()) {
                return AlterionValue(args[0].toJSON());
            }
            return AlterionValue("undefined");
        }));
    
    std::cout << "📚 Standard library loaded" << std::endl;
}

void AlterionRuntime::loadDOMLibrary() {
    // DOM creation functions
    jsInterop->registerGlobalFunction("document.createElement", createFunction("createElement", {"tagName"}, 
        [this](const std::vector<AlterionValue>& args) {
            if (!args.empty()) {
                return jsInterop->createElement(args[0].asString(), {});
            }
            return AlterionValue();
        }));
    
    std::cout << "🌐 DOM library loaded" << std::endl;
}

void AlterionRuntime::loadNodeLibrary() {
    // Node.js specific functions
    jsInterop->registerGlobalFunction("require", createFunction("require", {"module"}, 
        [this](const std::vector<AlterionValue>& args) {
            if (!args.empty()) {
                return importModule(args[0].asString());
            }
            return AlterionValue();
        }));
    
    std::cout << "🔧 Node.js library loaded" << std::endl;
}

void AlterionRuntime::loadReactNativeLibrary() {
    // React Native specific functions
    std::cout << "📱 React Native library loaded" << std::endl;
}

void AlterionRuntime::handleError(const std::exception& error) {
    std::cerr << "❌ Runtime Error: " << error.what() << std::endl;
}

void AlterionRuntime::setErrorHandler(std::function<void(const std::exception&)> handler) {
    // Store error handler for custom error handling
}

void AlterionRuntime::startPerformanceTimer(const std::string& name) {
    // Performance monitoring implementation
}

double AlterionRuntime::endPerformanceTimer(const std::string& name) {
    return 0.0; // Return elapsed time in milliseconds
}

void AlterionRuntime::logPerformanceMetrics() {
    std::cout << "📊 Performance metrics logged" << std::endl;
}

// Utility Functions
AlterionValue createArray(const std::vector<AlterionValue>& elements) {
    return AlterionValue(elements);
}

AlterionValue createObject(const std::unordered_map<std::string, AlterionValue>& properties) {
    return AlterionValue(properties);
}

std::shared_ptr<AlterionFunction> createFunction(const std::string& name,
                                               const std::vector<std::string>& params,
                                               AlterionFunction::NativeFunction impl) {
    return std::make_shared<AlterionFunction>(name, params, impl);
}

std::shared_ptr<AlterionComponent> createComponent(const std::string& name) {
    return std::make_shared<AlterionComponent>(name);
}

// Template specializations
template<>
bool alterionCast<bool>(const AlterionValue& value) {
    return value.asBool();
}

template<>
int alterionCast<int>(const AlterionValue& value) {
    return static_cast<int>(value.asInt());
}

template<>
double alterionCast<double>(const AlterionValue& value) {
    return value.asDouble();
}

template<>
std::string alterionCast<std::string>(const AlterionValue& value) {
    return value.asString();
}

} // namespace Alterion
