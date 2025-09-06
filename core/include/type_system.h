#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <optional>
#include <variant>

// Phase 2C: Alterion Type System
// Comprehensive type checking with gradual typing support

namespace Alterion {

// Forward declarations
class Type;
class TypeChecker;
class TypeEnvironment;

using TypePtr = std::shared_ptr<Type>;

// Base Type class
class Type {
public:
    enum class Kind {
        Primitive,    // Int, Float, Bool, String, Null
        Array,        // Array<T>
        Object,       // {key: type, ...}
        Function,     // (params) => returnType
        Union,        // T | U
        Optional,     // T?
        Component,    // component type
        Any,          // Any type (for gradual typing)
        Unknown       // Unknown type (inference needed)
    };
    
    Kind kind;
    
    explicit Type(Kind k) : kind(k) {}
    virtual ~Type() = default;
    
    virtual std::string toString() const = 0;
    virtual bool isAssignableTo(const TypePtr& other) const = 0;
    virtual bool equals(const TypePtr& other) const = 0;
    
    // Type checking helpers
    bool isPrimitive() const { return kind == Kind::Primitive; }
    bool isArray() const { return kind == Kind::Array; }
    bool isObject() const { return kind == Kind::Object; }
    bool isFunction() const { return kind == Kind::Function; }
    bool isUnion() const { return kind == Kind::Union; }
    bool isOptional() const { return kind == Kind::Optional; }
    bool isComponent() const { return kind == Kind::Component; }
    bool isAny() const { return kind == Kind::Any; }
    bool isUnknown() const { return kind == Kind::Unknown; }
};

// Primitive types: Int, Float, Bool, String, Null
class PrimitiveType : public Type {
public:
    enum class PrimitiveKind {
        Int,     // 42, 0xFF, 0b1010
        Float,   // 3.14, 2.0
        Bool,    // true, false
        String,  // "hello", 'world'
        Null     // null, none
    };
    
    PrimitiveKind primitiveKind;
    
    explicit PrimitiveType(PrimitiveKind pk) 
        : Type(Kind::Primitive), primitiveKind(pk) {}
    
    std::string toString() const override {
        switch (primitiveKind) {
            case PrimitiveKind::Int: return "Int";
            case PrimitiveKind::Float: return "Float";
            case PrimitiveKind::Bool: return "Bool";
            case PrimitiveKind::String: return "String";
            case PrimitiveKind::Null: return "Null";
        }
        return "Unknown";
    }
    
    bool isAssignableTo(const TypePtr& other) const override {
        if (other->isAny()) return true;
        if (!other->isPrimitive()) return false;
        
        auto otherPrim = std::static_pointer_cast<PrimitiveType>(other);
        
        // Exact match
        if (primitiveKind == otherPrim->primitiveKind) return true;
        
        // Int can be assigned to Float
        if (primitiveKind == PrimitiveKind::Int && 
            otherPrim->primitiveKind == PrimitiveKind::Float) return true;
            
        // Null can be assigned to optional types (handled elsewhere)
        return false;
    }
    
    bool equals(const TypePtr& other) const override {
        if (!other->isPrimitive()) return false;
        auto otherPrim = std::static_pointer_cast<PrimitiveType>(other);
        return primitiveKind == otherPrim->primitiveKind;
    }
};

// Array type: Array<ElementType>
class ArrayType : public Type {
public:
    TypePtr elementType;
    
    explicit ArrayType(TypePtr elemType) 
        : Type(Kind::Array), elementType(std::move(elemType)) {}
    
    std::string toString() const override {
        return "Array<" + elementType->toString() + ">";
    }
    
    bool isAssignableTo(const TypePtr& other) const override {
        if (other->isAny()) return true;
        if (!other->isArray()) return false;
        
        auto otherArray = std::static_pointer_cast<ArrayType>(other);
        return elementType->isAssignableTo(otherArray->elementType);
    }
    
    bool equals(const TypePtr& other) const override {
        if (!other->isArray()) return false;
        auto otherArray = std::static_pointer_cast<ArrayType>(other);
        return elementType->equals(otherArray->elementType);
    }
};

// Object type: {key1: type1, key2: type2, ...}
class ObjectType : public Type {
public:
    std::unordered_map<std::string, TypePtr> properties;
    
    ObjectType() : Type(Kind::Object) {}
    
    void addProperty(const std::string& name, TypePtr type) {
        properties[name] = std::move(type);
    }
    
