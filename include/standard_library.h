#pragma once

#include "runtime.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

namespace Alterion {

// Standard Library Modules
namespace StdLib {

// Core module - basic utilities and types
class CoreModule {
public:
    static AlterionValue createModule();
    
    // Type checking functions
    static AlterionValue typeof_impl(const std::vector<AlterionValue>& args);
    static AlterionValue instanceof_impl(const std::vector<AlterionValue>& args);
    
    // Conversion functions
    static AlterionValue toString_impl(const std::vector<AlterionValue>& args);
    static AlterionValue toNumber_impl(const std::vector<AlterionValue>& args);
    static AlterionValue toBoolean_impl(const std::vector<AlterionValue>& args);
    
    // Array utilities
    static AlterionValue isArray_impl(const std::vector<AlterionValue>& args);
    static AlterionValue arrayLength_impl(const std::vector<AlterionValue>& args);
};

// Array module - array manipulation functions
class ArrayModule {
public:
    static AlterionValue createModule();
    
    // Array methods
    static AlterionValue push_impl(const std::vector<AlterionValue>& args);
    static AlterionValue pop_impl(const std::vector<AlterionValue>& args);
    static AlterionValue shift_impl(const std::vector<AlterionValue>& args);
    static AlterionValue unshift_impl(const std::vector<AlterionValue>& args);
    static AlterionValue slice_impl(const std::vector<AlterionValue>& args);
    static AlterionValue splice_impl(const std::vector<AlterionValue>& args);
    static AlterionValue indexOf_impl(const std::vector<AlterionValue>& args);
    static AlterionValue includes_impl(const std::vector<AlterionValue>& args);
    static AlterionValue join_impl(const std::vector<AlterionValue>& args);
    static AlterionValue reverse_impl(const std::vector<AlterionValue>& args);
    static AlterionValue sort_impl(const std::vector<AlterionValue>& args);
    
    // Higher-order functions
    static AlterionValue map_impl(const std::vector<AlterionValue>& args);
    static AlterionValue filter_impl(const std::vector<AlterionValue>& args);
    static AlterionValue reduce_impl(const std::vector<AlterionValue>& args);
    static AlterionValue forEach_impl(const std::vector<AlterionValue>& args);
    static AlterionValue find_impl(const std::vector<AlterionValue>& args);
    static AlterionValue some_impl(const std::vector<AlterionValue>& args);
    static AlterionValue every_impl(const std::vector<AlterionValue>& args);
};

// String module - string manipulation functions
class StringModule {
public:
    static AlterionValue createModule();
    
    // String methods
    static AlterionValue charAt_impl(const std::vector<AlterionValue>& args);
    static AlterionValue charCodeAt_impl(const std::vector<AlterionValue>& args);
    static AlterionValue substring_impl(const std::vector<AlterionValue>& args);
    static AlterionValue substr_impl(const std::vector<AlterionValue>& args);
    static AlterionValue indexOf_impl(const std::vector<AlterionValue>& args);
    static AlterionValue lastIndexOf_impl(const std::vector<AlterionValue>& args);
    static AlterionValue toLowerCase_impl(const std::vector<AlterionValue>& args);
    static AlterionValue toUpperCase_impl(const std::vector<AlterionValue>& args);
    static AlterionValue trim_impl(const std::vector<AlterionValue>& args);
    static AlterionValue split_impl(const std::vector<AlterionValue>& args);
    static AlterionValue replace_impl(const std::vector<AlterionValue>& args);
    static AlterionValue startsWith_impl(const std::vector<AlterionValue>& args);
    static AlterionValue endsWith_impl(const std::vector<AlterionValue>& args);
    static AlterionValue includes_impl(const std::vector<AlterionValue>& args);
    static AlterionValue repeat_impl(const std::vector<AlterionValue>& args);
    static AlterionValue padStart_impl(const std::vector<AlterionValue>& args);
    static AlterionValue padEnd_impl(const std::vector<AlterionValue>& args);
};

// Math module - mathematical functions
class MathModule {
public:
    static AlterionValue createModule();
    
    // Basic math functions
    static AlterionValue abs_impl(const std::vector<AlterionValue>& args);
    static AlterionValue ceil_impl(const std::vector<AlterionValue>& args);
    static AlterionValue floor_impl(const std::vector<AlterionValue>& args);
    static AlterionValue round_impl(const std::vector<AlterionValue>& args);
    static AlterionValue max_impl(const std::vector<AlterionValue>& args);
    static AlterionValue min_impl(const std::vector<AlterionValue>& args);
    static AlterionValue pow_impl(const std::vector<AlterionValue>& args);
    static AlterionValue sqrt_impl(const std::vector<AlterionValue>& args);
    static AlterionValue random_impl(const std::vector<AlterionValue>& args);
    
