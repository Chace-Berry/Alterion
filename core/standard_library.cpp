#include "standard_library.h"
#include <algorithm>
#include <random>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace Alterion {

// Global standard library instance
std::unique_ptr<StandardLibrary> globalStdLib = nullptr;

// CoreModule Implementation
AlterionValue StdLib::CoreModule::createModule() {
    std::unordered_map<std::string, AlterionValue> module;
    
    module["typeof"] = AlterionValue(createFunction("typeof", {"value"}, typeof_impl));
    module["instanceof"] = AlterionValue(createFunction("instanceof", {"object", "constructor"}, instanceof_impl));
    module["toString"] = AlterionValue(createFunction("toString", {"value"}, toString_impl));
    module["toNumber"] = AlterionValue(createFunction("toNumber", {"value"}, toNumber_impl));
    module["toBoolean"] = AlterionValue(createFunction("toBoolean", {"value"}, toBoolean_impl));
    module["isArray"] = AlterionValue(createFunction("isArray", {"value"}, isArray_impl));
    module["arrayLength"] = AlterionValue(createFunction("arrayLength", {"array"}, arrayLength_impl));
    
    return AlterionValue(module);
}

AlterionValue StdLib::CoreModule::typeof_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue("undefined");
    return AlterionValue(args[0].getType());
}

AlterionValue StdLib::CoreModule::instanceof_impl(const std::vector<AlterionValue>& args) {
    if (args.size() < 2) return AlterionValue(false);
    // Simplified instanceof implementation
    return AlterionValue(false);
}

AlterionValue StdLib::CoreModule::toString_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue("undefined");
    return AlterionValue(args[0].toString());
}

AlterionValue StdLib::CoreModule::toNumber_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue(0.0);
    return AlterionValue(args[0].asDouble());
}

AlterionValue StdLib::CoreModule::toBoolean_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue(false);
    return AlterionValue(args[0].asBool());
}

AlterionValue StdLib::CoreModule::isArray_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue(false);
    return AlterionValue(args[0].isArray());
}

AlterionValue StdLib::CoreModule::arrayLength_impl(const std::vector<AlterionValue>& args) {
    if (args.empty() || !args[0].isArray()) return AlterionValue(0);
    return AlterionValue(static_cast<int64_t>(args[0].asArray().size()));
}

// ArrayModule Implementation
AlterionValue StdLib::ArrayModule::createModule() {
    std::unordered_map<std::string, AlterionValue> module;
    
    module["push"] = AlterionValue(createFunction("push", {"array", "element"}, push_impl));
    module["pop"] = AlterionValue(createFunction("pop", {"array"}, pop_impl));
    module["shift"] = AlterionValue(createFunction("shift", {"array"}, shift_impl));
    module["unshift"] = AlterionValue(createFunction("unshift", {"array", "element"}, unshift_impl));
    module["slice"] = AlterionValue(createFunction("slice", {"array", "start", "end"}, slice_impl));
    module["indexOf"] = AlterionValue(createFunction("indexOf", {"array", "element"}, indexOf_impl));
    module["includes"] = AlterionValue(createFunction("includes", {"array", "element"}, includes_impl));
    module["join"] = AlterionValue(createFunction("join", {"array", "separator"}, join_impl));
    module["reverse"] = AlterionValue(createFunction("reverse", {"array"}, reverse_impl));
    module["sort"] = AlterionValue(createFunction("sort", {"array", "compareFn"}, sort_impl));
    module["map"] = AlterionValue(createFunction("map", {"array", "callback"}, map_impl));
    module["filter"] = AlterionValue(createFunction("filter", {"array", "callback"}, filter_impl));
    module["reduce"] = AlterionValue(createFunction("reduce", {"array", "callback", "initialValue"}, reduce_impl));
    module["forEach"] = AlterionValue(createFunction("forEach", {"array", "callback"}, forEach_impl));
    module["find"] = AlterionValue(createFunction("find", {"array", "callback"}, find_impl));
    module["some"] = AlterionValue(createFunction("some", {"array", "callback"}, some_impl));
    module["every"] = AlterionValue(createFunction("every", {"array", "callback"}, every_impl));
    
    return AlterionValue(module);
}

