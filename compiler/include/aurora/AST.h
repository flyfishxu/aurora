#pragma once

#include "aurora/Type.h"
#include <memory>
#include <string>
#include <vector>

namespace llvm {
class Value;
class Function;
}

namespace aurora {

/// Base class for all AST nodes
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual llvm::Value* codegen() = 0;
};

/// Base class for all expression nodes
class Expr : public ASTNode {
public:
    virtual ~Expr() = default;
    virtual std::shared_ptr<Type> getType() const = 0;
};

/// Integer literal (e.g., 42)
class IntLiteralExpr : public Expr {
public:
    explicit IntLiteralExpr(int64_t val) : value(val) {}
    
    llvm::Value* codegen() override;
    std::shared_ptr<Type> getType() const override;
    int64_t getValue() const { return value; }

private:
    int64_t value;
};

/// Double literal (e.g., 3.14)
class DoubleLiteralExpr : public Expr {
public:
    explicit DoubleLiteralExpr(double val) : value(val) {}
    
    llvm::Value* codegen() override;
    std::shared_ptr<Type> getType() const override;
    double getValue() const { return value; }

private:
    double value;
};

// Boolean, String, and Null literals are defined later in the file
// (BoolExpr, StringExpr, NullExpr)

/// Variable reference (e.g., x, myVar)
class VariableExpr : public Expr {
public:
    VariableExpr(std::string n, std::shared_ptr<Type> t)
        : name(std::move(n)), type(std::move(t)) {}
    
    llvm::Value* codegen() override;
    std::shared_ptr<Type> getType() const override;
    const std::string& getName() const { return name; }

private:
    std::string name;
    mutable std::shared_ptr<Type> type;  // mutable for lazy type resolution
};

/// Boolean literal (true, false)
class BoolExpr : public Expr {
public:
    explicit BoolExpr(bool v) : value(v) {}
    
    llvm::Value* codegen() override;
    std::shared_ptr<Type> getType() const override;
    bool getValue() const { return value; }

private:
    bool value;
};

/// Null literal
class NullExpr : public Expr {
public:
    NullExpr();
    
    llvm::Value* codegen() override;
    std::shared_ptr<Type> getType() const override;
    void setResolvedType(std::shared_ptr<Type> t);

private:
    std::shared_ptr<Type> resolvedType;
};

/// String literal (e.g., "hello")
class StringExpr : public Expr {
public:
    explicit StringExpr(std::string v) : value(std::move(v)) {}
    
    llvm::Value* codegen() override;
    std::shared_ptr<Type> getType() const override;
    const std::string& getValue() const { return value; }

private:
    std::string value;
};

/// Binary operation (e.g., a + b, x * y)
class BinaryExpr : public Expr {
public:
    enum class Op {
        // Arithmetic
        Add, Sub, Mul, Div, Mod,
        // Comparison
        Less, Greater, LessEq, GreaterEq, Equal, NotEqual,
        // Logical
        And, Or,
        // Bitwise
        BitwiseAnd, BitwiseOr, BitwiseXor, LeftShift, RightShift,
        // Nullable
        NullCoalesce  // ??
    };
    
    BinaryExpr(Op o, std::unique_ptr<Expr> l, std::unique_ptr<Expr> r)
        : op(o), left(std::move(l)), right(std::move(r)) {}
    
    llvm::Value* codegen() override;
    std::shared_ptr<Type> getType() const override;
    
    Op getOp() const { return op; }
    Expr* getLeft() const { return left.get(); }
    Expr* getRight() const { return right.get(); }

private:
    Op op;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
};

/// Unary operation (e.g., !x, -y, ~x)
class UnaryExpr : public Expr {
public:
    enum class Op { 
        Not,        // !x (logical not)
        Neg,        // -x (numeric negation)
        BitwiseNot  // ~x (bitwise not)
    };
    
