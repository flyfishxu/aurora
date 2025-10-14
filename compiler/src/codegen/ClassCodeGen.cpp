#include "aurora/AST.h"
#include "aurora/CodeGen.h"
#include "aurora/Diagnostic.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

namespace aurora {

// Helper to log errors using diagnostic system
static llvm::Value* logError(const char* str, const char* code = "E3004") {
    auto& diag = getDiagnosticEngine();
    SourceLocation loc("<codegen>", 0, 0, 0);
    diag.reportError(code, str, loc);
    return nullptr;
}

// Generate mangled name for method with parameter types (for overloading)
static std::string mangleMethodName(const std::string& className, 
                                   const std::string& methodName,
                                   const std::vector<Parameter>& params) {
    std::string mangledName = className + "_" + methodName;
    
    // Add parameter types to mangled name for overloading
    if (!params.empty()) {
        mangledName += "_";
        for (size_t i = 0; i < params.size(); i++) {
            if (i > 0) mangledName += "_";
            mangledName += params[i].type->getMangledName();
        }
    }
    
    return mangledName;
}

// ===== Member Access Expression =====

llvm::Value* MemberAccessExpr::codegen() {
    auto& ctx = getGlobalContext();
    
    // Check if object is 'this'
    if (dynamic_cast<ThisExpr*>(object.get())) {
        auto& named_values = ctx.getNamedValues();
        llvm::AllocaInst* thisAlloca = named_values["this"];
        if (!thisAlloca) {
            return logError("'this' not found in current context");
        }
        
        llvm::Value* thisPtr = ctx.getBuilder().CreateLoad(
            thisAlloca->getAllocatedType(), thisAlloca, "this");
        
        // Extract class name from current function name
        llvm::BasicBlock* currentBlock = ctx.getBuilder().GetInsertBlock();
        if (!currentBlock) {
            return logError("No current basic block in member access");
        }
        
        llvm::Function* currentFunc = currentBlock->getParent();
        if (!currentFunc) {
            return logError("No parent function in member access");
        }
        
        std::string funcName = std::string(currentFunc->getName());
        size_t underscorePos = funcName.find('_');
        if (underscorePos == std::string::npos) {
            return logError("Cannot determine class name for member access");
        }
        std::string className = funcName.substr(0, underscorePos);
        
        // Get the class type and declaration
        auto classType = TypeRegistry::instance().getClassType(className);
        if (!classType) {
            return logError(("Class type not found: " + className).c_str());
        }
        
        ClassDecl* classDecl = classType->getDecl();
        if (!classDecl) {
            return logError(("Class declaration not found for member access: " + className).c_str());
        }
        
        // Find the field
        FieldDecl* field = classDecl->findField(member);
        if (!field) {
            return logError(("Field not found: " + member + " in class " + className).c_str());
        }
        
        // Get field index
        size_t fieldIndex = 0;
        for (const auto& f : classDecl->getFields()) {
            if (f.name == member) break;
            fieldIndex++;
        }
        
        // Get the struct type
        llvm::Type* structType = classDecl->codegen();
        if (!structType) {
            return logError(("Failed to generate struct type for class: " + className).c_str());
        }
        
        // GEP to get field pointer
        llvm::Value* fieldPtr = ctx.getBuilder().CreateStructGEP(
            structType, thisPtr, fieldIndex, member.c_str());
        
        // Load the field value
        return ctx.getBuilder().CreateLoad(field->type->toLLVMType(ctx.getContext()), 
                                           fieldPtr, member.c_str());
    } else {
        // Handle member access for regular objects
        llvm::Value* objPtr = object->codegen();
        if (!objPtr) return nullptr;
        
        // Try to get the class type from the object
        auto classType = std::dynamic_pointer_cast<ClassType>(object->getType());
        
        if (!classType) {
            if (auto* newExpr = dynamic_cast<NewExpr*>(object.get())) {
                classType = std::dynamic_pointer_cast<ClassType>(newExpr->getType());
            }
        }
        
        if (!classType) {
            if (auto* varExpr = dynamic_cast<VariableExpr*>(object.get())) {
                auto varType = ctx.getVariableType(varExpr->getName());
                if (varType) {
                    classType = std::dynamic_pointer_cast<ClassType>(varType);
                }
            }
        }
        
        if (!classType) {
            return logError(("Cannot determine class type for member access: " + member).c_str());
        }
        
        // Get the class declaration
        ClassDecl* classDecl = classType->getDecl();
        if (!classDecl) {
            return logError(("Class declaration not found for member access: " + classType->getName()).c_str());
        }
        
        // Find the field
        FieldDecl* field = classDecl->findField(member);
        if (!field) {
            return logError(("Field not found: " + member + " in class " + classType->getName()).c_str());
        }
        
        // Get field index
        size_t fieldIndex = 0;
        for (const auto& f : classDecl->getFields()) {
            if (f.name == member) break;
            fieldIndex++;
        }
        
        // Get the struct type
        llvm::Type* structType = classDecl->codegen();
        if (!structType) {
            return logError(("Failed to generate struct type for class: " + classType->getName()).c_str());
        }
        
        // GEP to get field pointer
        llvm::Value* fieldPtr = ctx.getBuilder().CreateStructGEP(
            structType, objPtr, fieldIndex, member.c_str());
        
        // Load the field value
        return ctx.getBuilder().CreateLoad(field->type->toLLVMType(ctx.getContext()), 
                                           fieldPtr, member.c_str());
    }
}

// ===== Member Call Expression =====

std::shared_ptr<Type> MemberCallExpr::getType() const {
    // Try to resolve the actual return type dynamically
    auto classType = std::dynamic_pointer_cast<ClassType>(object->getType());
    
    if (!classType) {
        if (auto* newExpr = dynamic_cast<NewExpr*>(object.get())) {
            classType = std::dynamic_pointer_cast<ClassType>(newExpr->getType());
        }
    }
    
    if (!classType) {
        if (auto* varExpr = dynamic_cast<VariableExpr*>(object.get())) {
            auto& ctx = getGlobalContext();
            auto varType = ctx.getVariableType(varExpr->getName());
            if (varType) {
                classType = std::dynamic_pointer_cast<ClassType>(varType);
            }
        }
    }
    
    if (classType) {
        ClassDecl* classDecl = classType->getDecl();
        if (classDecl) {
            MethodDecl* methodDecl = classDecl->findMethod(method);
            if (methodDecl) {
                // Update cached type with actual return type
                type = methodDecl->returnType;
                return type;
            }
        }
    }
    
    // Fallback to cached type
    return type;
}

llvm::Value* MemberCallExpr::codegen() {
    auto& ctx = getGlobalContext();
    
    llvm::Value* objPtr = object->codegen();
    if (!objPtr) return nullptr;
    
    // Try to get the class type
    auto classType = std::dynamic_pointer_cast<ClassType>(object->getType());
    
    if (!classType) {
        if (auto* newExpr = dynamic_cast<NewExpr*>(object.get())) {
            classType = std::dynamic_pointer_cast<ClassType>(newExpr->getType());
        }
    }
    
    if (!classType) {
        if (auto* varExpr = dynamic_cast<VariableExpr*>(object.get())) {
            auto varType = ctx.getVariableType(varExpr->getName());
            if (varType) {
                classType = std::dynamic_pointer_cast<ClassType>(varType);
            }
        }
    }
    
    if (!classType) {
        return logError("Cannot determine class type for method call (variable type not found)");
    }
    
    ClassDecl* classDecl = classType->getDecl();
    if (!classDecl) {
        return logError("Class declaration not found for method call");
    }
    
    // Find the method
    MethodDecl* methodDecl = classDecl->findMethod(method);
    if (!methodDecl) {
        return logError(("Method not found: " + method).c_str());
    }
    
    // Look up the function
    std::string mangledName = classDecl->getName() + "_" + method;
    llvm::Function* func = ctx.getModule().getFunction(mangledName);
    if (!func) {
        return logError(("Method function not found: " + mangledName).c_str());
    }
    
    // Evaluate arguments with type conversion
    std::vector<llvm::Value*> argsV;
    argsV.push_back(objPtr);  // First argument is 'this'
    
    size_t idx = 1;
    for (auto& arg : args) {
        llvm::Value* argV = arg->codegen();
        if (!argV) return nullptr;
        
        llvm::Type* expected_type = func->getFunctionType()->getParamType(idx);
        llvm::Type* actual_type = argV->getType();
        
        if (expected_type != actual_type) {
            if (actual_type->isIntegerTy() && expected_type->isDoubleTy()) {
                argV = ctx.getBuilder().CreateSIToFP(argV, expected_type, "arg_conv");
            }
            else if (actual_type->isDoubleTy() && expected_type->isIntegerTy()) {
                argV = ctx.getBuilder().CreateFPToSI(argV, expected_type, "arg_conv");
            }
        }
        
        argsV.push_back(argV);
        idx++;
    }
    
    if (func->getReturnType()->isVoidTy()) {
        return ctx.getBuilder().CreateCall(func, argsV);
    } else {
        return ctx.getBuilder().CreateCall(func, argsV, "calltmp");
    }
}

// ===== New Expression =====

llvm::Value* NewExpr::codegen() {
    auto& ctx = getGlobalContext();
    
    auto classType = TypeRegistry::instance().getClassType(className);
    ClassDecl* classDecl = classType->getDecl();
    if (!classDecl) {
        return logError(("Class not found: " + className).c_str());
    }
    
    // Use aurora_object_create for proper ARC initialization
    llvm::Type* structType = classDecl->codegen();
    llvm::Value* size = llvm::ConstantInt::get(
        llvm::Type::getInt64Ty(ctx.getContext()),
        ctx.getModule().getDataLayout().getTypeAllocSize(structType));
    
    // Get or create malloc function
    llvm::Function* mallocFunc = ctx.getModule().getFunction("malloc");
    if (!mallocFunc) {
        llvm::FunctionType* mallocType = llvm::FunctionType::get(
            llvm::PointerType::get(ctx.getContext(), 0),
            {llvm::Type::getInt64Ty(ctx.getContext())},
            false);
        mallocFunc = llvm::Function::Create(mallocType, llvm::Function::ExternalLinkage,
                                           "malloc", ctx.getModule());
    }
    
    llvm::Value* objPtr = ctx.getBuilder().CreateCall(mallocFunc, {size}, "newtmp");
    
    // Initialize fields to default values
    size_t fieldIndex = 0;
    for (const auto& field : classDecl->getFields()) {
        llvm::Value* fieldPtr = ctx.getBuilder().CreateStructGEP(structType, objPtr, fieldIndex);
        if (field.initializer) {
            llvm::Value* initVal = field.initializer->codegen();
            ctx.getBuilder().CreateStore(initVal, fieldPtr);
        } else {
            llvm::Type* fieldType = field.type->toLLVMType(ctx.getContext());
            llvm::Value* zeroVal = llvm::Constant::getNullValue(fieldType);
            ctx.getBuilder().CreateStore(zeroVal, fieldPtr);
        }
        fieldIndex++;
    }
    
    // Find the correct constructor based on argument types
    std::vector<std::shared_ptr<Type>> argTypes;
    for (const auto& arg : args) {
        argTypes.push_back(arg->getType());
    }
    
    MethodDecl* constructor = classDecl->findMethod("constructor", argTypes);
    
    if (constructor) {
        // Mangle constructor name with parameter types for overloading support
        std::string mangledName = className + "_constructor";
        if (!constructor->params.empty()) {
            mangledName += "_";
            for (size_t i = 0; i < constructor->params.size(); i++) {
                if (i > 0) mangledName += "_";
                mangledName += constructor->params[i].type->getMangledName();
            }
        }
        llvm::Function* ctorFunc = ctx.getModule().getFunction(mangledName);
        if (ctorFunc) {
            std::vector<llvm::Value*> argsV;
            argsV.push_back(objPtr);  // this pointer
            
            size_t idx = 1;
            for (auto& arg : args) {
                llvm::Value* argV = arg->codegen();
                if (!argV) return nullptr;
                
                llvm::Type* expected_type = ctorFunc->getFunctionType()->getParamType(idx);
                llvm::Type* actual_type = argV->getType();
                
                if (expected_type != actual_type) {
                    if (actual_type->isIntegerTy() && expected_type->isDoubleTy()) {
                        argV = ctx.getBuilder().CreateSIToFP(argV, expected_type, "arg_conv");
                    }
                    else if (actual_type->isDoubleTy() && expected_type->isIntegerTy()) {
                        argV = ctx.getBuilder().CreateFPToSI(argV, expected_type, "arg_conv");
                    }
                }
                
                argsV.push_back(argV);
                idx++;
            }
            ctx.getBuilder().CreateCall(ctorFunc, argsV);
        }
    }
    
    return objPtr;
}

// ===== This Expression =====

llvm::Value* ThisExpr::codegen() {
    auto& ctx = getGlobalContext();
    
    auto& named_values = ctx.getNamedValues();
    llvm::AllocaInst* thisAlloca = named_values["this"];
    
    if (!thisAlloca) {
        return logError("'this' used outside of method context");
    }
    
    return ctx.getBuilder().CreateLoad(thisAlloca->getAllocatedType(), thisAlloca, "this");
}

// ===== Class Declaration =====

llvm::Type* ClassDecl::codegen() {
    auto& ctx = getGlobalContext();
    
    // Check if struct type already exists
    llvm::StructType* structType = llvm::StructType::getTypeByName(ctx.getContext(), name);
    if (structType) {
        return structType;
    }
    
    // Create struct type for the class
    std::vector<llvm::Type*> fieldTypes;
    for (const auto& field : fields) {
        fieldTypes.push_back(field.type->toLLVMType(ctx.getContext()));
    }
    
    structType = llvm::StructType::create(ctx.getContext(), fieldTypes, name);
    
    return structType;
}

void ClassDecl::codegenMethods() {
    for (auto& method : methods) {
        method.codegen(name);
    }
}

FieldDecl* ClassDecl::findField(const std::string& fieldName) {
    for (auto& field : fields) {
        if (field.name == fieldName) {
            return &field;
        }
    }
    return nullptr;
}

MethodDecl* ClassDecl::findMethod(const std::string& methodName) {
    for (auto& method : methods) {
        if (method.name == methodName) {
            return &method;
        }
    }
    return nullptr;
}

MethodDecl* ClassDecl::findMethod(const std::string& methodName, 
                                  const std::vector<std::shared_ptr<Type>>& paramTypes) {
    for (auto& method : methods) {
        if (method.name == methodName && method.params.size() == paramTypes.size()) {
            bool match = true;
            for (size_t i = 0; i < paramTypes.size(); ++i) {
                if (!method.params[i].type->equals(paramTypes[i].get())) {
                    match = false;
                    break;
                }
            }
            if (match) {
                return &method;
            }
        }
    }
    return nullptr;
}

// ===== Method Declaration =====

llvm::Function* MethodDecl::codegen(const std::string& className) {
    auto& ctx = getGlobalContext();
    
    // Mangle method name: ClassName_methodName
    // For constructors, add parameter types for overloading support
    std::string mangledName = className + "_" + name;
    
    if (isConstructor && !params.empty()) {
        mangledName += "_";
        for (size_t i = 0; i < params.size(); i++) {
            if (i > 0) mangledName += "_";
            mangledName += params[i].type->getMangledName();
        }
    }
    
    // Check if function already exists
    llvm::Function* func = ctx.getModule().getFunction(mangledName);
    if (func) {
        return func;
    }
    
    // Build parameter types: first param is 'this' pointer
    std::vector<llvm::Type*> paramTypes;
    
    auto classType = TypeRegistry::instance().getClassType(className);
    paramTypes.push_back(classType->toLLVMType(ctx.getContext()));
    
    for (const auto& param : params) {
        paramTypes.push_back(param.type->toLLVMType(ctx.getContext()));
    }
    
    // Create function type
    llvm::FunctionType* funcType = llvm::FunctionType::get(
        returnType->toLLVMType(ctx.getContext()),
        paramTypes,
        false
    );
    
    // Create function
    func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                                 mangledName, ctx.getModule());
    
