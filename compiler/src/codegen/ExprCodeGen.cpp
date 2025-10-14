#include "aurora/AST.h"
#include "aurora/CodeGen.h"
#include "aurora/Diagnostic.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>

namespace aurora {

// Helper to log errors using diagnostic system
static llvm::Value* logError(const char* str, const char* code = "E3002") {
    auto& diag = getDiagnosticEngine();
    SourceLocation loc("<codegen>", 0, 0, 0);
    diag.reportError(code, str, loc);
    return nullptr;
}

// Helper function to convert any value to boolean (i1)
static llvm::Value* convertToBool(llvm::Value* val, CodeGenContext& ctx) {
    if (val->getType()->isIntegerTy(1)) {
        return val;
    } else if (val->getType()->isIntegerTy()) {
        return ctx.getBuilder().CreateICmpNE(val, 
            llvm::ConstantInt::get(val->getType(), 0), "tobool");
    } else {
        return ctx.getBuilder().CreateFCmpUNE(val, 
            llvm::ConstantFP::get(ctx.getContext(), llvm::APFloat(0.0)), "tobool");
    }
}

// ===== Integer Literal =====

llvm::Value* IntLiteralExpr::codegen() {
    auto& ctx = getGlobalContext();
    return llvm::ConstantInt::get(ctx.getContext(), llvm::APInt(64, value, true));
}

std::shared_ptr<Type> IntLiteralExpr::getType() const {
    return TypeRegistry::instance().getIntType();
}

// ===== Double Literal =====

llvm::Value* DoubleLiteralExpr::codegen() {
    auto& ctx = getGlobalContext();
    return llvm::ConstantFP::get(ctx.getContext(), llvm::APFloat(value));
}

std::shared_ptr<Type> DoubleLiteralExpr::getType() const {
    return TypeRegistry::instance().getDoubleType();
}

// ===== Boolean Literal =====

llvm::Value* BoolExpr::codegen() {
    auto& ctx = getGlobalContext();
    // Generate proper boolean (i1) value
    return llvm::ConstantInt::get(llvm::Type::getInt1Ty(ctx.getContext()), value ? 1 : 0);
}

std::shared_ptr<Type> BoolExpr::getType() const {
    return TypeRegistry::instance().getBoolType();
}

// ===== Null Literal =====

NullExpr::NullExpr() {
    resolvedType = TypeRegistry::instance().getOptionalType(
        TypeRegistry::instance().getVoidType()
    );
}

void NullExpr::setResolvedType(std::shared_ptr<Type> t) {
    if (t) {
        resolvedType = std::move(t);
    }
}

llvm::Value* NullExpr::codegen() {
    auto& ctx = getGlobalContext();
    
    if (auto optionalType = std::dynamic_pointer_cast<OptionalType>(resolvedType)) {
        llvm::Type* llvmOptionalType = optionalType->toLLVMType(ctx.getContext());
        return llvm::Constant::getNullValue(llvmOptionalType);
    }
    
    // Fallback: treat null as a generic pointer
    return llvm::ConstantPointerNull::get(llvm::PointerType::get(ctx.getContext(), 0));
}

std::shared_ptr<Type> NullExpr::getType() const {
    return resolvedType;
}

// ===== String Literal =====

llvm::Value* StringExpr::codegen() {
    auto& ctx = getGlobalContext();
    
    llvm::Constant* strConstant = llvm::ConstantDataArray::getString(
        ctx.getContext(), value, true);
    
    auto* globalStr = new llvm::GlobalVariable(
        ctx.getModule(),
        strConstant->getType(),
        true,
        llvm::GlobalValue::PrivateLinkage,
        strConstant,
        ".str"
    );
    
    std::vector<llvm::Value*> indices = {
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx.getContext()), 0),
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx.getContext()), 0)
    };
    
    return ctx.getBuilder().CreateInBoundsGEP(
        strConstant->getType(),
        globalStr,
        indices,
        "str"
    );
}

std::shared_ptr<Type> StringExpr::getType() const {
    return TypeRegistry::instance().getStringType();
}