    UnaryExpr(Op o, std::unique_ptr<Expr> e)
        : op(o), expr(std::move(e)) {}
    
    llvm::Value* codegen() override;
    std::shared_ptr<Type> getType() const override;

private:
    Op op;
    std::unique_ptr<Expr> expr;
};

/// Array literal expression (e.g., [1, 2, 3, 4])
class ArrayLiteralExpr : public Expr {
public:
    ArrayLiteralExpr(std::vector<std::unique_ptr<Expr>> elems, std::shared_ptr<Type> t)
        : elements(std::move(elems)), type(std::move(t)) {}
    
    llvm::Value* codegen() override;
    std::shared_ptr<Type> getType() const override { return type; }
    
    const std::vector<std::unique_ptr<Expr>>& getElements() const { return elements; }

private:
    std::vector<std::unique_ptr<Expr>> elements;
    std::shared_ptr<Type> type;
};

/// Array index expression (e.g., arr[0], matrix[i][j])
class ArrayIndexExpr : public Expr {
public:
    ArrayIndexExpr(std::unique_ptr<Expr> arr, std::unique_ptr<Expr> idx)
        : array(std::move(arr)), index(std::move(idx)) {}
    
    llvm::Value* codegen() override;
    std::shared_ptr<Type> getType() const override;
    
    Expr* getArray() const { return array.get(); }
    Expr* getIndex() const { return index.get(); }

private:
    std::unique_ptr<Expr> array;
    std::unique_ptr<Expr> index;
};

/// Ternary conditional expression (e.g., cond ? true_val : false_val)
class TernaryExpr : public Expr {
public:
    TernaryExpr(std::unique_ptr<Expr> c, std::unique_ptr<Expr> t, std::unique_ptr<Expr> f)
        : condition(std::move(c)), trueExpr(std::move(t)), falseExpr(std::move(f)) {}
    
    llvm::Value* codegen() override;
    std::shared_ptr<Type> getType() const override;

private:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> trueExpr;
    std::unique_ptr<Expr> falseExpr;
};

/// Safe navigation (e.g., obj?.field)
class SafeNavigationExpr : public Expr {
public:
    SafeNavigationExpr(std::unique_ptr<Expr> o, std::string m)
        : object(std::move(o)), member(std::move(m)) {}
    
    llvm::Value* codegen() override;
    std::shared_ptr<Type> getType() const override;
    
    const std::string& getMember() const { return member; }

private:
    std::unique_ptr<Expr> object;
    std::string member;
};

/// Force unwrap (e.g., optional!)
class ForceUnwrapExpr : public Expr {
public:
    explicit ForceUnwrapExpr(std::unique_ptr<Expr> e)
        : expr(std::move(e)) {}
    
    llvm::Value* codegen() override;
    std::shared_ptr<Type> getType() const override;

private:
    std::unique_ptr<Expr> expr;
};

/// Function call (e.g., foo(a, b))
class CallExpr : public Expr {
public:
    CallExpr(std::string c, std::vector<std::unique_ptr<Expr>> a, std::shared_ptr<Type> t)
        : callee(std::move(c)), args(std::move(a)), type(std::move(t)) {}
    
    llvm::Value* codegen() override;
    std::shared_ptr<Type> getType() const override { return type; }
    
    const std::string& getCallee() const { return callee; }
    const std::vector<std::unique_ptr<Expr>>& getArgs() const { return args; }

private:
    std::string callee;
    std::vector<std::unique_ptr<Expr>> args;
    std::shared_ptr<Type> type;
};

/// Null check (e.g., x?)
class NullCheckExpr : public Expr {
public:
    explicit NullCheckExpr(std::unique_ptr<Expr> e)
        : expr(std::move(e)) {}
    
    llvm::Value* codegen() override;
    std::shared_ptr<Type> getType() const override;

private:
    std::unique_ptr<Expr> expr;
};