    // Trigonometric functions
    static AlterionValue sin_impl(const std::vector<AlterionValue>& args);
    static AlterionValue cos_impl(const std::vector<AlterionValue>& args);
    static AlterionValue tan_impl(const std::vector<AlterionValue>& args);
    static AlterionValue asin_impl(const std::vector<AlterionValue>& args);
    static AlterionValue acos_impl(const std::vector<AlterionValue>& args);
    static AlterionValue atan_impl(const std::vector<AlterionValue>& args);
    static AlterionValue atan2_impl(const std::vector<AlterionValue>& args);
    
    // Logarithmic functions
    static AlterionValue log_impl(const std::vector<AlterionValue>& args);
    static AlterionValue log10_impl(const std::vector<AlterionValue>& args);
    static AlterionValue log2_impl(const std::vector<AlterionValue>& args);
    static AlterionValue exp_impl(const std::vector<AlterionValue>& args);
};

// Object module - object manipulation functions
class ObjectModule {
public:
    static AlterionValue createModule();
    
    // Object methods
    static AlterionValue keys_impl(const std::vector<AlterionValue>& args);
    static AlterionValue values_impl(const std::vector<AlterionValue>& args);
    static AlterionValue entries_impl(const std::vector<AlterionValue>& args);
    static AlterionValue hasOwnProperty_impl(const std::vector<AlterionValue>& args);
    static AlterionValue assign_impl(const std::vector<AlterionValue>& args);
    static AlterionValue freeze_impl(const std::vector<AlterionValue>& args);
    static AlterionValue seal_impl(const std::vector<AlterionValue>& args);
    static AlterionValue create_impl(const std::vector<AlterionValue>& args);
    static AlterionValue defineProperty_impl(const std::vector<AlterionValue>& args);
    static AlterionValue getOwnPropertyNames_impl(const std::vector<AlterionValue>& args);
};

// JSON module - JSON parsing and serialization
class JSONModule {
public:
    static AlterionValue createModule();
    
    // JSON methods
    static AlterionValue stringify_impl(const std::vector<AlterionValue>& args);
    static AlterionValue parse_impl(const std::vector<AlterionValue>& args);
};

// Date module - date and time functions
class DateModule {
public:
    static AlterionValue createModule();
    
    // Date constructors and methods
    static AlterionValue now_impl(const std::vector<AlterionValue>& args);
    static AlterionValue parse_impl(const std::vector<AlterionValue>& args);
    static AlterionValue UTC_impl(const std::vector<AlterionValue>& args);
    
    // Instance methods would be implemented as object methods
    static std::shared_ptr<AlterionObject> createDateObject(int64_t timestamp = 0);
};

// Promise module - asynchronous operations
class PromiseModule {
public:
    static AlterionValue createModule();
    
    // Promise methods
    static AlterionValue resolve_impl(const std::vector<AlterionValue>& args);
    static AlterionValue reject_impl(const std::vector<AlterionValue>& args);
    static AlterionValue all_impl(const std::vector<AlterionValue>& args);
    static AlterionValue race_impl(const std::vector<AlterionValue>& args);
    static AlterionValue allSettled_impl(const std::vector<AlterionValue>& args);
    
    // Promise constructor
    static std::shared_ptr<AlterionObject> createPromiseObject(
        std::function<void(std::function<void(AlterionValue)>, std::function<void(AlterionValue)>)> executor);
};

// Console module - console output functions
class ConsoleModule {
public:
    static AlterionValue createModule();
    
    // Console methods
    static AlterionValue log_impl(const std::vector<AlterionValue>& args);
    static AlterionValue warn_impl(const std::vector<AlterionValue>& args);
    static AlterionValue error_impl(const std::vector<AlterionValue>& args);
    static AlterionValue info_impl(const std::vector<AlterionValue>& args);
    static AlterionValue debug_impl(const std::vector<AlterionValue>& args);
    static AlterionValue trace_impl(const std::vector<AlterionValue>& args);
    static AlterionValue assert_impl(const std::vector<AlterionValue>& args);
    static AlterionValue time_impl(const std::vector<AlterionValue>& args);
    static AlterionValue timeEnd_impl(const std::vector<AlterionValue>& args);
    static AlterionValue table_impl(const std::vector<AlterionValue>& args);
    static AlterionValue group_impl(const std::vector<AlterionValue>& args);
    static AlterionValue groupEnd_impl(const std::vector<AlterionValue>& args);
    static AlterionValue clear_impl(const std::vector<AlterionValue>& args);
};

// Component module - UI component utilities
class ComponentModule {
public:
    static AlterionValue createModule();
    
