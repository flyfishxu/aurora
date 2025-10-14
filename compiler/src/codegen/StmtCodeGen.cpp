#include "aurora/AST.h"
#include "aurora/CodeGen.h"
#include "aurora/Diagnostic.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

namespace aurora {

// Forward declaration for member field assignment (implemented in ClassCodeGen.cpp)
llvm::Value* assignToMemberField(MemberAccessExpr* memberAccess, llvm::Value* val);

// Helper to log errors using diagnostic system
static llvm::Value* logError(const char* str, const char* code = "E3003") {
    auto& diag = getDiagnosticEngine();
    SourceLocation loc("<codegen>", 0, 0, 0);
    diag.reportError(code, str, loc);
    return nullptr;
}

// ===== Return Statement =====

llvm::Value* ReturnStmt::codegen() {
    auto& ctx = getGlobalContext();
    
    llvm::Function* currentFunc = ctx.getBuilder().GetInsertBlock()->getParent();
    llvm::Type* expected_ret_type = currentFunc->getReturnType();
    
    if (value) {
        if (auto* nullExpr = dynamic_cast<NullExpr*>(value.get())) {
            if (auto targetType = ctx.currentFunctionReturnType()) {
                if (targetType->isOptional()) {
                    nullExpr->setResolvedType(targetType);
                }
            }
        }
        
        if (expected_ret_type->isVoidTy()) {
            return logError("Cannot return a value from a void function");
        }
        
        llvm::Value* ret_val = value->codegen();
        if (!ret_val) return nullptr;
        
        llvm::Type* actual_ret_type = ret_val->getType();
        
        // Perform type conversion if needed
        if (expected_ret_type != actual_ret_type) {
            if (actual_ret_type->isIntegerTy() && expected_ret_type->isDoubleTy()) {
                ret_val = ctx.getBuilder().CreateSIToFP(ret_val, expected_ret_type, "ret_conv");
            }
            else if (actual_ret_type->isDoubleTy() && expected_ret_type->isIntegerTy()) {
                ret_val = ctx.getBuilder().CreateFPToSI(ret_val, expected_ret_type, "ret_conv");
            }
            else if (actual_ret_type->isDoubleTy() && expected_ret_type->isIntegerTy(1)) {
                ret_val = ctx.getBuilder().CreateFCmpUNE(ret_val, 
                    llvm::ConstantFP::get(ctx.getContext(), llvm::APFloat(0.0)), "tobool");
            }
            else if (actual_ret_type->isIntegerTy() && expected_ret_type->isIntegerTy(1) && 
                     !actual_ret_type->isIntegerTy(1)) {
                ret_val = ctx.getBuilder().CreateICmpNE(ret_val,
                    llvm::ConstantInt::get(actual_ret_type, 0), "tobool");
            }
            else if (actual_ret_type->isIntegerTy(1) && expected_ret_type->isDoubleTy()) {
                ret_val = ctx.getBuilder().CreateUIToFP(ret_val, expected_ret_type, "ret_conv");
            }
            else if (actual_ret_type->isIntegerTy(1) && expected_ret_type->isIntegerTy()) {
                ret_val = ctx.getBuilder().CreateZExt(ret_val, expected_ret_type, "ret_conv");
            }
        }
        
        // Release all local variables before returning
        ctx.releaseAllInScope();
        
        return ctx.getBuilder().CreateRet(ret_val);
    } else {
        if (!expected_ret_type->isVoidTy()) {
            return logError("Cannot use empty return in non-void function");
        }
        
        // Release all local variables before returning
        ctx.releaseAllInScope();
        
        return ctx.getBuilder().CreateRetVoid();
    }
}

// ===== Expression Statement =====

llvm::Value* ExprStmt::codegen() {
    return expr->codegen();
}

// ===== Variable Declaration =====

llvm::Value* VarDeclStmt::codegen() {
    auto& ctx = getGlobalContext();
    
    llvm::Function* function = ctx.getBuilder().GetInsertBlock()->getParent();
    
    if (auto* nullExpr = dynamic_cast<NullExpr*>(init.get())) {
        if (type && type->isOptional()) {
            nullExpr->setResolvedType(type);
        }
    }
    
    llvm::Value* init_val = init->codegen();
    if (!init_val) return nullptr;
    
    // For heap-allocated objects, retain the value (ownership transfer)
    // Note: new expression already returns ref_count=1, so we don't retain here
    // Only retain when assigning existing values
    
    llvm::Type* allocaType = init_val->getType();
    if (type) {
        allocaType = type->toLLVMType(ctx.getContext());
    }
    
    llvm::AllocaInst* alloca = ctx.createEntryBlockAlloca(
        function, name, allocaType);
    
    ctx.getBuilder().CreateStore(init_val, alloca);
    ctx.getNamedValues()[name] = alloca;
    ctx.setVariableType(name, type);
    
    // Track this variable for release at scope end
    // Only track arrays for now (objects don't have ARC header yet)
    auto arrayType = std::dynamic_pointer_cast<ArrayType>(type);
    if (arrayType) {
        ctx.trackVariable(name, alloca);
    }
    
    return alloca;
}

// ===== If Statement =====

llvm::Value* IfStmt::codegen() {
    auto& ctx = getGlobalContext();
    
    llvm::Value* cond_val = condition->codegen();
    if (!cond_val) return nullptr;
    
    // Convert condition to bool (i1)
    llvm::Type* cond_type = cond_val->getType();
    if (cond_type->isIntegerTy(1)) {
        // Already a boolean
    } else if (cond_type->isIntegerTy()) {
        cond_val = ctx.getBuilder().CreateICmpNE(cond_val,
            llvm::ConstantInt::get(cond_type, 0), "ifcond");
    } else if (cond_type->isDoubleTy() || cond_type->isFloatTy()) {
        cond_val = ctx.getBuilder().CreateFCmpONE(cond_val,
            llvm::ConstantFP::get(cond_type, 0.0), "ifcond");
    } else {
        return logError("Invalid condition type in if statement");
    }
    
    llvm::Function* function = ctx.getBuilder().GetInsertBlock()->getParent();
    
    llvm::BasicBlock* then_bb = llvm::BasicBlock::Create(ctx.getContext(), "then", function);
    llvm::BasicBlock* else_bb = llvm::BasicBlock::Create(ctx.getContext(), "else");
    llvm::BasicBlock* merge_bb = llvm::BasicBlock::Create(ctx.getContext(), "ifcont");
    
    ctx.getBuilder().CreateCondBr(cond_val, then_bb, else_bb);
    
    // Emit then block
    ctx.getBuilder().SetInsertPoint(then_bb);
    for (auto& stmt : thenBranch) {
        stmt->codegen();
    }
    llvm::BasicBlock* then_final_bb = ctx.getBuilder().GetInsertBlock();
    bool then_terminated = then_final_bb->getTerminator() != nullptr;
    if (!then_terminated) {
        ctx.getBuilder().CreateBr(merge_bb);
    }
    
    // Emit else block
    function->insert(function->end(), else_bb);
    ctx.getBuilder().SetInsertPoint(else_bb);
    for (auto& stmt : elseBranch) {
        stmt->codegen();
    }
    llvm::BasicBlock* else_final_bb = ctx.getBuilder().GetInsertBlock();
    bool else_terminated = else_final_bb->getTerminator() != nullptr;
    if (!else_terminated) {
        ctx.getBuilder().CreateBr(merge_bb);
    }
    
    // Only emit merge block if at least one branch needs it
    if (!then_terminated || !else_terminated) {
        function->insert(function->end(), merge_bb);
        ctx.getBuilder().SetInsertPoint(merge_bb);
        return merge_bb;
    } else {
        delete merge_bb;
        llvm::BasicBlock* unreachable_bb = llvm::BasicBlock::Create(
            ctx.getContext(), "unreachable", function);
        ctx.getBuilder().SetInsertPoint(unreachable_bb);
        return unreachable_bb;
    }
}

// ===== While Statement =====

llvm::Value* WhileStmt::codegen() {
    auto& ctx = getGlobalContext();
    
    llvm::Function* function = ctx.getBuilder().GetInsertBlock()->getParent();
    
    llvm::BasicBlock* cond_bb = llvm::BasicBlock::Create(ctx.getContext(), "whilecond", function);
    llvm::BasicBlock* body_bb = llvm::BasicBlock::Create(ctx.getContext(), "whilebody");
    llvm::BasicBlock* after_bb = llvm::BasicBlock::Create(ctx.getContext(), "afterwhile");
    
    ctx.pushLoopContext(after_bb, cond_bb);
    ctx.getBuilder().CreateBr(cond_bb);
    
    // Emit condition block
    ctx.getBuilder().SetInsertPoint(cond_bb);
    llvm::Value* cond_val = condition->codegen();
    if (!cond_val) return nullptr;
    
    cond_val = ctx.getBuilder().CreateFCmpONE(cond_val,
        llvm::ConstantFP::get(ctx.getContext(), llvm::APFloat(0.0)), "whilecond");
    ctx.getBuilder().CreateCondBr(cond_val, body_bb, after_bb);
    
    // Emit loop body
    function->insert(function->end(), body_bb);
    ctx.getBuilder().SetInsertPoint(body_bb);
    for (auto& stmt : body) {
        stmt->codegen();
    }
    llvm::BasicBlock* current_bb = ctx.getBuilder().GetInsertBlock();
    if (!current_bb->getTerminator()) {
        ctx.getBuilder().CreateBr(cond_bb);
    }
    
    ctx.popLoopContext();
    
    function->insert(function->end(), after_bb);
    ctx.getBuilder().SetInsertPoint(after_bb);
    
    return after_bb;
}

// ===== For Statement =====

llvm::Value* ForStmt::codegen() {
    auto& ctx = getGlobalContext();
    
    llvm::Function* function = ctx.getBuilder().GetInsertBlock()->getParent();
    
    llvm::Value* start_val = startExpr->codegen();
    llvm::Value* end_val = endExpr->codegen();
    if (!start_val || !end_val) return nullptr;
    
    // Type checking
    if (start_val->getType() != end_val->getType()) {
        if (start_val->getType()->isDoubleTy() && end_val->getType()->isIntegerTy()) {
            end_val = ctx.getBuilder().CreateSIToFP(end_val, start_val->getType(), "endconv");
        } else if (start_val->getType()->isIntegerTy() && end_val->getType()->isDoubleTy()) {
            start_val = ctx.getBuilder().CreateSIToFP(start_val, end_val->getType(), "startconv");
        }
    }
    
    llvm::AllocaInst* var_alloca = ctx.createEntryBlockAlloca(
        function, varName, start_val->getType());
    
    ctx.getBuilder().CreateStore(start_val, var_alloca);
    
    auto old_var = ctx.getNamedValues()[varName];
    ctx.getNamedValues()[varName] = var_alloca;
    
    llvm::BasicBlock* cond_bb = llvm::BasicBlock::Create(ctx.getContext(), "forcond", function);
    llvm::BasicBlock* body_bb = llvm::BasicBlock::Create(ctx.getContext(), "forbody");
    llvm::BasicBlock* step_bb = llvm::BasicBlock::Create(ctx.getContext(), "forstep");
    llvm::BasicBlock* after_bb = llvm::BasicBlock::Create(ctx.getContext(), "afterfor");
    
    ctx.pushLoopContext(after_bb, step_bb);
    ctx.getBuilder().CreateBr(cond_bb);
    
    // Emit condition block
    ctx.getBuilder().SetInsertPoint(cond_bb);
    llvm::Value* cur_val = ctx.getBuilder().CreateLoad(
        var_alloca->getAllocatedType(), var_alloca, varName.c_str());
    
    llvm::Value* cond_val;
    if (cur_val->getType()->isIntegerTy()) {
        cond_val = ctx.getBuilder().CreateICmpSLT(cur_val, end_val, "forcond");
    } else {
        cond_val = ctx.getBuilder().CreateFCmpULT(cur_val, end_val, "forcond");
    }
    ctx.getBuilder().CreateCondBr(cond_val, body_bb, after_bb);
    
    // Emit body block
    function->insert(function->end(), body_bb);
    ctx.getBuilder().SetInsertPoint(body_bb);
    for (auto& stmt : body) {
        stmt->codegen();
    }
    llvm::BasicBlock* current_bb = ctx.getBuilder().GetInsertBlock();
    if (!current_bb->getTerminator()) {
        ctx.getBuilder().CreateBr(step_bb);
    }
    
    // Emit step block
    function->insert(function->end(), step_bb);
    ctx.getBuilder().SetInsertPoint(step_bb);
    llvm::Value* next_val = ctx.getBuilder().CreateLoad(
        var_alloca->getAllocatedType(), var_alloca, varName.c_str());
    
    llvm::Value* step_val;
    if (stepExpr) {
        step_val = stepExpr->codegen();
        if (step_val && step_val->getType() != next_val->getType()) {
            if (next_val->getType()->isDoubleTy() && step_val->getType()->isIntegerTy()) {
                step_val = ctx.getBuilder().CreateSIToFP(step_val, next_val->getType(), "stepconv");
            } else if (next_val->getType()->isIntegerTy() && step_val->getType()->isDoubleTy()) {
                step_val = ctx.getBuilder().CreateFPToSI(step_val, next_val->getType(), "stepconv");
            }
        }
    } else {
        if (next_val->getType()->isIntegerTy()) {
            step_val = llvm::ConstantInt::get(ctx.getContext(), llvm::APInt(64, 1, true));
        } else {
            step_val = llvm::ConstantFP::get(ctx.getContext(), llvm::APFloat(1.0));
        }
    }
    
    if (next_val->getType()->isIntegerTy()) {
        next_val = ctx.getBuilder().CreateAdd(next_val, step_val, "nextvar");
    } else {
        next_val = ctx.getBuilder().CreateFAdd(next_val, step_val, "nextvar");
    }
    ctx.getBuilder().CreateStore(next_val, var_alloca);
    ctx.getBuilder().CreateBr(cond_bb);
    
    ctx.popLoopContext();
    
    function->insert(function->end(), after_bb);
    ctx.getBuilder().SetInsertPoint(after_bb);
    
    if (old_var) {
        ctx.getNamedValues()[varName] = old_var;
    } else {
        ctx.getNamedValues().erase(varName);
    }
    
    return after_bb;
}

// ===== Loop Statement =====

llvm::Value* LoopStmt::codegen() {
    auto& ctx = getGlobalContext();
    
    llvm::Function* function = ctx.getBuilder().GetInsertBlock()->getParent();
    
    llvm::BasicBlock* body_bb = llvm::BasicBlock::Create(ctx.getContext(), "loopbody", function);
    llvm::BasicBlock* after_bb = llvm::BasicBlock::Create(ctx.getContext(), "afterloop");
    
    ctx.pushLoopContext(after_bb, body_bb);
    ctx.getBuilder().CreateBr(body_bb);
    
    ctx.getBuilder().SetInsertPoint(body_bb);
    for (auto& stmt : body) {
        stmt->codegen();
    }
    
    llvm::BasicBlock* current_bb = ctx.getBuilder().GetInsertBlock();
    if (!current_bb->getTerminator()) {
        ctx.getBuilder().CreateBr(body_bb);
    }
    
    ctx.popLoopContext();
    
    function->insert(function->end(), after_bb);
    ctx.getBuilder().SetInsertPoint(after_bb);
    
    return after_bb;
}

// ===== Break Statement =====

llvm::Value* BreakStmt::codegen() {
    auto& ctx = getGlobalContext();
    
    LoopContext* loop = ctx.getCurrentLoop();
    if (!loop) {
        return logError("'break' statement must be inside a loop (while, for, or loop)");
    }
    
    return ctx.getBuilder().CreateBr(loop->breakTarget);
}

// ===== Continue Statement =====

llvm::Value* ContinueStmt::codegen() {
    auto& ctx = getGlobalContext();
    
    LoopContext* loop = ctx.getCurrentLoop();
    if (!loop) {
        return logError("'continue' statement must be inside a loop (while, for, or loop)");
    }
    
    return ctx.getBuilder().CreateBr(loop->continueTarget);
}

// ===== Assignment Statement =====

llvm::Value* AssignStmt::codegen() {
    auto& ctx = getGlobalContext();
    
    if (auto* nullExpr = dynamic_cast<NullExpr*>(value.get())) {
        if (auto* varExpr = dynamic_cast<VariableExpr*>(target.get())) {
            if (auto targetType = ctx.getVariableType(varExpr->getName())) {
                if (targetType->isOptional()) {
                    nullExpr->setResolvedType(targetType);
                }
            }
        }
    }
    
    llvm::Value* val = value->codegen();
    if (!val) return nullptr;
    
    if (auto* varExpr = dynamic_cast<VariableExpr*>(target.get())) {
        std::string name = varExpr->getName();
        auto& named_values = ctx.getNamedValues();
        llvm::AllocaInst* alloca = named_values[name];
        
        if (!alloca) {
            return logError(("Unknown variable: " + name).c_str());
        }
        
        // Memory management: release old value, retain new value
        if (ctx.needsMemoryManagement(val->getType())) {
            // Release the old value first
            ctx.insertRelease(alloca);
            
            // Retain the new value (increases ref count)
            val = ctx.insertRetain(val);
        }
        
        ctx.getBuilder().CreateStore(val, alloca);
        return val;
        
    } else if (auto* memberAccess = dynamic_cast<MemberAccessExpr*>(target.get())) {
        // Member field assignment implemented in ClassCodeGen.cpp
        return assignToMemberField(memberAccess, val);
        
    } else if (auto* arrayIndex = dynamic_cast<ArrayIndexExpr*>(target.get())) {
        // Array element assignment: arr[i] = value
        llvm::Value* arrayVal = arrayIndex->getArray()->codegen();
        llvm::Value* indexVal = arrayIndex->getIndex()->codegen();
        
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
        auto arrayType = std::dynamic_pointer_cast<ArrayType>(arrayIndex->getArray()->getType());
        if (!arrayType) {
            return logError("Array index on non-array type in assignment");
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
        
        // Store the value
        ctx.getBuilder().CreateStore(val, elemPtr);
        return val;
        
    } else {
        return logError("Invalid assignment target");
    }
}

// ===== Function Prototype =====

llvm::Function* Prototype::codegen() {
    auto& ctx = getGlobalContext();
    
    std::vector<llvm::Type*> param_types;
    for (const auto& param : params) {
        param_types.push_back(param.type->toLLVMType(ctx.getContext()));
    }
    
    llvm::Type* return_llvm_type = returnType->toLLVMType(ctx.getContext());
    
    llvm::FunctionType* func_type = llvm::FunctionType::get(
        return_llvm_type,
        param_types,
        false);
    
    llvm::Function* func = llvm::Function::Create(
        func_type,
        llvm::Function::ExternalLinkage,
        name,
        ctx.getModule());
    
    size_t idx = 0;
    for (auto& arg : func->args()) {
        arg.setName(params[idx++].name);
    }
    
    ctx.setFunction(name, func);
    return func;
}

// ===== Function Definition =====

llvm::Function* Function::codegen() {
    auto& ctx = getGlobalContext();
    
    llvm::Function* func = proto->codegen();
    if (!func) return nullptr;
    
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(ctx.getContext(), "entry", func);
    ctx.getBuilder().SetInsertPoint(bb);
    ctx.pushFunctionReturnType(proto->getReturnType());
    
    // Push a new scope for the function body
    ctx.pushScope();
    
    ctx.getNamedValues().clear();
    size_t param_idx = 0;
    for (auto& arg : func->args()) {
        llvm::AllocaInst* alloca = ctx.createEntryBlockAlloca(
            func, std::string(arg.getName()), arg.getType());
        ctx.getBuilder().CreateStore(&arg, alloca);
        ctx.getNamedValues()[std::string(arg.getName())] = alloca;
        
        // Store parameter type information for type checking
        if (param_idx < proto->getParams().size()) {
            ctx.setVariableType(std::string(arg.getName()), 
                              proto->getParams()[param_idx].type);
        }
        param_idx++;
    }
    
    for (auto& stmt : body) {
        stmt->codegen();
    }
    
    llvm::BasicBlock* currentBlock = ctx.getBuilder().GetInsertBlock();
    if (currentBlock && !currentBlock->getTerminator()) {
        // Release all variables in scope before returning
        ctx.releaseAllInScope();
        
        llvm::Type* returnType = func->getReturnType();
        if (returnType->isVoidTy()) {
            ctx.getBuilder().CreateRetVoid();
        } else {
            ctx.getBuilder().CreateUnreachable();
        }
    }
    
    // Pop scope (cleanup, but don't insert release instructions)
    ctx.popScope();
    ctx.popFunctionReturnType();
    
    if (llvm::verifyFunction(*func, &llvm::errs())) {
        func->eraseFromParent();
        return nullptr;
    }
    
    return func;
}

} // namespace aurora