/// Member access expression (e.g., obj.field or obj.method())
class MemberAccessExpr : public Expr {
public:
    MemberAccessExpr(std::unique_ptr<Expr> obj, std::string mem, std::shared_ptr<Type> t)
        : object(std::move(obj)), member(std::move(mem)), type(std::move(t)) {}
    
    llvm::Value* codegen() override;
    std::shared_ptr<Type> getType() const override { return type; }
    
    Expr* getObject() const { return object.get(); }
    const std::string& getMember() const { return member; }

private:
    std::unique_ptr<Expr> object;
    std::string member;
    std::shared_ptr<Type> type;
};

/// Member method call expression (e.g., obj.method(args))
class MemberCallExpr : public Expr {
public:
    MemberCallExpr(std::unique_ptr<Expr> obj, std::string meth, 
                   std::vector<std::unique_ptr<Expr>> a, std::shared_ptr<Type> t)
        : object(std::move(obj)), method(std::move(meth)), 
          args(std::move(a)), type(std::move(t)) {}
    
    llvm::Value* codegen() override;
    std::shared_ptr<Type> getType() const override;
    
    Expr* getObject() const { return object.get(); }
    const std::string& getMethod() const { return method; }
    const std::vector<std::unique_ptr<Expr>>& getArgs() const { return args; }

private:
    std::unique_ptr<Expr> object;
    std::string method;
    std::vector<std::unique_ptr<Expr>> args;
    mutable std::shared_ptr<Type> type;  // mutable for lazy type resolution
};

/// New expression for creating class instances (e.g., new MyClass(args))
class NewExpr : public Expr {
public:
    NewExpr(std::string className, std::vector<std::unique_ptr<Expr>> a, std::shared_ptr<Type> t)
        : className(std::move(className)), args(std::move(a)), type(std::move(t)) {}
    
    llvm::Value* codegen() override;
    std::shared_ptr<Type> getType() const override { return type; }
    
    const std::string& getClassName() const { return className; }
    const std::vector<std::unique_ptr<Expr>>& getArgs() const { return args; }

private:
    std::string className;
    std::vector<std::unique_ptr<Expr>> args;
    std::shared_ptr<Type> type;
};

/// This expression (refers to current object in methods)
class ThisExpr : public Expr {
public:
    explicit ThisExpr(std::shared_ptr<Type> t) : type(std::move(t)) {}
    
    llvm::Value* codegen() override;
    std::shared_ptr<Type> getType() const override { return type; }

private:
    std::shared_ptr<Type> type;
};

/// Statement base class
class Stmt : public ASTNode {
public:
    virtual ~Stmt() = default;
};

/// Return statement
class ReturnStmt : public Stmt {
public:
    explicit ReturnStmt(std::unique_ptr<Expr> v)
        : value(std::move(v)) {}
    
    llvm::Value* codegen() override;
    Expr* getValue() const { return value.get(); }

private:
    std::unique_ptr<Expr> value;
};

/// Expression statement
class ExprStmt : public Stmt {
public:
    explicit ExprStmt(std::unique_ptr<Expr> e)
        : expr(std::move(e)) {}
    
    llvm::Value* codegen() override;

private:
    std::unique_ptr<Expr> expr;
};

/// Variable declaration (let x: int = 42;)
class VarDeclStmt : public Stmt {
public:
    VarDeclStmt(std::string n, std::shared_ptr<Type> t, std::unique_ptr<Expr> i, bool mut)
        : name(std::move(n)), type(std::move(t)), init(std::move(i)), mutable_(mut) {}
    
    llvm::Value* codegen() override;
    
    const std::string& getName() const { return name; }
    std::shared_ptr<Type> getType() const { return type; }
    Expr* getInit() const { return init.get(); }
    bool isMutable() const { return mutable_; }

private:
    std::string name;
    std::shared_ptr<Type> type;
    std::unique_ptr<Expr> init;
    bool mutable_;
};

