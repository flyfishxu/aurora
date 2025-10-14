#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>

namespace llvm {
class Type;
class LLVMContext;
}

namespace aurora {

/// Type system for AuroraLang with null safety
class Type {
public:
    enum class Kind {
        Void,       // void type
        Int,        // 64-bit integer
        Double,     // 64-bit floating point
        Bool,       // boolean (true/false)
        String,     // string type
        Optional,   // Optional<T> for null safety
        Function,   // function type
        Class,      // class type
        Array,      // array type [T]
    };

    virtual ~Type() = default;
    virtual Kind getKind() const = 0;
    virtual std::string toString() const = 0;
    virtual llvm::Type* toLLVMType(llvm::LLVMContext& ctx) const = 0;
    virtual bool isNullable() const { return false; }
    
    // For method overloading and name mangling
    virtual std::string getMangledName() const = 0;
    virtual bool equals(const Type* other) const = 0;
    
    bool isInt() const { return getKind() == Kind::Int; }
    bool isDouble() const { return getKind() == Kind::Double; }
    bool isBool() const { return getKind() == Kind::Bool; }
    bool isString() const { return getKind() == Kind::String; }
    bool isOptional() const { return getKind() == Kind::Optional; }
    bool isFunction() const { return getKind() == Kind::Function; }
    bool isVoid() const { return getKind() == Kind::Void; }
    bool isClass() const { return getKind() == Kind::Class; }
    bool isArray() const { return getKind() == Kind::Array; }
};

/// Void type
class VoidType : public Type {
public:
    Kind getKind() const override { return Kind::Void; }
    std::string toString() const override { return "void"; }
    llvm::Type* toLLVMType(llvm::LLVMContext& ctx) const override;
    std::string getMangledName() const override;
    bool equals(const Type* other) const override;
};

/// Integer type (64-bit)
class IntType : public Type {
public:
    Kind getKind() const override { return Kind::Int; }
    std::string toString() const override { return "int"; }
    llvm::Type* toLLVMType(llvm::LLVMContext& ctx) const override;
    std::string getMangledName() const override;
    bool equals(const Type* other) const override;
};

/// Double type (64-bit floating point)
class DoubleType : public Type {
public:
    Kind getKind() const override { return Kind::Double; }
    std::string toString() const override { return "double"; }
    llvm::Type* toLLVMType(llvm::LLVMContext& ctx) const override;
    std::string getMangledName() const override;
    bool equals(const Type* other) const override;
};

/// Boolean type
class BoolType : public Type {
public:
    Kind getKind() const override { return Kind::Bool; }
    std::string toString() const override { return "bool"; }
    llvm::Type* toLLVMType(llvm::LLVMContext& ctx) const override;
    std::string getMangledName() const override;
    bool equals(const Type* other) const override;
};

/// String type
class StringType : public Type {
public:
    Kind getKind() const override { return Kind::String; }
    std::string toString() const override { return "string"; }
    llvm::Type* toLLVMType(llvm::LLVMContext& ctx) const override;
    std::string getMangledName() const override;
    bool equals(const Type* other) const override;
};

/// Optional type for null safety (Optional<T>)
class OptionalType : public Type {
public:
    explicit OptionalType(std::shared_ptr<Type> inner) : innerType(std::move(inner)) {}
    
    Kind getKind() const override { return Kind::Optional; }
    std::string toString() const override;
    llvm::Type* toLLVMType(llvm::LLVMContext& ctx) const override;
    bool isNullable() const override { return true; }
    std::string getMangledName() const override;
    bool equals(const Type* other) const override;
    
    std::shared_ptr<Type> getInnerType() const { return innerType; }

private:
    std::shared_ptr<Type> innerType;
};

/// Function type
class FunctionType : public Type {
public:
    FunctionType(std::shared_ptr<Type> ret, std::vector<std::shared_ptr<Type>> params)
        : returnType(std::move(ret)), paramTypes(std::move(params)) {}
    
    Kind getKind() const override { return Kind::Function; }
    std::string toString() const override;
    llvm::Type* toLLVMType(llvm::LLVMContext& ctx) const override;
    std::string getMangledName() const override;
    bool equals(const Type* other) const override;
    
    std::shared_ptr<Type> getReturnType() const { return returnType; }
    const std::vector<std::shared_ptr<Type>>& getParamTypes() const { return paramTypes; }

private:
    std::shared_ptr<Type> returnType;
    std::vector<std::shared_ptr<Type>> paramTypes;
};

/// Array type [T]
class ArrayType : public Type {
public:
    explicit ArrayType(std::shared_ptr<Type> elem) : elementType(std::move(elem)) {}
    
    Kind getKind() const override { return Kind::Array; }
    std::string toString() const override;
    llvm::Type* toLLVMType(llvm::LLVMContext& ctx) const override;
    std::string getMangledName() const override;
    bool equals(const Type* other) const override;
    
    std::shared_ptr<Type> getElementType() const { return elementType; }

private:
    std::shared_ptr<Type> elementType;
};

/// Class type (forward declaration of ClassDecl for circular dependency)
class ClassDecl;

/// Class type
class ClassType : public Type {
public:
    explicit ClassType(std::string n) : name(std::move(n)), decl(nullptr) {}
    
    Kind getKind() const override { return Kind::Class; }
    std::string toString() const override { return name; }
    llvm::Type* toLLVMType(llvm::LLVMContext& ctx) const override;
    std::string getMangledName() const override;
    bool equals(const Type* other) const override;
    
    const std::string& getName() const { return name; }
    void setDecl(ClassDecl* d) { decl = d; }
    ClassDecl* getDecl() const { return decl; }

private:
    std::string name;
    ClassDecl* decl;  // Pointer to the class declaration
};

/// Type registry and factory
class TypeRegistry {
public:
    static TypeRegistry& instance();
    
    std::shared_ptr<VoidType> getVoidType();
    std::shared_ptr<IntType> getIntType();
    std::shared_ptr<DoubleType> getDoubleType();
    std::shared_ptr<BoolType> getBoolType();
    std::shared_ptr<StringType> getStringType();
    std::shared_ptr<OptionalType> getOptionalType(std::shared_ptr<Type> inner);
    std::shared_ptr<FunctionType> getFunctionType(std::shared_ptr<Type> ret, 
                                                   std::vector<std::shared_ptr<Type>> params);
    std::shared_ptr<ClassType> getClassType(const std::string& name);
    bool hasClassType(const std::string& name) const;
    std::shared_ptr<ArrayType> getArrayType(std::shared_ptr<Type> elementType);

private:
    TypeRegistry() = default;
    
    std::shared_ptr<VoidType> voidType;
    std::shared_ptr<IntType> intType;
    std::shared_ptr<DoubleType> doubleType;
    std::shared_ptr<BoolType> boolType;
    std::shared_ptr<StringType> stringType;
    std::map<std::string, std::shared_ptr<ClassType>> classTypes;
};

} // namespace aurora