AlterionValue StdLib::ArrayModule::push_impl(const std::vector<AlterionValue>& args) {
    if (args.size() < 2 || !args[0].isArray()) return AlterionValue(0);
    
    auto& arr = const_cast<AlterionValue&>(args[0]).asArray();
    arr.push_back(args[1]);
    return AlterionValue(static_cast<int64_t>(arr.size()));
}

AlterionValue StdLib::ArrayModule::pop_impl(const std::vector<AlterionValue>& args) {
    if (args.empty() || !args[0].isArray()) return AlterionValue();
    
    auto& arr = const_cast<AlterionValue&>(args[0]).asArray();
    if (arr.empty()) return AlterionValue();
    
    AlterionValue result = arr.back();
    arr.pop_back();
    return result;
}

AlterionValue StdLib::ArrayModule::join_impl(const std::vector<AlterionValue>& args) {
    if (args.empty() || !args[0].isArray()) return AlterionValue("");
    
    const auto& arr = args[0].asArray();
    std::string separator = args.size() > 1 ? args[1].asString() : ",";
    
    std::string result;
    for (size_t i = 0; i < arr.size(); ++i) {
        if (i > 0) result += separator;
        result += arr[i].toString();
    }
    
    return AlterionValue(result);
}

AlterionValue StdLib::ArrayModule::indexOf_impl(const std::vector<AlterionValue>& args) {
    if (args.size() < 2 || !args[0].isArray()) return AlterionValue(-1);
    
    const auto& arr = args[0].asArray();
    for (size_t i = 0; i < arr.size(); ++i) {
        if (arr[i].toString() == args[1].toString()) {
            return AlterionValue(static_cast<int64_t>(i));
        }
    }
    
    return AlterionValue(-1);
}

AlterionValue StdLib::ArrayModule::includes_impl(const std::vector<AlterionValue>& args) {
    if (args.size() < 2 || !args[0].isArray()) return AlterionValue(false);
    
    const auto& arr = args[0].asArray();
    for (const auto& elem : arr) {
        if (elem.toString() == args[1].toString()) {
            return AlterionValue(true);
        }
    }
    
    return AlterionValue(false);
}

// Placeholder implementations for other array methods
AlterionValue StdLib::ArrayModule::shift_impl(const std::vector<AlterionValue>& args) {
    return AlterionValue(); // Implementation would remove first element
}

AlterionValue StdLib::ArrayModule::unshift_impl(const std::vector<AlterionValue>& args) {
    return AlterionValue(0); // Implementation would add to beginning
}

AlterionValue StdLib::ArrayModule::slice_impl(const std::vector<AlterionValue>& args) {
    return createArray({}); // Implementation would slice array
}

AlterionValue StdLib::ArrayModule::splice_impl(const std::vector<AlterionValue>& args) {
    return createArray({}); // Implementation would splice array
}

AlterionValue StdLib::ArrayModule::reverse_impl(const std::vector<AlterionValue>& args) {
    return AlterionValue(); // Implementation would reverse array
}

AlterionValue StdLib::ArrayModule::sort_impl(const std::vector<AlterionValue>& args) {
    return AlterionValue(); // Implementation would sort array
}

AlterionValue StdLib::ArrayModule::map_impl(const std::vector<AlterionValue>& args) {
    return createArray({}); // Implementation would map array
}

AlterionValue StdLib::ArrayModule::filter_impl(const std::vector<AlterionValue>& args) {
    return createArray({}); // Implementation would filter array
}

AlterionValue StdLib::ArrayModule::reduce_impl(const std::vector<AlterionValue>& args) {
    return AlterionValue(); // Implementation would reduce array
}

AlterionValue StdLib::ArrayModule::forEach_impl(const std::vector<AlterionValue>& args) {
    return AlterionValue(); // Implementation would iterate array
}

AlterionValue StdLib::ArrayModule::find_impl(const std::vector<AlterionValue>& args) {
    return AlterionValue(); // Implementation would find element
}