/// If statement
class IfStmt : public Stmt {
public:
    IfStmt(std::unique_ptr<Expr> c, std::vector<std::unique_ptr<Stmt>> t,
           std::vector<std::unique_ptr<Stmt>> e)
        : condition(std::move(c)), thenBranch(std::move(t)), elseBranch(std::move(e)) {}
    
    llvm::Value* codegen() override;

private:
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> thenBranch;
    std::vector<std::unique_ptr<Stmt>> elseBranch;
};

/// While statement
class WhileStmt : public Stmt {
public:
    WhileStmt(std::unique_ptr<Expr> c, std::vector<std::unique_ptr<Stmt>> b)
        : condition(std::move(c)), body(std::move(b)) {}
    
    llvm::Value* codegen() override;

private:
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> body;
};

/// For statement with range (e.g., for i in 0..10 { ... })
class ForStmt : public Stmt {
public:
    ForStmt(std::string v, std::unique_ptr<Expr> start, std::unique_ptr<Expr> end,
            std::unique_ptr<Expr> step, std::vector<std::unique_ptr<Stmt>> b)
        : varName(std::move(v)), startExpr(std::move(start)), 
          endExpr(std::move(end)), stepExpr(std::move(step)), body(std::move(b)) {}
    
    llvm::Value* codegen() override;
    
    const std::string& getVarName() const { return varName; }

private:
    std::string varName;
    std::unique_ptr<Expr> startExpr;
    std::unique_ptr<Expr> endExpr;
    std::unique_ptr<Expr> stepExpr;  // nullable, default is 1
    std::vector<std::unique_ptr<Stmt>> body;
};

/// Loop statement (infinite loop: loop { ... })
class LoopStmt : public Stmt {
public:
    explicit LoopStmt(std::vector<std::unique_ptr<Stmt>> b)
        : body(std::move(b)) {}
    
    llvm::Value* codegen() override;

private:
    std::vector<std::unique_ptr<Stmt>> body;
};

/// Break statement
class BreakStmt : public Stmt {
public:
    BreakStmt() = default;
    
    llvm::Value* codegen() override;
};

/// Continue statement
class ContinueStmt : public Stmt {
public:
    ContinueStmt() = default;
    
    llvm::Value* codegen() override;
};

/// Assignment statement (target = value)
class AssignStmt : public Stmt {
public:
    AssignStmt(std::unique_ptr<Expr> t, std::unique_ptr<Expr> v)
        : target(std::move(t)), value(std::move(v)) {}
    
    llvm::Value* codegen() override;
    
    Expr* getTarget() const { return target.get(); }
    Expr* getValue() const { return value.get(); }

private:
    std::unique_ptr<Expr> target;  // Variable or member access
    std::unique_ptr<Expr> value;
};

/// Function parameter
struct Parameter {
    std::string name;
    std::shared_ptr<Type> type;
    
    Parameter(std::string n, std::shared_ptr<Type> t)
        : name(std::move(n)), type(std::move(t)) {}
};

/// Function prototype
class Prototype {
public:
    Prototype(std::string n, std::vector<Parameter> p, std::shared_ptr<Type> r)
        : name(std::move(n)), params(std::move(p)), returnType(std::move(r)), line_(0), column_(0) {}
    
    llvm::Function* codegen();
    
    const std::string& getName() const { return name; }
    const std::vector<Parameter>& getParams() const { return params; }
    std::shared_ptr<Type> getReturnType() const { return returnType; }
    
    void setLocation(size_t line, size_t col) { line_ = line; column_ = col; }
    size_t getLine() const { return line_; }
    size_t getColumn() const { return column_; }

private:
    std::string name;
    std::vector<Parameter> params;
    std::shared_ptr<Type> returnType;
    size_t line_;
    size_t column_;
};