// ===== Variable Expression =====

std::shared_ptr<Type> VariableExpr::getType() const {
    auto& ctx = getGlobalContext();
    auto varType = ctx.getVariableType(name);
    if (varType) {
        type = varType;
    }
    return type;
}

llvm::Value* VariableExpr::codegen() {
    auto& ctx = getGlobalContext();
    
    auto& named_values = ctx.getNamedValues();
    llvm::AllocaInst* alloca = named_values[name];
    
    if (!alloca) {
        // Check if it's a function parameter (legacy behavior)
        llvm::Function* current_fn = ctx.getBuilder().GetInsertBlock()->getParent();
        for (auto& arg : current_fn->args()) {
            if (arg.getName() == name) {
                return &arg;
            }
        }
        return logError(("Unknown variable: " + name).c_str());
    }
    
    return ctx.getBuilder().CreateLoad(alloca->getAllocatedType(), alloca, name.c_str());
}

// ===== Binary Expression =====

llvm::Value* BinaryExpr::codegen() {
    auto& ctx = getGlobalContext();
    
    // Special handling for logical operators with short-circuit evaluation
    if (op == Op::And || op == Op::Or) {
        llvm::Value* L = left->codegen();
        if (!L) return nullptr;
        
        llvm::Value* L_bool = convertToBool(L, ctx);
        
        llvm::Function* func = ctx.getBuilder().GetInsertBlock()->getParent();
        llvm::BasicBlock* lhsBlock = ctx.getBuilder().GetInsertBlock();
        llvm::BasicBlock* rhsBlock = llvm::BasicBlock::Create(ctx.getContext(), "rhs", func);
        llvm::BasicBlock* mergeBlock = llvm::BasicBlock::Create(ctx.getContext(), "merge", func);
        
        if (op == Op::And) {
            ctx.getBuilder().CreateCondBr(L_bool, rhsBlock, mergeBlock);
        } else {
            ctx.getBuilder().CreateCondBr(L_bool, mergeBlock, rhsBlock);
        }
        
        ctx.getBuilder().SetInsertPoint(rhsBlock);
        llvm::Value* R = right->codegen();
        if (!R) return nullptr;
        llvm::Value* R_bool = convertToBool(R, ctx);
        rhsBlock = ctx.getBuilder().GetInsertBlock();
        ctx.getBuilder().CreateBr(mergeBlock);
        
        ctx.getBuilder().SetInsertPoint(mergeBlock);
        llvm::PHINode* phi = ctx.getBuilder().CreatePHI(llvm::Type::getInt1Ty(ctx.getContext()), 2, "logictmp");
        
        if (op == Op::And) {
            phi->addIncoming(llvm::ConstantInt::get(llvm::Type::getInt1Ty(ctx.getContext()), 0), 
                            lhsBlock);
            phi->addIncoming(R_bool, rhsBlock);
        } else {
            phi->addIncoming(llvm::ConstantInt::get(llvm::Type::getInt1Ty(ctx.getContext()), 1), 
                            lhsBlock);
            phi->addIncoming(R_bool, rhsBlock);
        }
        
        return phi;
    }
    
    // Standard evaluation for all other operators
    llvm::Value* L = left->codegen();
    llvm::Value* R = right->codegen();
    
    if (!L || !R) return nullptr;
    
    // Check for pointer types (null comparisons)
    bool left_is_ptr = L->getType()->isPointerTy();
    bool right_is_ptr = R->getType()->isPointerTy();
    
    auto leftOptional = left ? std::dynamic_pointer_cast<OptionalType>(left->getType()) : nullptr;
    auto rightOptional = right ? std::dynamic_pointer_cast<OptionalType>(right->getType()) : nullptr;
    
    auto isNullConstant = [](llvm::Value* value) {
        if (auto* constant = llvm::dyn_cast<llvm::Constant>(value)) {
            return constant->isNullValue();
        }
        return false;
    };
    
    if (leftOptional || rightOptional) {
        if (op != Op::Equal && op != Op::NotEqual) {
            return logError("Optional values only support == or != comparisons");
        }
        
        llvm::Value* optionalValue = nullptr;
        bool checkingLeft = false;
        
        if (leftOptional && isNullConstant(R)) {
            optionalValue = L;
            checkingLeft = true;
        } else if (rightOptional && isNullConstant(L)) {
            optionalValue = R;
            checkingLeft = false;
        } else {
            return logError("Optional comparisons currently support only comparisons against null");
        }
        
        llvm::Value* hasValue = ctx.getBuilder().CreateExtractValue(optionalValue, 0, "has_value");
        llvm::Value* cmp = ctx.getBuilder().CreateICmpEQ(
            hasValue,
            llvm::ConstantInt::getFalse(ctx.getContext()),
            checkingLeft ? "opt_is_null" : "opt_is_null_rhs"
        );
        
        if (op == Op::NotEqual) {
            cmp = ctx.getBuilder().CreateNot(cmp, "opt_not");
        }
        return cmp;
    }
    
    if (left_is_ptr || right_is_ptr) {
        // Handle pointer comparisons (including null)
        if (left_is_ptr && right_is_ptr) {
            // Both are pointers - use pointer comparison
            switch (op) {
                case Op::Equal:
                    L = ctx.getBuilder().CreateICmpEQ(L, R, "cmptmp");
                    break;
                case Op::NotEqual:
                    L = ctx.getBuilder().CreateICmpNE(L, R, "cmptmp");
                    break;
                default:
                    return logError("Only equality operators supported for pointer types");
            }
            return L;
        } else {
            return logError("Cannot compare pointer with non-pointer type");
        }
    }
    
    // Type promotion for non-pointer types
    bool left_is_int = L->getType()->isIntegerTy();
    bool right_is_int = R->getType()->isIntegerTy();
    
    if (left_is_int && !right_is_int) {
        L = ctx.getBuilder().CreateSIToFP(L, llvm::Type::getDoubleTy(ctx.getContext()), "promotetmp");
        left_is_int = false;
    } else if (!left_is_int && right_is_int) {
        R = ctx.getBuilder().CreateSIToFP(R, llvm::Type::getDoubleTy(ctx.getContext()), "promotetmp");
        right_is_int = false;
    }
    
    bool is_int = left_is_int && right_is_int;
    
    switch (op) {
        case Op::Add:
            return is_int ? ctx.getBuilder().CreateAdd(L, R, "addtmp") 
                         : ctx.getBuilder().CreateFAdd(L, R, "addtmp");
        case Op::Sub:
            return is_int ? ctx.getBuilder().CreateSub(L, R, "subtmp")
                         : ctx.getBuilder().CreateFSub(L, R, "subtmp");
        case Op::Mul:
            return is_int ? ctx.getBuilder().CreateMul(L, R, "multmp")
                         : ctx.getBuilder().CreateFMul(L, R, "multmp");
        case Op::Div:
            return is_int ? ctx.getBuilder().CreateSDiv(L, R, "divtmp")
                         : ctx.getBuilder().CreateFDiv(L, R, "divtmp");
        case Op::Mod:
            return is_int ? ctx.getBuilder().CreateSRem(L, R, "modtmp")
                         : ctx.getBuilder().CreateFRem(L, R, "modtmp");
        case Op::Less:
            if (is_int) {
                return ctx.getBuilder().CreateICmpSLT(L, R, "cmptmp");
            } else {
                return ctx.getBuilder().CreateFCmpULT(L, R, "cmptmp");
            }
        case Op::Greater:
            if (is_int) {
                return ctx.getBuilder().CreateICmpSGT(L, R, "cmptmp");
            } else {
                return ctx.getBuilder().CreateFCmpUGT(L, R, "cmptmp");
            }
        case Op::LessEq:
            if (is_int) {
                return ctx.getBuilder().CreateICmpSLE(L, R, "cmptmp");
            } else {
                return ctx.getBuilder().CreateFCmpULE(L, R, "cmptmp");
            }
        case Op::GreaterEq:
            if (is_int) {
                return ctx.getBuilder().CreateICmpSGE(L, R, "cmptmp");
            } else {
                return ctx.getBuilder().CreateFCmpUGE(L, R, "cmptmp");
            }
        case Op::Equal:
            if (is_int) {
                return ctx.getBuilder().CreateICmpEQ(L, R, "cmptmp");
            } else {
                return ctx.getBuilder().CreateFCmpUEQ(L, R, "cmptmp");
            }
        case Op::NotEqual:
            if (is_int) {
                return ctx.getBuilder().CreateICmpNE(L, R, "cmptmp");
            } else {
                return ctx.getBuilder().CreateFCmpUNE(L, R, "cmptmp");
            }
        case Op::And:
        case Op::Or:
            return logError("Logical operators should be handled earlier");
        case Op::BitwiseAnd:
            if (!is_int) {
                return logError("Bitwise AND requires integer operands");
            }
            return ctx.getBuilder().CreateAnd(L, R, "andtmp");
        case Op::BitwiseOr:
            if (!is_int) {
                return logError("Bitwise OR requires integer operands");
            }
            return ctx.getBuilder().CreateOr(L, R, "ortmp");
        case Op::BitwiseXor:
            if (!is_int) {
                return logError("Bitwise XOR requires integer operands");
            }
            return ctx.getBuilder().CreateXor(L, R, "xortmp");
        case Op::LeftShift:
            if (!is_int) {
                return logError("Left shift requires integer operands");
            }
            return ctx.getBuilder().CreateShl(L, R, "shltmp");
        case Op::RightShift:
            if (!is_int) {
                return logError("Right shift requires integer operands");
            }
            return ctx.getBuilder().CreateAShr(L, R, "ashrtmp");
        default:
            return logError("Invalid binary operator");
    }
}

