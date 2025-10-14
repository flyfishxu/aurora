#include "aurora/AST.h"
#include "aurora/CodeGen.h"
#include "aurora/Diagnostic.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>

namespace aurora {

// Helper to log errors using diagnostic system
static llvm::Value* logError(const char* str, const char* code = "E3001") {
    auto& diag = getDiagnosticEngine();
    SourceLocation loc("<codegen>", 0, 0, 0);
    diag.reportError(code, str, loc);
    return nullptr;
}

// ===== Array Literal Expression =====
// Uses Runtime API (aurora_array_create)

llvm::Value* ArrayLiteralExpr::codegen() {
    auto& ctx = getGlobalContext();
    
    // Get the element type from the array type
    auto arrayType = std::dynamic_pointer_cast<ArrayType>(type);
    if (!arrayType) {
        return logError("ArrayLiteralExpr has non-array type");
    }
    
    llvm::Type* elemLLVMType = arrayType->getElementType()->toLLVMType(ctx.getContext());
    size_t numElements = elements.size();
    uint64_t elemSize = ctx.getModule().getDataLayout().getTypeAllocSize(elemLLVMType);
    
    // Declare runtime function if not already declared
    llvm::Function* arrayCreateFunc = ctx.getModule().getFunction("aurora_array_create");
    if (!arrayCreateFunc) {
        // AuroraArray* aurora_array_create(int64_t element_size, int64_t element_count)
        llvm::FunctionType* funcType = llvm::FunctionType::get(
            llvm::PointerType::get(ctx.getContext(), 0),
            {llvm::Type::getInt64Ty(ctx.getContext()), 
             llvm::Type::getInt64Ty(ctx.getContext())},
            false);
        arrayCreateFunc = llvm::Function::Create(funcType, 
            llvm::Function::ExternalLinkage,
            "aurora_array_create", ctx.getModule());
    }
    
    // Call runtime to create array
    llvm::Value* elemSizeVal = llvm::ConstantInt::get(
        llvm::Type::getInt64Ty(ctx.getContext()), elemSize);
    llvm::Value* elemCountVal = llvm::ConstantInt::get(
        llvm::Type::getInt64Ty(ctx.getContext()), numElements);
    
    llvm::Value* runtimeArray = ctx.getBuilder().CreateCall(
        arrayCreateFunc, {elemSizeVal, elemCountVal}, "array");
    
    // Declare aurora_array_set if needed
    llvm::Function* arraySetFunc = ctx.getModule().getFunction("aurora_array_set");
    if (!arraySetFunc) {
        // void aurora_array_set(AuroraArray* array, int64_t index, void* element, int64_t element_size)
        llvm::FunctionType* funcType = llvm::FunctionType::get(
            llvm::Type::getVoidTy(ctx.getContext()),
            {llvm::PointerType::get(ctx.getContext(), 0),
             llvm::Type::getInt64Ty(ctx.getContext()),
             llvm::PointerType::get(ctx.getContext(), 0),
             llvm::Type::getInt64Ty(ctx.getContext())},
            false);
        arraySetFunc = llvm::Function::Create(funcType,
            llvm::Function::ExternalLinkage,
            "aurora_array_set", ctx.getModule());
    }
    
    // Store elements using runtime API
    for (size_t i = 0; i < numElements; ++i) {
        llvm::Value* elemVal = elements[i]->codegen();
        if (!elemVal) return nullptr;
        
        // Allocate temporary storage for element
        llvm::AllocaInst* elemAlloca = ctx.getBuilder().CreateAlloca(elemLLVMType, nullptr, "elem_tmp");
        ctx.getBuilder().CreateStore(elemVal, elemAlloca);
        
        // Call aurora_array_set
        llvm::Value* indexVal = llvm::ConstantInt::get(
            llvm::Type::getInt64Ty(ctx.getContext()), i);
        ctx.getBuilder().CreateCall(arraySetFunc, 
            {runtimeArray, indexVal, elemAlloca, elemSizeVal});
    }
    
    // Convert runtime array pointer to our array struct format
    llvm::Function* arrayGetLengthFunc = ctx.getModule().getFunction("aurora_array_length");
    if (!arrayGetLengthFunc) {
        llvm::FunctionType* funcType = llvm::FunctionType::get(
            llvm::Type::getInt64Ty(ctx.getContext()),
            {llvm::PointerType::get(ctx.getContext(), 0)},
            false);
        arrayGetLengthFunc = llvm::Function::Create(funcType,
            llvm::Function::ExternalLinkage,
            "aurora_array_length", ctx.getModule());
    }
    
    // Get length
    llvm::Value* length = ctx.getBuilder().CreateCall(arrayGetLengthFunc, {runtimeArray}, "length");
    
    // Get data pointer (access AuroraArray->data field)
    // AuroraArray is: struct { AuroraRefCountHeader header; int64_t length; void* data; }
    // AuroraRefCountHeader is: struct { int64_t ref_count; int32_t type; }
    llvm::StructType* refCountHeaderType = llvm::StructType::get(ctx.getContext(),
        {llvm::Type::getInt64Ty(ctx.getContext()),    // ref_count
         llvm::Type::getInt32Ty(ctx.getContext())});  // type (enum)
    
    llvm::StructType* runtimeArrayType = llvm::StructType::get(ctx.getContext(),
        {refCountHeaderType,                           // header (field 0)
         llvm::Type::getInt64Ty(ctx.getContext()),     // length (field 1)
         llvm::PointerType::get(ctx.getContext(), 0)}); // data (field 2)
    
    llvm::Value* dataFieldPtr = ctx.getBuilder().CreateStructGEP(
        runtimeArrayType, runtimeArray, 2, "data_field_ptr");  // Field 2, not 1!
    llvm::Value* dataPtr = ctx.getBuilder().CreateLoad(
        llvm::PointerType::get(ctx.getContext(), 0), dataFieldPtr, "data");
    
    // Create our array struct: {length, data}
    llvm::Type* arrayStructType = type->toLLVMType(ctx.getContext());
    llvm::Value* arrayStruct = llvm::UndefValue::get(arrayStructType);
    
    arrayStruct = ctx.getBuilder().CreateInsertValue(arrayStruct, length, 0);
    arrayStruct = ctx.getBuilder().CreateInsertValue(arrayStruct, dataPtr, 1);
    
    return arrayStruct;
}

// ===== Array Index Expression =====

llvm::Value* ArrayIndexExpr::codegen() {
    auto& ctx = getGlobalContext();
    
    llvm::Value* arrayVal = array->codegen();
    llvm::Value* indexVal = index->codegen();
    
    if (!arrayVal || !indexVal) return nullptr;
    
    // Convert index to i64 if needed
    if (!indexVal->getType()->isIntegerTy(64)) {
        if (indexVal->getType()->isDoubleTy()) {
            indexVal = ctx.getBuilder().CreateFPToSI(indexVal, 
                llvm::Type::getInt64Ty(ctx.getContext()), "idx_conv");
        } else if (indexVal->getType()->isIntegerTy()) {
            indexVal = ctx.getBuilder().CreateZExtOrTrunc(indexVal,
                llvm::Type::getInt64Ty(ctx.getContext()), "idx_ext");
        }
    }
    
    // Extract data pointer from array struct
    llvm::Value* dataPtr = ctx.getBuilder().CreateExtractValue(arrayVal, 1, "array_data");
    
    // Get element type
    auto arrayType = std::dynamic_pointer_cast<ArrayType>(array->getType());
    if (!arrayType) {
        return logError("Array index on non-array type");
    }
    
    llvm::Type* elemType = arrayType->getElementType()->toLLVMType(ctx.getContext());
    uint64_t elemSize = ctx.getModule().getDataLayout().getTypeAllocSize(elemType);
    
    // Calculate offset: index * sizeof(element)
    llvm::Value* elemSizeVal = llvm::ConstantInt::get(
        llvm::Type::getInt64Ty(ctx.getContext()), elemSize);
    llvm::Value* offset = ctx.getBuilder().CreateMul(indexVal, elemSizeVal, "offset");
    
    // GEP to get element address
    llvm::Value* elemPtr = ctx.getBuilder().CreateGEP(
        llvm::Type::getInt8Ty(ctx.getContext()), dataPtr, offset, "elem_ptr");
    
    // Load the element
    return ctx.getBuilder().CreateLoad(elemType, elemPtr, "elem");
}

std::shared_ptr<Type> ArrayIndexExpr::getType() const {
    auto arrayType = std::dynamic_pointer_cast<ArrayType>(array->getType());
    if (arrayType) {
        return arrayType->getElementType();
    }
    return TypeRegistry::instance().getIntType(); // fallback
}

} // namespace aurora