/// Function definition
class Function {
public:
    Function(std::unique_ptr<Prototype> p, std::vector<std::unique_ptr<Stmt>> b)
        : proto(std::move(p)), body(std::move(b)) {}
    
    llvm::Function* codegen();
    
    Prototype* getProto() const { return proto.get(); }
    const std::vector<std::unique_ptr<Stmt>>& getBody() const { return body; }

private:
    std::unique_ptr<Prototype> proto;
    std::vector<std::unique_ptr<Stmt>> body;
};

/// Field declaration in a class
struct FieldDecl {
    std::string name;
    std::shared_ptr<Type> type;
    bool isPublic;
    std::unique_ptr<Expr> initializer;  // Optional default value
    
    FieldDecl(std::string n, std::shared_ptr<Type> t, bool pub, std::unique_ptr<Expr> init = nullptr)
        : name(std::move(n)), type(std::move(t)), isPublic(pub), initializer(std::move(init)) {}
};

/// Method declaration in a class (similar to Function but with class context)
struct MethodDecl {
    std::string name;
    std::vector<Parameter> params;
    std::shared_ptr<Type> returnType;
    std::vector<std::unique_ptr<Stmt>> body;
    bool isPublic;
    bool isStatic;
    bool isConstructor;
    
    MethodDecl(std::string n, std::vector<Parameter> p, std::shared_ptr<Type> ret, 
               std::vector<std::unique_ptr<Stmt>> b, bool pub, bool stat, bool ctor = false)
        : name(std::move(n)), params(std::move(p)), returnType(std::move(ret)),
          body(std::move(b)), isPublic(pub), isStatic(stat), isConstructor(ctor) {}
    
    llvm::Function* codegen(const std::string& className);
};

/// Class declaration
class ClassDecl {
public:
    ClassDecl(std::string n, std::vector<FieldDecl> f, std::vector<MethodDecl> m, bool isObj = false)
        : name(std::move(n)), fields(std::move(f)), methods(std::move(m)), isSingleton(isObj), line_(0), column_(0) {}
    
    llvm::Type* codegen();
    void codegenMethods();
    
    const std::string& getName() const { return name; }
    const std::vector<FieldDecl>& getFields() const { return fields; }
    const std::vector<MethodDecl>& getMethods() const { return methods; }
    bool isObjectSingleton() const { return isSingleton; }
    
    void setLocation(size_t line, size_t col) { line_ = line; column_ = col; }
    size_t getLine() const { return line_; }
    size_t getColumn() const { return column_; }
    
    // Helper methods
    FieldDecl* findField(const std::string& fieldName);
    MethodDecl* findMethod(const std::string& methodName);
    MethodDecl* findMethod(const std::string& methodName, 
                          const std::vector<std::shared_ptr<Type>>& paramTypes);
    
    // Generate implicit constructor if no explicit constructor exists
    void generateImplicitConstructor();

private:
    std::string name;
    std::vector<FieldDecl> fields;
    std::vector<MethodDecl> methods;
    bool isSingleton;  // true if declared with 'object' keyword
    size_t line_;
    size_t column_;
};

/// Package declaration
class PackageDecl {
public:
    explicit PackageDecl(std::string name) : packageName(std::move(name)) {}
    
    const std::string& getPackageName() const { return packageName; }
    
    // Convert package name to file system path (com.example.app -> com/example/app)
    std::string toPath() const;

private:
    std::string packageName;  // e.g., "com.example.myapp"
};

/// Import declaration
class ImportDecl {
public:
    explicit ImportDecl(std::string path) : modulePath(std::move(path)) {}
    
    const std::string& getModulePath() const { return modulePath; }
    
    // Load and compile the imported module
    // currentFile: path to the file that contains this import (for relative path resolution)
    // currentPackage: the package of the file containing this import (for package-relative imports)
    bool load(const std::string& currentFile = "", const std::string& currentPackage = "");

private:
    std::string modulePath;
};

} // namespace aurora
