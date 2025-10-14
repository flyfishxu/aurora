#pragma once

#include "aurora/AST.h"
#include "aurora/Lexer.h"
#include <memory>
#include <vector>

namespace aurora {

/// Recursive descent parser for AuroraLang
class Parser {
public:
    explicit Parser(Lexer& lexer);
    
    /// Parse the entire program (returns list of functions and classes)
    std::vector<std::unique_ptr<Function>> parseProgram();
    std::vector<std::unique_ptr<ClassDecl>>& getClasses() { return classes_; }
    std::vector<std::unique_ptr<ImportDecl>>& getImports() { return imports_; }
    
private:
    Lexer& lexer_;
    Token current_token_;
    std::vector<std::unique_ptr<ClassDecl>> classes_;
    std::vector<std::unique_ptr<ImportDecl>> imports_;
    
    // Symbol table for tracking variable types during parsing
    std::map<std::string, std::shared_ptr<Type>> local_types_;
    
    // Helper methods
    void advance();
    bool match(TokenType type);
    bool expect(TokenType type, const std::string& message);
    Token peek();
    
    // Parsing methods
    std::unique_ptr<Function> parseFunction();
    std::unique_ptr<Prototype> parsePrototype();
    std::unique_ptr<Prototype> parseExtern();
    std::unique_ptr<ImportDecl> parseImport();
    std::vector<std::unique_ptr<Stmt>> parseBlock();
    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<Stmt> parseReturnStatement();
    std::unique_ptr<Stmt> parseVarDecl();
    std::unique_ptr<Stmt> parseIfStatement();
    std::unique_ptr<Stmt> parseWhileStatement();
    std::unique_ptr<Stmt> parseForStatement();
    std::unique_ptr<Stmt> parseLoopStatement();
    std::unique_ptr<Stmt> parseBreakStatement();
    std::unique_ptr<Stmt> parseContinueStatement();
    
    // Class parsing methods
    std::unique_ptr<ClassDecl> parseClass();
    std::unique_ptr<ClassDecl> parseObject();
    std::unique_ptr<ClassDecl> parseClassOrObject(bool isSingleton);
    FieldDecl parseField(bool isPublic);
    MethodDecl parseMethod(bool isPublic);
    
    // Expression parsing (operator precedence climbing)
    std::unique_ptr<Expr> parseExpression();
    std::unique_ptr<Expr> parseLogicalOr();
    std::unique_ptr<Expr> parseLogicalAnd();
    std::unique_ptr<Expr> parseComparison();
    std::unique_ptr<Expr> parseBitwise();
    std::unique_ptr<Expr> parseAdditive();
    std::unique_ptr<Expr> parseMultiplicative();
    std::unique_ptr<Expr> parseUnary();
    std::unique_ptr<Expr> parsePostfix();
    std::unique_ptr<Expr> parsePrimary();
    std::unique_ptr<Expr> parseCall(std::string callee);
    
    // Type parsing
    std::shared_ptr<Type> parseType();
    
    // Type inference helpers
    std::shared_ptr<Type> inferMethodReturnType(const std::shared_ptr<Type>& objType, 
                                                 const std::string& methodName);
    
    // Error handling
    void error(const std::string& message);
};

} // namespace aurora
