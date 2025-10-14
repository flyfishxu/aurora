#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <memory>
#include <map>
#include <string>
#include <vector>

namespace aurora {

// Forward declaration
class Type;

/// Loop context for tracking break/continue targets
struct LoopContext {
    llvm::BasicBlock* breakTarget;    // Where break jumps to (after loop)
    llvm::BasicBlock* continueTarget; // Where continue jumps to (loop condition)
    
    LoopContext(llvm::BasicBlock* brk, llvm::BasicBlock* cont)
        : breakTarget(brk), continueTarget(cont) {}
};

/// Global code generation context
class CodeGenContext {
public:
    CodeGenContext();
    
    llvm::LLVMContext& getContext() { return *context_; }
    llvm::IRBuilder<>& getBuilder() { return *builder_; }
    llvm::Module& getModule() { return *module_; }
    
    // Symbol table for variables
    std::map<std::string, llvm::AllocaInst*>& getNamedValues() { return named_values_; }
    
    // Type table for variables (Aurora types)
    void setVariableType(const std::string& name, std::shared_ptr<Type> type) { variable_types_[name] = type; }
    std::shared_ptr<Type> getVariableType(const std::string& name) {
        auto it = variable_types_.find(name);
        return (it != variable_types_.end()) ? it->second : nullptr;
    }
    
    // Create an alloca instruction in the entry block of the function
    llvm::AllocaInst* createEntryBlockAlloca(llvm::Function* function,
                                             const std::string& var_name,
                                             llvm::Type* type);
    
    // Get or create external function declaration (like printd)
    llvm::Function* getFunction(const std::string& name);
    void setFunction(const std::string& name, llvm::Function* func);
    
    // Loop context management for break/continue
    void pushLoopContext(llvm::BasicBlock* breakTarget, llvm::BasicBlock* continueTarget);
    void popLoopContext();
    LoopContext* getCurrentLoop();
    bool isInLoop() const { return !loop_stack_.empty(); }
    
    // Memory management (ARC - Automatic Reference Counting)
    void pushScope();
    void popScope();
    void trackVariable(const std::string& name, llvm::Value* ptr);
    llvm::Value* insertRetain(llvm::Value* ptr);
    void insertRelease(llvm::Value* ptr);
    void releaseAllInScope();
    bool needsMemoryManagement(llvm::Type* type);
    
    // JIT compilation and execution
    void initializeJIT();
    int runMain();
    
    void pushFunctionReturnType(std::shared_ptr<Type> type);
    void popFunctionReturnType();
    std::shared_ptr<Type> currentFunctionReturnType() const;
    
private:
    std::unique_ptr<llvm::LLVMContext> context_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;
    std::unique_ptr<llvm::Module> module_;
    std::map<std::string, llvm::AllocaInst*> named_values_;
    std::map<std::string, std::shared_ptr<Type>> variable_types_;  // Aurora types for variables
    std::map<std::string, llvm::Function*> functions_;
    
    // Loop context stack for break/continue
    std::vector<LoopContext> loop_stack_;
    
    // Memory management: track variables that need release at scope end
    struct ScopeVariables {
        std::vector<std::pair<std::string, llvm::Value*>> variables;
    };
    std::vector<ScopeVariables> scope_stack_;
    
    // JIT engine
    std::unique_ptr<llvm::orc::LLJIT> jit_;
    
    // Main function return type (saved before JIT initialization)
    enum class MainReturnType { Int, Double, Void, Unknown };
    MainReturnType main_return_type_ = MainReturnType::Unknown;
    
    std::vector<std::shared_ptr<Type>> function_return_stack_;
};

// Global instance
CodeGenContext& getGlobalContext();

} // namespace aurora