AlterionValue StdLib::ArrayModule::some_impl(const std::vector<AlterionValue>& args) {
    return AlterionValue(false); // Implementation would test some elements
}

AlterionValue StdLib::ArrayModule::every_impl(const std::vector<AlterionValue>& args) {
    return AlterionValue(false); // Implementation would test every element
}

// StringModule Implementation
AlterionValue StdLib::StringModule::createModule() {
    std::unordered_map<std::string, AlterionValue> module;
    
    module["charAt"] = AlterionValue(createFunction("charAt", {"string", "index"}, charAt_impl));
    module["substring"] = AlterionValue(createFunction("substring", {"string", "start", "end"}, substring_impl));
    module["indexOf"] = AlterionValue(createFunction("indexOf", {"string", "searchString"}, indexOf_impl));
    module["toLowerCase"] = AlterionValue(createFunction("toLowerCase", {"string"}, toLowerCase_impl));
    module["toUpperCase"] = AlterionValue(createFunction("toUpperCase", {"string"}, toUpperCase_impl));
    module["trim"] = AlterionValue(createFunction("trim", {"string"}, trim_impl));
    module["split"] = AlterionValue(createFunction("split", {"string", "separator"}, split_impl));
    module["replace"] = AlterionValue(createFunction("replace", {"string", "search", "replace"}, replace_impl));
    module["startsWith"] = AlterionValue(createFunction("startsWith", {"string", "searchString"}, startsWith_impl));
    module["endsWith"] = AlterionValue(createFunction("endsWith", {"string", "searchString"}, endsWith_impl));
    module["includes"] = AlterionValue(createFunction("includes", {"string", "searchString"}, includes_impl));
    
    return AlterionValue(module);
}

AlterionValue StdLib::StringModule::charAt_impl(const std::vector<AlterionValue>& args) {
    if (args.size() < 2) return AlterionValue("");
    
    std::string str = args[0].asString();
    int64_t index = args[1].asInt();
    
    if (index < 0 || index >= static_cast<int64_t>(str.length())) {
        return AlterionValue("");
    }
    
    return AlterionValue(std::string(1, str[index]));
}

AlterionValue StdLib::StringModule::substring_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue("");
    
    std::string str = args[0].asString();
    int64_t start = args.size() > 1 ? args[1].asInt() : 0;
    int64_t end = args.size() > 2 ? args[2].asInt() : static_cast<int64_t>(str.length());
    
    start = std::max(0LL, std::min(start, static_cast<int64_t>(str.length())));
    end = std::max(0LL, std::min(end, static_cast<int64_t>(str.length())));
    
    if (start > end) std::swap(start, end);
    
    return AlterionValue(str.substr(start, end - start));
}

AlterionValue StdLib::StringModule::indexOf_impl(const std::vector<AlterionValue>& args) {
    if (args.size() < 2) return AlterionValue(-1);
    
    std::string str = args[0].asString();
    std::string search = args[1].asString();
    
    size_t pos = str.find(search);
    return AlterionValue(pos != std::string::npos ? static_cast<int64_t>(pos) : -1);
}

AlterionValue StdLib::StringModule::toLowerCase_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue("");
    
    std::string str = args[0].asString();
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return AlterionValue(str);
}

AlterionValue StdLib::StringModule::toUpperCase_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue("");
    
    std::string str = args[0].asString();
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return AlterionValue(str);
}

AlterionValue StdLib::StringModule::trim_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue("");
    
    std::string str = args[0].asString();
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch) {
        return !std::isspace(ch);
    }));
    str.erase(std::find_if(str.rbegin(), str.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), str.end());
    
    return AlterionValue(str);
}

AlterionValue StdLib::StringModule::split_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return createArray({});
    
    std::string str = args[0].asString();
    std::string delimiter = args.size() > 1 ? args[1].asString() : "";
    
    std::vector<AlterionValue> result;
    if (delimiter.empty()) {
        for (char c : str) {
            result.push_back(AlterionValue(std::string(1, c)));
        }
    } else {
        size_t pos = 0;
        size_t found = 0;
        while ((found = str.find(delimiter, pos)) != std::string::npos) {
            result.push_back(AlterionValue(str.substr(pos, found - pos)));
            pos = found + delimiter.length();
        }
        result.push_back(AlterionValue(str.substr(pos)));
    }
    
    return createArray(result);
}

