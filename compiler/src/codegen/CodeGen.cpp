#include "aurora/CodeGen.h"
#include "aurora/Logger.h"
#include "aurora_runtime.h"
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/IR/Verifier.h>
#include <iostream>

namespace aurora {

static std::unique_ptr<CodeGenContext> g_context;

CodeGenContext::CodeGenContext() {
    context_ = std::make_unique<llvm::LLVMContext>();
    module_ = std::make_unique<llvm::Module>("AuroraModule", *context_);
    builder_ = std::make_unique<llvm::IRBuilder<>>(*context_);
}

llvm::AllocaInst* CodeGenContext::createEntryBlockAlloca(
    llvm::Function* function,
    const std::string& var_name,
    llvm::Type* type) {
    
    llvm::IRBuilder<> tmp_builder(&function->getEntryBlock(),
                                  function->getEntryBlock().begin());
    return tmp_builder.CreateAlloca(type, nullptr, var_name);
}

llvm::Function* CodeGenContext::getFunction(const std::string& name) {
    // First check if function is in module
    if (auto* func = module_->getFunction(name)) {
        return func;
    }
    
    // Check our local cache
    auto it = functions_.find(name);
    if (it != functions_.end()) {
        return it->second;
    }
    
    return nullptr;
}

void CodeGenContext::setFunction(const std::string& name, llvm::Function* func) {
    functions_[name] = func;
}

void CodeGenContext::pushLoopContext(llvm::BasicBlock* breakTarget, 
                                      llvm::BasicBlock* continueTarget) {
    loop_stack_.emplace_back(breakTarget, continueTarget);
}

void CodeGenContext::popLoopContext() {
    if (!loop_stack_.empty()) {
        loop_stack_.pop_back();
    }
}

LoopContext* CodeGenContext::getCurrentLoop() {
    if (loop_stack_.empty()) {
        return nullptr;
    }
    return &loop_stack_.back();
}

// ============================================================================
// Memory Management (ARC) Implementation
// ============================================================================

void CodeGenContext::pushScope() {
    scope_stack_.emplace_back();
}

void CodeGenContext::popScope() {
    if (!scope_stack_.empty()) {
        // Note: releaseAllInScope() should be called manually before popScope
        // to control exactly where release instructions are inserted
        scope_stack_.pop_back();
    }
}

void CodeGenContext::trackVariable(const std::string& name, llvm::Value* ptr) {
    if (!scope_stack_.empty() && needsMemoryManagement(ptr->getType())) {
        scope_stack_.back().variables.push_back({name, ptr});
    }
}

llvm::Value* CodeGenContext::insertRetain(llvm::Value* ptr) {
    if (!ptr || !needsMemoryManagement(ptr->getType())) {
        return ptr;
    }
    
    // Get or create aurora_retain function
    llvm::Function* retainFunc = module_->getFunction("aurora_retain");
    if (!retainFunc) {
        llvm::FunctionType* retainType = llvm::FunctionType::get(
            llvm::PointerType::get(*context_, 0),
            {llvm::PointerType::get(*context_, 0)},
            false
        );
        retainFunc = llvm::Function::Create(
            retainType,
            llvm::Function::ExternalLinkage,
            "aurora_retain",
            *module_
        );
    }
    
    return builder_->CreateCall(retainFunc, {ptr}, "retained");
}

void CodeGenContext::insertRelease(llvm::Value* ptr) {
    if (!ptr) {
        return;
    }
    
    // Get or create aurora_release function
    llvm::Function* releaseFunc = module_->getFunction("aurora_release");
    if (!releaseFunc) {
        llvm::FunctionType* releaseType = llvm::FunctionType::get(
            llvm::Type::getVoidTy(*context_),
            {llvm::PointerType::get(*context_, 0)},
            false
        );
        releaseFunc = llvm::Function::Create(
            releaseType,
            llvm::Function::ExternalLinkage,
            "aurora_release",
            *module_
        );
    }
    
    // Load the pointer value if it's stored in an alloca
    llvm::Value* loadedPtr = ptr;
    if (llvm::isa<llvm::AllocaInst>(ptr)) {
        llvm::AllocaInst* alloca = llvm::cast<llvm::AllocaInst>(ptr);
        llvm::Type* allocatedType = alloca->getAllocatedType();
        
        // Only load if the allocated type is a pointer type
        if (!allocatedType->isPointerTy()) {
            return;  // Not a pointer type, nothing to release
        }
        
        loadedPtr = builder_->CreateLoad(
            allocatedType,
            ptr,
            "loaded_for_release"
        );
    } else if (!ptr->getType()->isPointerTy()) {
        // Direct value that's not a pointer
        return;
    }
    
    builder_->CreateCall(releaseFunc, {loadedPtr});
}

void CodeGenContext::releaseAllInScope() {
    if (scope_stack_.empty()) {
        return;
    }
    
    // Check if we have a valid insertion point
    llvm::BasicBlock* currentBlock = builder_->GetInsertBlock();
    if (!currentBlock || currentBlock->getTerminator()) {
        // Can't insert release instructions if block already has terminator
        return;
    }
    
    // Release variables in reverse order (LIFO)
    auto& vars = scope_stack_.back().variables;
    for (auto it = vars.rbegin(); it != vars.rend(); ++it) {
        insertRelease(it->second);
    }
}

bool CodeGenContext::needsMemoryManagement(llvm::Type* type) {
    // Only pointer types (objects, arrays, strings) need memory management
    return type && type->isPointerTy();
}

void CodeGenContext::pushFunctionReturnType(std::shared_ptr<Type> type) {
    function_return_stack_.push_back(std::move(type));
}

void CodeGenContext::popFunctionReturnType() {
    if (!function_return_stack_.empty()) {
        function_return_stack_.pop_back();
    }
}

std::shared_ptr<Type> CodeGenContext::currentFunctionReturnType() const {
    if (function_return_stack_.empty()) {
        return nullptr;
    }
    return function_return_stack_.back();
}

void CodeGenContext::initializeJIT() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    
    // Save main function's return type before moving the module
    if (llvm::Function* main_func = module_->getFunction("main")) {
        llvm::Type* return_type = main_func->getReturnType();
        if (return_type->isIntegerTy()) {
            main_return_type_ = MainReturnType::Int;
        } else if (return_type->isDoubleTy()) {
            main_return_type_ = MainReturnType::Double;
        } else if (return_type->isVoidTy()) {
            main_return_type_ = MainReturnType::Void;
        }
    }
    