    // Set parameter names
    auto argIt = func->arg_begin();
    argIt->setName("this");
    ++argIt;
    
    for (const auto& param : params) {
        argIt->setName(param.name);
        ++argIt;
    }
    
    // Create entry block
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(ctx.getContext(), "entry", func);
    ctx.getBuilder().SetInsertPoint(bb);
    
    // Set up symbol table with parameters
    ctx.getNamedValues().clear();
    
    argIt = func->arg_begin();
    // Store 'this' pointer
    llvm::AllocaInst* thisAlloca = ctx.createEntryBlockAlloca(
        func, "this", argIt->getType());
    ctx.getBuilder().CreateStore(&(*argIt), thisAlloca);
    ctx.getNamedValues()["this"] = thisAlloca;
    ++argIt;
    
    // Store other parameters
    for (const auto& param : params) {
        llvm::AllocaInst* alloca = ctx.createEntryBlockAlloca(
            func, param.name, argIt->getType());
        ctx.getBuilder().CreateStore(&(*argIt), alloca);
        ctx.getNamedValues()[param.name] = alloca;
        
        // Store parameter type information for type checking
        ctx.setVariableType(param.name, param.type);
        
        ++argIt;
    }
    
    // Generate method body
    for (auto& stmt : body) {
        stmt->codegen();
    }
    