AlterionValue StdLib::StringModule::startsWith_impl(const std::vector<AlterionValue>& args) {
    if (args.size() < 2) return AlterionValue(false);
    
    std::string str = args[0].asString();
    std::string search = args[1].asString();
    
    return AlterionValue(str.find(search) == 0);
}

AlterionValue StdLib::StringModule::endsWith_impl(const std::vector<AlterionValue>& args) {
    if (args.size() < 2) return AlterionValue(false);
    
    std::string str = args[0].asString();
    std::string search = args[1].asString();
    
    if (search.length() > str.length()) return AlterionValue(false);
    
    return AlterionValue(str.compare(str.length() - search.length(), search.length(), search) == 0);
}

AlterionValue StdLib::StringModule::includes_impl(const std::vector<AlterionValue>& args) {
    if (args.size() < 2) return AlterionValue(false);
    
    std::string str = args[0].asString();
    std::string search = args[1].asString();
    
    return AlterionValue(str.find(search) != std::string::npos);
}

// Placeholder implementations for other string methods
AlterionValue StdLib::StringModule::charCodeAt_impl(const std::vector<AlterionValue>& args) {
    return AlterionValue(0); // Implementation would return char code
}

AlterionValue StdLib::StringModule::substr_impl(const std::vector<AlterionValue>& args) {
    return AlterionValue(""); // Implementation would return substring
}

AlterionValue StdLib::StringModule::lastIndexOf_impl(const std::vector<AlterionValue>& args) {
    return AlterionValue(-1); // Implementation would find last index
}

AlterionValue StdLib::StringModule::replace_impl(const std::vector<AlterionValue>& args) {
    return AlterionValue(""); // Implementation would replace text
}

AlterionValue StdLib::StringModule::repeat_impl(const std::vector<AlterionValue>& args) {
    return AlterionValue(""); // Implementation would repeat string
}

AlterionValue StdLib::StringModule::padStart_impl(const std::vector<AlterionValue>& args) {
    return AlterionValue(""); // Implementation would pad start
}

AlterionValue StdLib::StringModule::padEnd_impl(const std::vector<AlterionValue>& args) {
    return AlterionValue(""); // Implementation would pad end
}

// MathModule Implementation
AlterionValue StdLib::MathModule::createModule() {
    std::unordered_map<std::string, AlterionValue> module;
    
    // Constants
    module["PI"] = AlterionValue(M_PI);
    module["E"] = AlterionValue(M_E);
    module["LN2"] = AlterionValue(M_LN2);
    module["LN10"] = AlterionValue(M_LN10);
    module["LOG2E"] = AlterionValue(M_LOG2E);
    module["LOG10E"] = AlterionValue(M_LOG10E);
    module["SQRT1_2"] = AlterionValue(M_SQRT1_2);
    module["SQRT2"] = AlterionValue(M_SQRT2);
    
    // Functions
    module["abs"] = AlterionValue(createFunction("abs", {"x"}, abs_impl));
    module["ceil"] = AlterionValue(createFunction("ceil", {"x"}, ceil_impl));
    module["floor"] = AlterionValue(createFunction("floor", {"x"}, floor_impl));
    module["round"] = AlterionValue(createFunction("round", {"x"}, round_impl));
    module["max"] = AlterionValue(createFunction("max", {"a", "b"}, max_impl));
    module["min"] = AlterionValue(createFunction("min", {"a", "b"}, min_impl));
    module["pow"] = AlterionValue(createFunction("pow", {"base", "exponent"}, pow_impl));
    module["sqrt"] = AlterionValue(createFunction("sqrt", {"x"}, sqrt_impl));
    module["random"] = AlterionValue(createFunction("random", {}, random_impl));
    module["sin"] = AlterionValue(createFunction("sin", {"x"}, sin_impl));
    module["cos"] = AlterionValue(createFunction("cos", {"x"}, cos_impl));
    module["tan"] = AlterionValue(createFunction("tan", {"x"}, tan_impl));
    module["log"] = AlterionValue(createFunction("log", {"x"}, log_impl));
    module["exp"] = AlterionValue(createFunction("exp", {"x"}, exp_impl));
    
    return AlterionValue(module);
}