    auto jit_expected = llvm::orc::LLJITBuilder().create();
    if (!jit_expected) {
        llvm::errs() << "Failed to create JIT: " << llvm::toString(jit_expected.takeError()) << "\n";
        return;
    }
    jit_ = std::move(*jit_expected);
    
    // Add runtime function symbols to JIT
    llvm::orc::MangleAndInterner mangle(jit_->getExecutionSession(), jit_->getDataLayout());
    llvm::orc::SymbolMap runtime_symbols;
    
    // Add array runtime functions
    runtime_symbols[mangle("aurora_array_create")] = 
        {llvm::orc::ExecutorAddr::fromPtr(&aurora_array_create), llvm::JITSymbolFlags::Exported};
    runtime_symbols[mangle("aurora_array_free")] = 
        {llvm::orc::ExecutorAddr::fromPtr(&aurora_array_free), llvm::JITSymbolFlags::Exported};
    runtime_symbols[mangle("aurora_array_length")] = 
        {llvm::orc::ExecutorAddr::fromPtr(&aurora_array_length), llvm::JITSymbolFlags::Exported};
    runtime_symbols[mangle("aurora_array_get_ptr")] = 
        {llvm::orc::ExecutorAddr::fromPtr(&aurora_array_get_ptr), llvm::JITSymbolFlags::Exported};
    runtime_symbols[mangle("aurora_array_set")] = 
        {llvm::orc::ExecutorAddr::fromPtr(&aurora_array_set), llvm::JITSymbolFlags::Exported};
    runtime_symbols[mangle("aurora_array_bounds_check")] = 
        {llvm::orc::ExecutorAddr::fromPtr(&aurora_array_bounds_check), llvm::JITSymbolFlags::Exported};
    
