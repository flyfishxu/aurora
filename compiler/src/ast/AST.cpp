#include "aurora/AST.h"
#include "aurora/CodeGen.h"

// This file contains AST infrastructure code.
// Code generation implementations have been moved to modular files:
// - ExprCodeGen.cpp: Expression code generation (literals, binary/unary ops, calls)
// - StmtCodeGen.cpp: Statement code generation (if/while/for/return/assign)
// - ClassCodeGen.cpp: OOP-related code generation (classes, methods, members)
// - ArrayCodeGen.cpp: Array-related code generation (array literals, indexing)
// - ModuleLoader.cpp: Import/module loading implementation

namespace aurora {

// ImportDecl::load() is implemented in ModuleLoader.cpp

std::shared_ptr<Type> BinaryExpr::getType() const {
    auto& registry = TypeRegistry::instance();
    
    auto leftType = left ? left->getType() : nullptr;
    auto rightType = right ? right->getType() : nullptr;
    
    auto isDouble = [](const std::shared_ptr<Type>& type) {
        return type && type->isDouble();
    };
    
    auto isInt = [](const std::shared_ptr<Type>& type) {
        return type && type->isInt();
    };
    
    switch (op) {
        case Op::Add:
        case Op::Sub:
        case Op::Mul:
        case Op::Div:
        case Op::Mod:
            if (isDouble(leftType) || isDouble(rightType)) {
                return registry.getDoubleType();
            }
            if (isInt(leftType) && isInt(rightType)) {
                return registry.getIntType();
            }
            return registry.getDoubleType();
        case Op::Less:
        case Op::Greater:
        case Op::LessEq:
        case Op::GreaterEq:
        case Op::Equal:
        case Op::NotEqual:
        case Op::And:
        case Op::Or:
            return registry.getBoolType();
        case Op::BitwiseAnd:
        case Op::BitwiseOr:
        case Op::BitwiseXor:
        case Op::LeftShift:
        case Op::RightShift:
            return registry.getIntType();
        case Op::NullCoalesce:
            return leftType ? leftType : rightType;
        default:
            return registry.getDoubleType();
    }
}

std::shared_ptr<Type> UnaryExpr::getType() const {
    auto& registry = TypeRegistry::instance();
    auto operandType = expr ? expr->getType() : nullptr;
    
    switch (op) {
        case UnaryExpr::Op::Not:
            return registry.getBoolType();
        case UnaryExpr::Op::BitwiseNot:
            return registry.getIntType();
        case UnaryExpr::Op::Neg:
            if (operandType && operandType->isDouble()) {
                return registry.getDoubleType();
            }
            if (operandType && operandType->isInt()) {
                return registry.getIntType();
            }
            return registry.getDoubleType();
        default:
            return operandType;
    }
}

} // namespace aurora