AlterionValue StdLib::MathModule::abs_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue(0.0);
    return AlterionValue(std::abs(args[0].asDouble()));
}

AlterionValue StdLib::MathModule::ceil_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue(0.0);
    return AlterionValue(std::ceil(args[0].asDouble()));
}

AlterionValue StdLib::MathModule::floor_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue(0.0);
    return AlterionValue(std::floor(args[0].asDouble()));
}

AlterionValue StdLib::MathModule::round_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue(0.0);
    return AlterionValue(std::round(args[0].asDouble()));
}

AlterionValue StdLib::MathModule::max_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue(-INFINITY);
    if (args.size() == 1) return AlterionValue(args[0].asDouble());
    
    double max_val = args[0].asDouble();
    for (size_t i = 1; i < args.size(); ++i) {
        max_val = std::max(max_val, args[i].asDouble());
    }
    return AlterionValue(max_val);
}

AlterionValue StdLib::MathModule::min_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue(INFINITY);
    if (args.size() == 1) return AlterionValue(args[0].asDouble());
    
    double min_val = args[0].asDouble();
    for (size_t i = 1; i < args.size(); ++i) {
        min_val = std::min(min_val, args[i].asDouble());
    }
    return AlterionValue(min_val);
}

AlterionValue StdLib::MathModule::pow_impl(const std::vector<AlterionValue>& args) {
    if (args.size() < 2) return AlterionValue(0.0);
    return AlterionValue(std::pow(args[0].asDouble(), args[1].asDouble()));
}

AlterionValue StdLib::MathModule::sqrt_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue(0.0);
    return AlterionValue(std::sqrt(args[0].asDouble()));
}

AlterionValue StdLib::MathModule::random_impl(const std::vector<AlterionValue>& args) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(0.0, 1.0);
    return AlterionValue(dis(gen));
}

AlterionValue StdLib::MathModule::sin_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue(0.0);
    return AlterionValue(std::sin(args[0].asDouble()));
}

AlterionValue StdLib::MathModule::cos_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue(1.0);
    return AlterionValue(std::cos(args[0].asDouble()));
}

AlterionValue StdLib::MathModule::tan_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue(0.0);
    return AlterionValue(std::tan(args[0].asDouble()));
}

AlterionValue StdLib::MathModule::log_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue(0.0);
    return AlterionValue(std::log(args[0].asDouble()));
}

AlterionValue StdLib::MathModule::exp_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue(1.0);
    return AlterionValue(std::exp(args[0].asDouble()));
}

// Placeholder implementations for other math functions
AlterionValue StdLib::MathModule::asin_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue(0.0);
    return AlterionValue(std::asin(args[0].asDouble()));
}

AlterionValue StdLib::MathModule::acos_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue(0.0);
    return AlterionValue(std::acos(args[0].asDouble()));
}

AlterionValue StdLib::MathModule::atan_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue(0.0);
    return AlterionValue(std::atan(args[0].asDouble()));
}

AlterionValue StdLib::MathModule::atan2_impl(const std::vector<AlterionValue>& args) {
    if (args.size() < 2) return AlterionValue(0.0);
    return AlterionValue(std::atan2(args[0].asDouble(), args[1].asDouble()));
}

AlterionValue StdLib::MathModule::log10_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue(0.0);
    return AlterionValue(std::log10(args[0].asDouble()));
}

AlterionValue StdLib::MathModule::log2_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue(0.0);
    return AlterionValue(std::log2(args[0].asDouble()));
}