    std::string toString() const override {
        if (properties.empty()) return "{}";
        
        std::string result = "{";
        bool first = true;
        for (const auto& [name, type] : properties) {
            if (!first) result += ", ";
            result += name + ": " + type->toString();
            first = false;
        }
        result += "}";
        return result;
    }
    
    bool isAssignableTo(const TypePtr& other) const override {
        if (other->isAny()) return true;
        if (!other->isObject()) return false;
        
        auto otherObj = std::static_pointer_cast<ObjectType>(other);
        
        // Duck typing: this object must have all properties that other requires
        for (const auto& [name, requiredType] : otherObj->properties) {
            auto it = properties.find(name);
            if (it == properties.end()) return false; // Missing property
            if (!it->second->isAssignableTo(requiredType)) return false; // Wrong type
        }
        
        return true;
    }
    
    bool equals(const TypePtr& other) const override {
        if (!other->isObject()) return false;
        auto otherObj = std::static_pointer_cast<ObjectType>(other);
        
        if (properties.size() != otherObj->properties.size()) return false;
        
        for (const auto& [name, type] : properties) {
            auto it = otherObj->properties.find(name);
            if (it == otherObj->properties.end()) return false;
            if (!type->equals(it->second)) return false;
        }
        
        return true;
    }
};

// Function type: (param1: type1, param2: type2) => returnType
class FunctionType : public Type {
public:
    std::vector<TypePtr> parameterTypes;
    TypePtr returnType;
    bool isAsync;
    
    FunctionType(std::vector<TypePtr> params, TypePtr ret, bool async = false)
        : Type(Kind::Function), parameterTypes(std::move(params)), 
          returnType(std::move(ret)), isAsync(async) {}
    
    std::string toString() const override {
        std::string result = "(";
        for (size_t i = 0; i < parameterTypes.size(); ++i) {
            if (i > 0) result += ", ";
            result += parameterTypes[i]->toString();
        }
        result += ") => " + returnType->toString();
        if (isAsync) result = "async " + result;
        return result;
    }
    
    bool isAssignableTo(const TypePtr& other) const override {
        if (other->isAny()) return true;
        if (!other->isFunction()) return false;
        
        auto otherFunc = std::static_pointer_cast<FunctionType>(other);
        
        // Parameter types must be contravariant
        if (parameterTypes.size() != otherFunc->parameterTypes.size()) return false;
        
        for (size_t i = 0; i < parameterTypes.size(); ++i) {
            if (!otherFunc->parameterTypes[i]->isAssignableTo(parameterTypes[i])) {
                return false;
            }
        }
        
        // Return type must be covariant
        return returnType->isAssignableTo(otherFunc->returnType);
    }
    
    bool equals(const TypePtr& other) const override {
        if (!other->isFunction()) return false;
        auto otherFunc = std::static_pointer_cast<FunctionType>(other);
        
        if (parameterTypes.size() != otherFunc->parameterTypes.size()) return false;
        if (isAsync != otherFunc->isAsync) return false;
        
        for (size_t i = 0; i < parameterTypes.size(); ++i) {
            if (!parameterTypes[i]->equals(otherFunc->parameterTypes[i])) return false;
        }
        
        return returnType->equals(otherFunc->returnType);
    }
};

// Union type: T | U
class UnionType : public Type {
public:
    std::vector<TypePtr> types;
    
    UnionType(std::vector<TypePtr> unionTypes) 
        : Type(Kind::Union), types(std::move(unionTypes)) {}
    
    std::string toString() const override {
        if (types.empty()) return "never";
        
        std::string result = types[0]->toString();
        for (size_t i = 1; i < types.size(); ++i) {
            result += " | " + types[i]->toString();
        }
        return result;
    }
    
    bool isAssignableTo(const TypePtr& other) const override {
        if (other->isAny()) return true;
        
        // All types in the union must be assignable to other
        for (const auto& type : types) {
            if (!type->isAssignableTo(other)) return false;
        }
        return true;
    }
    
    bool equals(const TypePtr& other) const override {
        if (!other->isUnion()) return false;
        auto otherUnion = std::static_pointer_cast<UnionType>(other);
        
        if (types.size() != otherUnion->types.size()) return false;
        
        // Order-independent comparison
        for (const auto& type : types) {
            bool found = false;
            for (const auto& otherType : otherUnion->types) {
                if (type->equals(otherType)) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }
        
        return true;
    }
};

// Optional type: T?
class OptionalType : public Type {
public:
    TypePtr innerType;
    