    // Component utilities
    static AlterionValue createElement_impl(const std::vector<AlterionValue>& args);
    static AlterionValue createComponent_impl(const std::vector<AlterionValue>& args);
    static AlterionValue useState_impl(const std::vector<AlterionValue>& args);
    static AlterionValue useEffect_impl(const std::vector<AlterionValue>& args);
    static AlterionValue useCallback_impl(const std::vector<AlterionValue>& args);
    static AlterionValue useMemo_impl(const std::vector<AlterionValue>& args);
    static AlterionValue useRef_impl(const std::vector<AlterionValue>& args);
    static AlterionValue useContext_impl(const std::vector<AlterionValue>& args);
};

// DOM module - Document Object Model manipulation (for web targets)
class DOMModule {
public:
    static AlterionValue createModule();
    
    // DOM manipulation
    static AlterionValue getElementById_impl(const std::vector<AlterionValue>& args);
    static AlterionValue getElementsByClassName_impl(const std::vector<AlterionValue>& args);
    static AlterionValue getElementsByTagName_impl(const std::vector<AlterionValue>& args);
    static AlterionValue querySelector_impl(const std::vector<AlterionValue>& args);
    static AlterionValue querySelectorAll_impl(const std::vector<AlterionValue>& args);
    static AlterionValue createElement_impl(const std::vector<AlterionValue>& args);
    static AlterionValue createTextNode_impl(const std::vector<AlterionValue>& args);
    static AlterionValue appendChild_impl(const std::vector<AlterionValue>& args);
    static AlterionValue removeChild_impl(const std::vector<AlterionValue>& args);
    static AlterionValue setAttribute_impl(const std::vector<AlterionValue>& args);
    static AlterionValue getAttribute_impl(const std::vector<AlterionValue>& args);
    static AlterionValue addEventListener_impl(const std::vector<AlterionValue>& args);
    static AlterionValue removeEventListener_impl(const std::vector<AlterionValue>& args);
    
    // DOM object creation
    static std::shared_ptr<AlterionObject> createElementObject(const std::string& tagName);
    static std::shared_ptr<AlterionObject> createDocumentObject();
    static std::shared_ptr<AlterionObject> createWindowObject();
};

// HTTP module - HTTP requests (for web and Node.js targets)
class HTTPModule {
public:
    static AlterionValue createModule();
    
    // HTTP methods
    static AlterionValue fetch_impl(const std::vector<AlterionValue>& args);
    static AlterionValue get_impl(const std::vector<AlterionValue>& args);
    static AlterionValue post_impl(const std::vector<AlterionValue>& args);
    static AlterionValue put_impl(const std::vector<AlterionValue>& args);
    static AlterionValue delete_impl(const std::vector<AlterionValue>& args);
    
    // Response object creation
    static std::shared_ptr<AlterionObject> createResponseObject(int status, const std::string& body);
    static std::shared_ptr<AlterionObject> createRequestObject(const std::string& url, const std::string& method);
};

} // namespace StdLib

// Standard Library Manager
class StandardLibrary {
private:
    std::unordered_map<std::string, std::function<AlterionValue()>> moduleFactories;
    
public:
    StandardLibrary();
    
    // Module registration
    void registerModule(const std::string& name, std::function<AlterionValue()> factory);
    
    // Module loading
    AlterionValue loadModule(const std::string& name);
    bool hasModule(const std::string& name) const;
    std::vector<std::string> getAvailableModules() const;
    
    // Initialize all standard modules
    void initializeStandardModules();
    
    // Platform-specific module loading
    void loadWebModules();      // Browser-specific modules
    void loadNodeModules();     // Node.js-specific modules
    void loadMobileModules();   // React Native-specific modules
    void loadDesktopModules();  // Electron-specific modules
    
    // Module installation into runtime
    void installIntoRuntime(AlterionRuntime& runtime);
};

// Global standard library instance
extern std::unique_ptr<StandardLibrary> globalStdLib;

// Utility functions for standard library
AlterionValue createStdFunction(const std::string& name, 
                               const std::vector<std::string>& params,
                               std::function<AlterionValue(const std::vector<AlterionValue>&)> impl);

AlterionValue createStdObject(const std::unordered_map<std::string, AlterionValue>& properties);

// Error handling for standard library
class StdLibError : public AlterionRuntimeError {
public:
    StdLibError(const std::string& module, const std::string& function, const std::string& message)
        : AlterionRuntimeError("StandardLibrary." + module + "." + function + ": " + message) {}
};

} // namespace Alterion