// ConsoleModule Implementation
AlterionValue StdLib::ConsoleModule::createModule() {
    std::unordered_map<std::string, AlterionValue> module;
    
    module["log"] = AlterionValue(createFunction("log", {"message"}, log_impl));
    module["warn"] = AlterionValue(createFunction("warn", {"message"}, warn_impl));
    module["error"] = AlterionValue(createFunction("error", {"message"}, error_impl));
    module["info"] = AlterionValue(createFunction("info", {"message"}, info_impl));
    module["debug"] = AlterionValue(createFunction("debug", {"message"}, debug_impl));
    module["trace"] = AlterionValue(createFunction("trace", {}, trace_impl));
    module["assert"] = AlterionValue(createFunction("assert", {"condition", "message"}, assert_impl));
    module["time"] = AlterionValue(createFunction("time", {"label"}, time_impl));
    module["timeEnd"] = AlterionValue(createFunction("timeEnd", {"label"}, timeEnd_impl));
    module["clear"] = AlterionValue(createFunction("clear", {}, clear_impl));
    
    return AlterionValue(module);
}

AlterionValue StdLib::ConsoleModule::log_impl(const std::vector<AlterionValue>& args) {
    std::cout << "[LOG] ";
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) std::cout << " ";
        std::cout << args[i].toString();
    }
    std::cout << std::endl;
    return AlterionValue();
}

AlterionValue StdLib::ConsoleModule::warn_impl(const std::vector<AlterionValue>& args) {
    std::cout << "[WARN] ";
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) std::cout << " ";
        std::cout << args[i].toString();
    }
    std::cout << std::endl;
    return AlterionValue();
}

AlterionValue StdLib::ConsoleModule::error_impl(const std::vector<AlterionValue>& args) {
    std::cerr << "[ERROR] ";
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) std::cerr << " ";
        std::cerr << args[i].toString();
    }
    std::cerr << std::endl;
    return AlterionValue();
}

AlterionValue StdLib::ConsoleModule::info_impl(const std::vector<AlterionValue>& args) {
    std::cout << "[INFO] ";
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) std::cout << " ";
        std::cout << args[i].toString();
    }
    std::cout << std::endl;
    return AlterionValue();
}

AlterionValue StdLib::ConsoleModule::debug_impl(const std::vector<AlterionValue>& args) {
    std::cout << "[DEBUG] ";
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) std::cout << " ";
        std::cout << args[i].toString();
    }
    std::cout << std::endl;
    return AlterionValue();
}

// Placeholder implementations for other console methods
AlterionValue StdLib::ConsoleModule::trace_impl(const std::vector<AlterionValue>& args) {
    std::cout << "[TRACE] Stack trace would be here" << std::endl;
    return AlterionValue();
}

AlterionValue StdLib::ConsoleModule::assert_impl(const std::vector<AlterionValue>& args) {
    if (!args.empty() && !args[0].asBool()) {
        std::string message = args.size() > 1 ? args[1].asString() : "Assertion failed";
        std::cerr << "[ASSERT] " << message << std::endl;
    }
    return AlterionValue();
}

AlterionValue StdLib::ConsoleModule::time_impl(const std::vector<AlterionValue>& args) {
    std::string label = args.empty() ? "default" : args[0].asString();
    std::cout << "[TIME] Started timer: " << label << std::endl;
    return AlterionValue();
}

AlterionValue StdLib::ConsoleModule::timeEnd_impl(const std::vector<AlterionValue>& args) {
    std::string label = args.empty() ? "default" : args[0].asString();
    std::cout << "[TIME] Timer ended: " << label << std::endl;
    return AlterionValue();
}

AlterionValue StdLib::ConsoleModule::table_impl(const std::vector<AlterionValue>& args) {
    std::cout << "[TABLE] Table would be displayed here" << std::endl;
    return AlterionValue();
}

AlterionValue StdLib::ConsoleModule::group_impl(const std::vector<AlterionValue>& args) {
    std::cout << "[GROUP] Starting group" << std::endl;
    return AlterionValue();
}

AlterionValue StdLib::ConsoleModule::groupEnd_impl(const std::vector<AlterionValue>& args) {
    std::cout << "[GROUP] Ending group" << std::endl;
    return AlterionValue();
}