std::shared_ptr<Type> BinaryExpr::getType() const {
    return TypeRegistry::instance().getDoubleType();
}

// ===== Unary Expression =====

llvm::Value* UnaryExpr::codegen() {
    auto& ctx = getGlobalContext();
    
    llvm::Value* operand = expr->codegen();
    if (!operand) return nullptr;
    
    switch (op) {
        case Op::Not: {
            llvm::Value* boolVal = convertToBool(operand, ctx);
            return ctx.getBuilder().CreateNot(boolVal, "nottmp");
        }
        case Op::Neg:
            if (operand->getType()->isIntegerTy()) {
                return ctx.getBuilder().CreateNeg(operand, "negtmp");
            } else {
                return ctx.getBuilder().CreateFNeg(operand, "negtmp");
            }
        case Op::BitwiseNot:
            if (!operand->getType()->isIntegerTy()) {
                return logError("Bitwise NOT requires integer operand");
            }
            return ctx.getBuilder().CreateNot(operand, "bitnottmp");
        default:
            return logError("Invalid unary operator");
    }
}

std::shared_ptr<Type> UnaryExpr::getType() const {
    return expr->getType();
}

// ===== Call Expression =====

llvm::Value* CallExpr::codegen() {
    auto& ctx = getGlobalContext();
    
    llvm::Function* callee_func = ctx.getFunction(callee);
    if (!callee_func) {
        return logError(("Unknown function: " + callee).c_str());
    }
    
    if (callee_func->arg_size() != args.size()) {
        return logError("Incorrect number of arguments");
    }
    
    std::vector<llvm::Value*> arg_values;
    for (size_t idx = 0; idx < args.size(); ++idx) {
        llvm::Type* expected_type = callee_func->getFunctionType()->getParamType(idx);
        
        if (auto* nullExpr = dynamic_cast<NullExpr*>(args[idx].get())) {
            // Use the expected LLVM type to materialise a null constant
            arg_values.push_back(llvm::Constant::getNullValue(expected_type));
            continue;
        }
        
        llvm::Value* arg_val = args[idx]->codegen();
        if (!arg_val) return nullptr;
        
        llvm::Type* actual_type = arg_val->getType();
        
        if (expected_type != actual_type) {
            if (actual_type->isIntegerTy() && expected_type->isDoubleTy()) {
                arg_val = ctx.getBuilder().CreateSIToFP(arg_val, expected_type, "arg_conv");
            }
            else if (actual_type->isDoubleTy() && expected_type->isIntegerTy()) {
                arg_val = ctx.getBuilder().CreateFPToSI(arg_val, expected_type, "arg_conv");
            }
        }
        
        arg_values.push_back(arg_val);
    }
    
    if (callee_func->getReturnType()->isVoidTy()) {
        return ctx.getBuilder().CreateCall(callee_func, arg_values);
    } else {
        return ctx.getBuilder().CreateCall(callee_func, arg_values, "calltmp");
    }
}