    // Add return if method doesn't have one
    llvm::BasicBlock* currentBlock = ctx.getBuilder().GetInsertBlock();
    if (currentBlock && (currentBlock->empty() || !currentBlock->back().isTerminator())) {
        if (returnType->isVoid()) {
            ctx.getBuilder().CreateRetVoid();
        } else {
            llvm::Type* llvmRetType = returnType->toLLVMType(ctx.getContext());
            llvm::Value* defaultVal;
            if (llvmRetType->isIntegerTy(1)) {
                defaultVal = llvm::ConstantInt::getFalse(ctx.getContext());
            } else {
                defaultVal = llvm::Constant::getNullValue(llvmRetType);
            }
            ctx.getBuilder().CreateRet(defaultVal);
        }
    }
    
    // Verify the function
    if (llvm::verifyFunction(*func, &llvm::errs())) {
        func->eraseFromParent();
        return nullptr;
    }
    
    return func;
}

// Handle assignment to member fields (called from AssignStmt)
// This is integrated into the AssignStmt::codegen in StmtCodeGen.cpp
// But we provide the implementation here for member field assignments

llvm::Value* assignToMemberField(MemberAccessExpr* memberAccess, llvm::Value* val) {
    auto& ctx = getGlobalContext();
    
    // Check if object is 'this'
    if (dynamic_cast<ThisExpr*>(memberAccess->getObject())) {
        auto& named_values = ctx.getNamedValues();
        llvm::AllocaInst* thisAlloca = named_values["this"];
        if (!thisAlloca) {
            return logError("'this' not found in current context");
        }
        
        llvm::Value* thisPtr = ctx.getBuilder().CreateLoad(
            thisAlloca->getAllocatedType(), thisAlloca, "this");
        
        llvm::Function* currentFunc = ctx.getBuilder().GetInsertBlock()->getParent();
        std::string funcName = std::string(currentFunc->getName());
        
        size_t underscorePos = funcName.find('_');
        if (underscorePos == std::string::npos) {
            return logError("Cannot determine class name for member access");
        }
        std::string className = funcName.substr(0, underscorePos);
        
        auto classType = TypeRegistry::instance().getClassType(className);
        if (!classType) {
            return logError(("Class type not found: " + className).c_str());
        }
        
        ClassDecl* classDecl = classType->getDecl();
        if (!classDecl) {
            return logError(("Class declaration not found for assignment: " + className).c_str());
        }
        
        FieldDecl* field = classDecl->findField(memberAccess->getMember());
        if (!field) {
            return logError(("Field not found: " + memberAccess->getMember() + " in class " + className).c_str());
        }
        
        // Get field index
        size_t fieldIndex = 0;
        for (const auto& f : classDecl->getFields()) {
            if (f.name == memberAccess->getMember()) break;
            fieldIndex++;
        }
        
        llvm::Type* structType = classDecl->codegen();
        if (!structType) {
            return logError(("Failed to generate struct type for class: " + className).c_str());
        }
        
        llvm::Value* fieldPtr = ctx.getBuilder().CreateStructGEP(
            structType, thisPtr, fieldIndex, memberAccess->getMember().c_str());
        
        ctx.getBuilder().CreateStore(val, fieldPtr);
        return val;
    } else {
        return logError("Member assignment only supported for 'this' currently");
    }
}

} // namespace aurora