AlterionValue StdLib::ConsoleModule::clear_impl(const std::vector<AlterionValue>& args) {
    std::cout << "\033[2J\033[H"; // ANSI escape codes to clear screen
    return AlterionValue();
}

// StandardLibrary Implementation
StandardLibrary::StandardLibrary() {
    initializeStandardModules();
}

void StandardLibrary::registerModule(const std::string& name, std::function<AlterionValue()> factory) {
    moduleFactories[name] = factory;
}

AlterionValue StandardLibrary::loadModule(const std::string& name) {
    auto it = moduleFactories.find(name);
    if (it != moduleFactories.end()) {
        return it->second();
    }
    throw StdLibError("StandardLibrary", "loadModule", "Module '" + name + "' not found");
}

bool StandardLibrary::hasModule(const std::string& name) const {
    return moduleFactories.find(name) != moduleFactories.end();
}

std::vector<std::string> StandardLibrary::getAvailableModules() const {
    std::vector<std::string> modules;
    for (const auto& [name, factory] : moduleFactories) {
        modules.push_back(name);
    }
    return modules;
}

void StandardLibrary::initializeStandardModules() {
    registerModule("core", StdLib::CoreModule::createModule);
    registerModule("array", StdLib::ArrayModule::createModule);
    registerModule("string", StdLib::StringModule::createModule);
    registerModule("math", StdLib::MathModule::createModule);
    registerModule("console", StdLib::ConsoleModule::createModule);
    
    std::cout << "📚 Standard library modules initialized" << std::endl;
}

void StandardLibrary::loadWebModules() {
    // Web-specific modules would be registered here
    std::cout << "🌐 Web modules loaded" << std::endl;
}

void StandardLibrary::loadNodeModules() {
    // Node.js-specific modules would be registered here
    std::cout << "🔧 Node.js modules loaded" << std::endl;
}

void StandardLibrary::loadMobileModules() {
    // React Native-specific modules would be registered here
    std::cout << "📱 Mobile modules loaded" << std::endl;
}

void StandardLibrary::loadDesktopModules() {
    // Electron-specific modules would be registered here
    std::cout << "🖥️ Desktop modules loaded" << std::endl;
}

void StandardLibrary::installIntoRuntime(AlterionRuntime& runtime) {
    for (const auto& [name, factory] : moduleFactories) {
        runtime.registerModule(name, factory());
    }
    std::cout << "🔧 Standard library installed into runtime" << std::endl;
}

// Utility functions
AlterionValue createStdFunction(const std::string& name, 
                               const std::vector<std::string>& params,
                               std::function<AlterionValue(const std::vector<AlterionValue>&)> impl) {
    return AlterionValue(createFunction(name, params, impl));
}

AlterionValue createStdObject(const std::unordered_map<std::string, AlterionValue>& properties) {
    return AlterionValue(properties);
}

// Placeholder implementations for other modules
AlterionValue StdLib::ObjectModule::createModule() {
    return createStdObject({});
}

AlterionValue StdLib::JSONModule::createModule() {
    std::unordered_map<std::string, AlterionValue> module;
    module["stringify"] = createStdFunction("stringify", {"value"}, stringify_impl);
    module["parse"] = createStdFunction("parse", {"text"}, parse_impl);
    return AlterionValue(module);
}

AlterionValue StdLib::JSONModule::stringify_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue("undefined");
    return AlterionValue(args[0].toJSON());
}

AlterionValue StdLib::JSONModule::parse_impl(const std::vector<AlterionValue>& args) {
    if (args.empty()) return AlterionValue();
    // Simplified JSON parsing - would need proper JSON parser in production
    return AlterionValue();
}

