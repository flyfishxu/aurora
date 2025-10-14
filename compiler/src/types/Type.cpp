#include "aurora/Type.h"
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>

namespace aurora {

// VoidType implementation
llvm::Type* VoidType::toLLVMType(llvm::LLVMContext& ctx) const {
    return llvm::Type::getVoidTy(ctx);
}

std::string VoidType::getMangledName() const {
    return "v";
}

bool VoidType::equals(const Type* other) const {
    return other && other->isVoid();
}

// IntType implementation
llvm::Type* IntType::toLLVMType(llvm::LLVMContext& ctx) const {
    return llvm::Type::getInt64Ty(ctx);
}

std::string IntType::getMangledName() const {
    return "i";
}

bool IntType::equals(const Type* other) const {
    return other && other->isInt();
}

// DoubleType implementation
llvm::Type* DoubleType::toLLVMType(llvm::LLVMContext& ctx) const {
    return llvm::Type::getDoubleTy(ctx);
}

std::string DoubleType::getMangledName() const {
    return "d";
}

bool DoubleType::equals(const Type* other) const {
    return other && other->isDouble();
}

// BoolType implementation
llvm::Type* BoolType::toLLVMType(llvm::LLVMContext& ctx) const {
    return llvm::Type::getInt1Ty(ctx);
}

std::string BoolType::getMangledName() const {
    return "b";
}

bool BoolType::equals(const Type* other) const {
    return other && other->isBool();
}

// StringType implementation
llvm::Type* StringType::toLLVMType(llvm::LLVMContext& ctx) const {
    // Strings are represented as runtime-managed pointers (AuroraString*)
    return llvm::PointerType::get(ctx, 0);
}

std::string StringType::getMangledName() const {
    return "s";
}

bool StringType::equals(const Type* other) const {
    return other && other->isString();
}

// OptionalType implementation
std::string OptionalType::toString() const {
    return innerType->toString() + "?";
}

llvm::Type* OptionalType::toLLVMType(llvm::LLVMContext& ctx) const {
    // Optional<T> is represented as {i1, T} (has_value flag + value)
    llvm::Type* payloadType = innerType->isVoid()
        ? static_cast<llvm::Type*>(llvm::Type::getInt8Ty(ctx))
        : innerType->toLLVMType(ctx);
    
    return llvm::StructType::get(
        llvm::Type::getInt1Ty(ctx),
        payloadType
    );
}

std::string OptionalType::getMangledName() const {
    return "o" + innerType->getMangledName();
}

bool OptionalType::equals(const Type* other) const {
    if (!other || !other->isOptional()) return false;
    const OptionalType* opt = static_cast<const OptionalType*>(other);
    return innerType->equals(opt->innerType.get());
}

// FunctionType implementation
std::string FunctionType::toString() const {
    std::string result = "fn(";
    for (size_t i = 0; i < paramTypes.size(); ++i) {
        if (i > 0) result += ", ";
        result += paramTypes[i]->toString();
    }
    result += ") -> " + returnType->toString();
    return result;
}

llvm::Type* FunctionType::toLLVMType(llvm::LLVMContext& ctx) const {
    std::vector<llvm::Type*> llvmParams;
    for (const auto& param : paramTypes) {
        llvmParams.push_back(param->toLLVMType(ctx));
    }
    return llvm::FunctionType::get(
        returnType->toLLVMType(ctx),
        llvmParams,
        false  // not vararg
    );
}

std::string FunctionType::getMangledName() const {
    std::string result = "f";
    for (const auto& param : paramTypes) {
        result += param->getMangledName();
    }
    result += "r" + returnType->getMangledName();
    return result;
}

bool FunctionType::equals(const Type* other) const {
    if (!other || !other->isFunction()) return false;
    const FunctionType* func = static_cast<const FunctionType*>(other);
    if (paramTypes.size() != func->paramTypes.size()) return false;
    if (!returnType->equals(func->returnType.get())) return false;
    for (size_t i = 0; i < paramTypes.size(); i++) {
        if (!paramTypes[i]->equals(func->paramTypes[i].get())) return false;
    }
    return true;
}

// TypeRegistry implementation
TypeRegistry& TypeRegistry::instance() {
    static TypeRegistry registry;
    return registry;
}

std::shared_ptr<VoidType> TypeRegistry::getVoidType() {
    if (!voidType) voidType = std::make_shared<VoidType>();
    return voidType;
}

std::shared_ptr<IntType> TypeRegistry::getIntType() {
    if (!intType) intType = std::make_shared<IntType>();
    return intType;
}

std::shared_ptr<DoubleType> TypeRegistry::getDoubleType() {
    if (!doubleType) doubleType = std::make_shared<DoubleType>();
    return doubleType;
}

std::shared_ptr<BoolType> TypeRegistry::getBoolType() {
    if (!boolType) boolType = std::make_shared<BoolType>();
    return boolType;
}

std::shared_ptr<StringType> TypeRegistry::getStringType() {
    if (!stringType) stringType = std::make_shared<StringType>();
    return stringType;
}

std::shared_ptr<OptionalType> TypeRegistry::getOptionalType(std::shared_ptr<Type> inner) {
    return std::make_shared<OptionalType>(std::move(inner));
}

std::shared_ptr<FunctionType> TypeRegistry::getFunctionType(
    std::shared_ptr<Type> ret, 
    std::vector<std::shared_ptr<Type>> params) {
    return std::make_shared<FunctionType>(std::move(ret), std::move(params));
}

std::shared_ptr<ClassType> TypeRegistry::getClassType(const std::string& name) {
    auto it = classTypes.find(name);
    if (it != classTypes.end()) {
        return it->second;
    }
    auto classType = std::make_shared<ClassType>(name);
    classTypes[name] = classType;
    return classType;
}

bool TypeRegistry::hasClassType(const std::string& name) const {
    return classTypes.find(name) != classTypes.end();
}

// ArrayType implementation
std::string ArrayType::toString() const {
    return "[" + elementType->toString() + "]";
}

llvm::Type* ArrayType::toLLVMType(llvm::LLVMContext& ctx) const {
    // Arrays are represented as {i64, ptr} (length + pointer to elements)
    return llvm::StructType::get(
        llvm::Type::getInt64Ty(ctx),  // length
        llvm::PointerType::get(ctx, 0)  // pointer to elements
    );
}

std::string ArrayType::getMangledName() const {
    return "a" + elementType->getMangledName();
}

bool ArrayType::equals(const Type* other) const {
    if (!other || !other->isArray()) return false;
    const ArrayType* arr = static_cast<const ArrayType*>(other);
    return elementType->equals(arr->elementType.get());
}

// ClassType implementation
llvm::Type* ClassType::toLLVMType(llvm::LLVMContext& ctx) const {
    // Class instances are represented as pointers to struct types
    // The struct type will be defined based on the class's fields
    // For now, we'll use an opaque pointer (i8*)
    return llvm::PointerType::get(ctx, 0);
}

std::string ClassType::getMangledName() const {
    return "c" + name;
}

bool ClassType::equals(const Type* other) const {
    if (!other || !other->isClass()) return false;
    const ClassType* cls = static_cast<const ClassType*>(other);
    return name == cls->name;
}

std::shared_ptr<ArrayType> TypeRegistry::getArrayType(std::shared_ptr<Type> elementType) {
    return std::make_shared<ArrayType>(std::move(elementType));
}

} // namespace aurora