// ===== Null Check Expression =====

llvm::Value* NullCheckExpr::codegen() {
    auto& ctx = getGlobalContext();
    
    llvm::Value* value = expr->codegen();
    if (!value) return nullptr;
    
    auto exprType = expr->getType();
    if (auto optionalType = std::dynamic_pointer_cast<OptionalType>(exprType)) {
        llvm::Value* hasValue = ctx.getBuilder().CreateExtractValue(value, 0, "has_value");
        return ctx.getBuilder().CreateICmpNE(
            hasValue,
            llvm::ConstantInt::getFalse(ctx.getContext()),
            "nullcheck"
        );
    }
    
    if (value->getType()->isPointerTy()) {
        llvm::Value* null_val = llvm::ConstantPointerNull::get(
            llvm::PointerType::get(ctx.getContext(), 0));
        return ctx.getBuilder().CreateICmpNE(value, null_val, "nullcheck");
    }
    
    return logError("Null check requires optional or pointer type");
}

std::shared_ptr<Type> NullCheckExpr::getType() const {
    return TypeRegistry::instance().getBoolType();
}

// ===== Ternary Expression =====

llvm::Value* TernaryExpr::codegen() {
    auto& ctx = getGlobalContext();
    
    llvm::Value* condVal = condition->codegen();
    if (!condVal) return nullptr;
    
    condVal = convertToBool(condVal, ctx);
    
    llvm::Function* func = ctx.getBuilder().GetInsertBlock()->getParent();
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(ctx.getContext(), "ternary_then", func);
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(ctx.getContext(), "ternary_else");
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(ctx.getContext(), "ternary_merge");
    
    ctx.getBuilder().CreateCondBr(condVal, thenBB, elseBB);
    
    ctx.getBuilder().SetInsertPoint(thenBB);
    llvm::Value* thenVal = trueExpr->codegen();
    if (!thenVal) return nullptr;
    ctx.getBuilder().CreateBr(mergeBB);
    thenBB = ctx.getBuilder().GetInsertBlock();
    
    func->insert(func->end(), elseBB);
    ctx.getBuilder().SetInsertPoint(elseBB);
    llvm::Value* elseVal = falseExpr->codegen();
    if (!elseVal) return nullptr;
    ctx.getBuilder().CreateBr(mergeBB);
    elseBB = ctx.getBuilder().GetInsertBlock();
    
    func->insert(func->end(), mergeBB);
    ctx.getBuilder().SetInsertPoint(mergeBB);
    
    if (thenVal->getType() != elseVal->getType()) {
        if (thenVal->getType()->isIntegerTy() && elseVal->getType()->isDoubleTy()) {
            thenVal = ctx.getBuilder().CreateSIToFP(thenVal, elseVal->getType());
        } else if (thenVal->getType()->isDoubleTy() && elseVal->getType()->isIntegerTy()) {
            elseVal = ctx.getBuilder().CreateSIToFP(elseVal, thenVal->getType());
        }
    }
    
    llvm::PHINode* phi = ctx.getBuilder().CreatePHI(thenVal->getType(), 2, "ternary_result");
    phi->addIncoming(thenVal, thenBB);
    phi->addIncoming(elseVal, elseBB);
    
    return phi;
}

std::shared_ptr<Type> TernaryExpr::getType() const {
    return trueExpr->getType();
}

} // namespace aurora