    // Add object runtime functions
    runtime_symbols[mangle("aurora_object_create")] = 
        {llvm::orc::ExecutorAddr::fromPtr(&aurora_object_create), llvm::JITSymbolFlags::Exported};
    runtime_symbols[mangle("aurora_object_free")] = 
        {llvm::orc::ExecutorAddr::fromPtr(&aurora_object_free), llvm::JITSymbolFlags::Exported};
    
    // Add string runtime functions
    runtime_symbols[mangle("aurora_string_create")] = 
        {llvm::orc::ExecutorAddr::fromPtr(&aurora_string_create), llvm::JITSymbolFlags::Exported};
    runtime_symbols[mangle("aurora_string_free")] = 
        {llvm::orc::ExecutorAddr::fromPtr(&aurora_string_free), llvm::JITSymbolFlags::Exported};
    runtime_symbols[mangle("aurora_string_length")] = 
        {llvm::orc::ExecutorAddr::fromPtr(&aurora_string_length), llvm::JITSymbolFlags::Exported};
    
    // Add debug functions
    runtime_symbols[mangle("aurora_assert")] = 
        {llvm::orc::ExecutorAddr::fromPtr(&aurora_assert), llvm::JITSymbolFlags::Exported};
    runtime_symbols[mangle("aurora_panic")] = 
        {llvm::orc::ExecutorAddr::fromPtr(&aurora_panic), llvm::JITSymbolFlags::Exported};
    
    // Define symbols in the JIT
    auto& main_jd = jit_->getMainJITDylib();
    if (auto err = main_jd.define(llvm::orc::absoluteSymbols(runtime_symbols))) {
        llvm::errs() << "Failed to add runtime symbols: " << llvm::toString(std::move(err)) << "\n";
    }
    
    // Add the module
    auto tsm = llvm::orc::ThreadSafeModule(std::move(module_), std::move(context_));
    if (auto err = jit_->addIRModule(std::move(tsm))) {
        llvm::errs() << "Failed to add module: " << llvm::toString(std::move(err)) << "\n";
    }
}

int CodeGenContext::runMain() {
    auto& logger = Logger::instance();
    
    if (!jit_) {
        logger.error("JIT not initialized");
        return -1;
    }
    
    // Look up the main function
    auto main_sym = jit_->lookup("main");
    if (!main_sym) {
        logger.error("Could not find main function: " + llvm::toString(main_sym.takeError()));
        return -1;
    }
    
    int exit_code = 0;
    
    // Use the saved return type to call main correctly
    switch (main_return_type_) {
        case MainReturnType::Int: {
            // main returns int
            auto* main_fn = main_sym->toPtr<int()>();
            exit_code = main_fn();
            break;
        }
        case MainReturnType::Double: {
            // main returns double (legacy)
            auto* main_fn = main_sym->toPtr<double()>();
            double result = main_fn();
            exit_code = static_cast<int>(result);
            break;
        }
        case MainReturnType::Void: {
            // main returns void
            auto* main_fn = main_sym->toPtr<void()>();
            main_fn();
            exit_code = 0;
            break;
        }
        default:
            logger.error("Unknown or unsupported main function return type");
            return -1;
    }
    
    // Clean up JIT before returning to avoid destructor issues
    jit_.reset();
    
    return exit_code;
}

CodeGenContext& getGlobalContext() {
    if (!g_context) {
        g_context = std::make_unique<CodeGenContext>();
    }
    return *g_context;
}

} // namespace aurora