    explicit OptionalType(TypePtr inner) 
        : Type(Kind::Optional), innerType(std::move(inner)) {}
    
    std::string toString() const override {
        return innerType->toString() + "?";
    }
    
    bool isAssignableTo(const TypePtr& other) const override {
        if (other->isAny()) return true;
        if (other->isOptional()) {
            auto otherOpt = std::static_pointer_cast<OptionalType>(other);
            return innerType->isAssignableTo(otherOpt->innerType);
        }
        
        // Optional can be assigned if inner type is assignable
        return innerType->isAssignableTo(other);
    }
    
    bool equals(const TypePtr& other) const override {
        if (!other->isOptional()) return false;
        auto otherOpt = std::static_pointer_cast<OptionalType>(other);
        return innerType->equals(otherOpt->innerType);
    }
};

// Component type
class ComponentType : public Type {
public:
    std::string name;
    std::unordered_map<std::string, TypePtr> props;
    std::unordered_map<std::string, TypePtr> state;
    std::vector<std::string> modifiers;
    
    explicit ComponentType(const std::string& componentName) 
        : Type(Kind::Component), name(componentName) {}
    
    std::string toString() const override {
        return "Component<" + name + ">";
    }
    
    bool isAssignableTo(const TypePtr& other) const override {
        if (other->isAny()) return true;
        if (!other->isComponent()) return false;
        
        auto otherComp = std::static_pointer_cast<ComponentType>(other);
        return name == otherComp->name;
    }
    
    bool equals(const TypePtr& other) const override {
        if (!other->isComponent()) return false;
        auto otherComp = std::static_pointer_cast<ComponentType>(other);
        return name == otherComp->name;
    }
};

// Any type (for gradual typing)
class AnyType : public Type {
public:
    AnyType() : Type(Kind::Any) {}
    
    std::string toString() const override { return "any"; }
    
    bool isAssignableTo(const TypePtr& other) const override {
        return true; // Any is assignable to everything
    }
    
    bool equals(const TypePtr& other) const override {
        return other->isAny();
    }
};

// Unknown type (for inference)
class UnknownType : public Type {
public:
    UnknownType() : Type(Kind::Unknown) {}
    
    std::string toString() const override { return "unknown"; }
    
    bool isAssignableTo(const TypePtr& other) const override {
        return false; // Unknown needs to be inferred first
    }
    
    bool equals(const TypePtr& other) const override {
        return other->isUnknown();
    }
};

// Type factory for creating common types
class TypeFactory {
public:
    static TypePtr createInt() {
        return std::make_shared<PrimitiveType>(PrimitiveType::PrimitiveKind::Int);
    }
    
    static TypePtr createFloat() {
        return std::make_shared<PrimitiveType>(PrimitiveType::PrimitiveKind::Float);
    }
    
    static TypePtr createBool() {
        return std::make_shared<PrimitiveType>(PrimitiveType::PrimitiveKind::Bool);
    }
    
    static TypePtr createString() {
        return std::make_shared<PrimitiveType>(PrimitiveType::PrimitiveKind::String);
    }
    
    static TypePtr createNull() {
        return std::make_shared<PrimitiveType>(PrimitiveType::PrimitiveKind::Null);
    }
    
    static TypePtr createArray(TypePtr elementType) {
        return std::make_shared<ArrayType>(std::move(elementType));
    }
    
    static TypePtr createObject() {
        return std::make_shared<ObjectType>();
    }
    
    static TypePtr createFunction(std::vector<TypePtr> params, TypePtr returnType, bool isAsync = false) {
        return std::make_shared<FunctionType>(std::move(params), std::move(returnType), isAsync);
    }
    
    static TypePtr createUnion(std::vector<TypePtr> types) {
        return std::make_shared<UnionType>(std::move(types));
    }
    
    static TypePtr createOptional(TypePtr innerType) {
        return std::make_shared<OptionalType>(std::move(innerType));
    }
    
    static TypePtr createComponent(const std::string& name) {
        return std::make_shared<ComponentType>(name);
    }
    
    static TypePtr createAny() {
        return std::make_shared<AnyType>();
    }
    
    static TypePtr createUnknown() {
        return std::make_shared<UnknownType>();
    }
};

} // namespace Alterion