// Placeholder implementations for other modules
AlterionValue StdLib::ObjectModule::keys_impl(const std::vector<AlterionValue>& args) { return createArray({}); }
AlterionValue StdLib::ObjectModule::values_impl(const std::vector<AlterionValue>& args) { return createArray({}); }
AlterionValue StdLib::ObjectModule::entries_impl(const std::vector<AlterionValue>& args) { return createArray({}); }
AlterionValue StdLib::ObjectModule::hasOwnProperty_impl(const std::vector<AlterionValue>& args) { return AlterionValue(false); }
AlterionValue StdLib::ObjectModule::assign_impl(const std::vector<AlterionValue>& args) { return AlterionValue(); }
AlterionValue StdLib::ObjectModule::freeze_impl(const std::vector<AlterionValue>& args) { return AlterionValue(); }
AlterionValue StdLib::ObjectModule::seal_impl(const std::vector<AlterionValue>& args) { return AlterionValue(); }
AlterionValue StdLib::ObjectModule::create_impl(const std::vector<AlterionValue>& args) { return AlterionValue(); }
AlterionValue StdLib::ObjectModule::defineProperty_impl(const std::vector<AlterionValue>& args) { return AlterionValue(); }
AlterionValue StdLib::ObjectModule::getOwnPropertyNames_impl(const std::vector<AlterionValue>& args) { return createArray({}); }

AlterionValue StdLib::DateModule::createModule() { return createStdObject({}); }
AlterionValue StdLib::DateModule::now_impl(const std::vector<AlterionValue>& args) { return AlterionValue(0); }
AlterionValue StdLib::DateModule::parse_impl(const std::vector<AlterionValue>& args) { return AlterionValue(0); }
AlterionValue StdLib::DateModule::UTC_impl(const std::vector<AlterionValue>& args) { return AlterionValue(0); }
std::shared_ptr<AlterionObject> StdLib::DateModule::createDateObject(int64_t timestamp) { return std::make_shared<AlterionObject>(); }

AlterionValue StdLib::PromiseModule::createModule() { return createStdObject({}); }
AlterionValue StdLib::PromiseModule::resolve_impl(const std::vector<AlterionValue>& args) { return AlterionValue(); }
AlterionValue StdLib::PromiseModule::reject_impl(const std::vector<AlterionValue>& args) { return AlterionValue(); }
AlterionValue StdLib::PromiseModule::all_impl(const std::vector<AlterionValue>& args) { return AlterionValue(); }
AlterionValue StdLib::PromiseModule::race_impl(const std::vector<AlterionValue>& args) { return AlterionValue(); }
AlterionValue StdLib::PromiseModule::allSettled_impl(const std::vector<AlterionValue>& args) { return AlterionValue(); }
std::shared_ptr<AlterionObject> StdLib::PromiseModule::createPromiseObject(
    std::function<void(std::function<void(AlterionValue)>, std::function<void(AlterionValue)>)> executor) {
    return std::make_shared<AlterionObject>();
}

AlterionValue StdLib::ComponentModule::createModule() { return createStdObject({}); }
AlterionValue StdLib::ComponentModule::createElement_impl(const std::vector<AlterionValue>& args) { return AlterionValue(); }
AlterionValue StdLib::ComponentModule::createComponent_impl(const std::vector<AlterionValue>& args) { return AlterionValue(); }
AlterionValue StdLib::ComponentModule::useState_impl(const std::vector<AlterionValue>& args) { return AlterionValue(); }
AlterionValue StdLib::ComponentModule::useEffect_impl(const std::vector<AlterionValue>& args) { return AlterionValue(); }
AlterionValue StdLib::ComponentModule::useCallback_impl(const std::vector<AlterionValue>& args) { return AlterionValue(); }
AlterionValue StdLib::ComponentModule::useMemo_impl(const std::vector<AlterionValue>& args) { return AlterionValue(); }
AlterionValue StdLib::ComponentModule::useRef_impl(const std::vector<AlterionValue>& args) { return AlterionValue(); }
AlterionValue StdLib::ComponentModule::useContext_impl(const std::vector<AlterionValue>& args) { return AlterionValue(); }

AlterionValue StdLib::DOMModule::createModule() { return createStdObject({}); }
// DOM module implementations would go here...

AlterionValue StdLib::HTTPModule::createModule() { return createStdObject({}); }
// HTTP module implementations would go here...

} // namespace Alterion
