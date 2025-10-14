#include "aurora/Parser.h"
#include "aurora/Diagnostic.h"
#include <stdexcept>

namespace aurora {

Parser::Parser(Lexer& lexer) 
    : lexer_(lexer), 
      current_token_(TokenType::Eof, "", 0, 0) {
    advance(); // Load first token
}

void Parser::advance() {
    current_token_ = lexer_.nextToken();
}

bool Parser::match(TokenType type) {
    if (current_token_.type == type) {
        advance();
        return true;
    }
    return false;
}

bool Parser::expect(TokenType type, const std::string& message) {
    if (current_token_.type != type) {
        error(message);
        return false;
    }
    advance();
    return true;
}

Token Parser::peek() {
    return current_token_;
}

void Parser::error(const std::string& message) {
    auto& diag = getDiagnosticEngine();
    
    // Create source location from current token
    SourceLocation loc(diag.isDebugMode() ? "<input>" : "<source>", 
                      current_token_.line, 
                      current_token_.column,
                      current_token_.value.length());
    
    // Report the error through diagnostic system
    std::string full_message = message;
    if (!current_token_.value.empty()) {
        full_message += " (got '" + current_token_.value + "')";
    }
    
    diag.reportError("E2001", full_message, loc);
    
    // Throw exception for error recovery (parser errors are fatal)
    throw std::runtime_error("Parse error: " + message);
}

std::vector<std::unique_ptr<Function>> Parser::parseProgram() {
    std::vector<std::unique_ptr<Function>> functions;
    
    while (current_token_.type != TokenType::Eof) {
        if (current_token_.type == TokenType::Import) {
            // Parse import declaration
            imports_.push_back(parseImport());
        } else if (current_token_.type == TokenType::Extern) {
            // Parse extern declaration and skip it (deprecated, kept for backward compatibility)
            parseExtern();
        } else if (current_token_.type == TokenType::Fn) {
            functions.push_back(parseFunction());
        } else if (current_token_.type == TokenType::Class) {
            classes_.push_back(parseClass());
        } else if (current_token_.type == TokenType::Object) {
            classes_.push_back(parseObject());
        } else {
            error("Expected 'import', 'fn', 'class', 'object', or 'extern'");
        }
    }
    
    return functions;
}

std::unique_ptr<ImportDecl> Parser::parseImport() {
    expect(TokenType::Import, "Expected 'import'");
    
    if (current_token_.type != TokenType::StringLiteral && 
        current_token_.type != TokenType::Identifier) {
        error("Expected module path after 'import' (string or identifier)");
    }
    
    std::string modulePath = current_token_.value;
    advance();
    
    // Optional semicolon
    match(TokenType::Semicolon);
    
    return std::make_unique<ImportDecl>(modulePath);
}

// DEPRECATED: extern declarations are no longer needed
// Built-in functions are automatically registered by the compiler
// This is kept for backward compatibility with old code
std::unique_ptr<Prototype> Parser::parseExtern() {
    expect(TokenType::Extern, "Expected 'extern'");
    
    if (current_token_.type != TokenType::Identifier) {
        error("Expected function name after 'extern'");
    }
    std::string name = current_token_.value;
    advance();
    
    expect(TokenType::LeftParen, "Expected '(' after function name");
    
    std::vector<Parameter> params;
    if (current_token_.type != TokenType::RightParen) {
        do {
            if (current_token_.type != TokenType::Identifier) {
                error("Expected parameter name");
            }
            std::string param_name = current_token_.value;
            advance();
            
            // For extern functions, assume double type (legacy compatibility)
            auto& registry = TypeRegistry::instance();
            params.push_back(Parameter(param_name, registry.getDoubleType()));
            
        } while (match(TokenType::Comma));
    }
    
    expect(TokenType::RightParen, "Expected ')' after parameters");
    
    // Optional semicolon
    match(TokenType::Semicolon);
    
    auto& registry = TypeRegistry::instance();
    auto proto = std::make_unique<Prototype>(name, params, registry.getDoubleType());
    
    // Note: External prototypes are no longer registered
    // Built-in functions are automatically available
    
    return proto;
}

std::unique_ptr<Function> Parser::parseFunction() {
    expect(TokenType::Fn, "Expected 'fn'");
    
    auto proto = parsePrototype();
    
    // Clear local types for new function scope
    local_types_.clear();
    
    // Add function parameters to local types
    for (const auto& param : proto->getParams()) {
        local_types_[param.name] = param.type;
    }
    
    auto body = parseBlock();
    
    return std::make_unique<Function>(std::move(proto), std::move(body));
}

std::unique_ptr<Prototype> Parser::parsePrototype() {
    if (current_token_.type != TokenType::Identifier) {
        error("Expected function name");
    }
    std::string name = current_token_.value;
    advance();
    
    expect(TokenType::LeftParen, "Expected '(' after function name");
    
    std::vector<Parameter> params;
    if (current_token_.type != TokenType::RightParen) {
        do {
            if (current_token_.type != TokenType::Identifier) {
                error("Expected parameter name");
            }
            std::string param_name = current_token_.value;
            advance();
            
            // Type annotation is optional in basic syntax
            std::shared_ptr<Type> param_type;
            if (match(TokenType::Colon)) {
                param_type = parseType();
            } else {
                // Default to double for legacy compatibility
                auto& registry = TypeRegistry::instance();
                param_type = registry.getDoubleType();
            }
            
            params.push_back(Parameter(param_name, param_type));
            
        } while (match(TokenType::Comma));
    }
    
    expect(TokenType::RightParen, "Expected ')' after parameters");
    
    // Return type annotation (optional)
    std::shared_ptr<Type> return_type;
    if (match(TokenType::Arrow)) {
        return_type = parseType();
    } else {
        // Default to void (modern design)
        // Functions that want to return a value should explicitly declare return type
        auto& registry = TypeRegistry::instance();
        return_type = registry.getVoidType();
    }
    
    auto proto = std::make_unique<Prototype>(name, params, return_type);
    
    // Note: We don't store prototypes in the map anymore to avoid memory issues
    // Type information is tracked through the Type system instead
    
    return proto;
}

std::vector<std::unique_ptr<Stmt>> Parser::parseBlock() {
    expect(TokenType::LeftBrace, "Expected '{'");
    
    std::vector<std::unique_ptr<Stmt>> statements;
    while (current_token_.type != TokenType::RightBrace && 
           current_token_.type != TokenType::Eof) {
        statements.push_back(parseStatement());
    }
    
    expect(TokenType::RightBrace, "Expected '}'");
    return statements;
}

std::unique_ptr<Stmt> Parser::parseStatement() {
    if (current_token_.type == TokenType::Return) {
        return parseReturnStatement();
    } else if (current_token_.type == TokenType::Let || 
               current_token_.type == TokenType::Var) {
        return parseVarDecl();
    } else if (current_token_.type == TokenType::If) {
        return parseIfStatement();
    } else if (current_token_.type == TokenType::While) {
        return parseWhileStatement();
    } else if (current_token_.type == TokenType::For) {
        return parseForStatement();
    } else if (current_token_.type == TokenType::Loop) {
        return parseLoopStatement();
    } else if (current_token_.type == TokenType::Break) {
        return parseBreakStatement();
    } else if (current_token_.type == TokenType::Continue) {
        return parseContinueStatement();
    } else {
        // Try parsing as expression, check for assignment
        auto expr = parseExpression();
        
        // Check if it's an assignment statement
        if (current_token_.type == TokenType::Equal) {
            advance(); // consume '='
            auto value = parseExpression();
            match(TokenType::Semicolon); // Optional semicolon
            return std::make_unique<AssignStmt>(std::move(expr), std::move(value));
        }
        
        // Otherwise it's an expression statement
        match(TokenType::Semicolon); // Optional semicolon
        return std::make_unique<ExprStmt>(std::move(expr));
    }
}

std::unique_ptr<Stmt> Parser::parseReturnStatement() {
    expect(TokenType::Return, "Expected 'return'");
    
    // Check if there's a value to return
    std::unique_ptr<Expr> value = nullptr;
    if (current_token_.type != TokenType::Semicolon && 
        current_token_.type != TokenType::RightBrace &&
        current_token_.type != TokenType::Eof) {
        value = parseExpression();
    }
    
    match(TokenType::Semicolon); // Optional semicolon
    return std::make_unique<ReturnStmt>(std::move(value));
}

std::unique_ptr<Stmt> Parser::parseVarDecl() {
    bool is_mutable = false;
    if (match(TokenType::Var)) {
        is_mutable = true;
    } else {
        expect(TokenType::Let, "Expected 'let' or 'var'");
        is_mutable = false;
    }
    
    if (current_token_.type != TokenType::Identifier) {
        error("Expected variable name");
    }
    std::string name = current_token_.value;
    advance();
    
    // Type annotation (optional)
    std::shared_ptr<Type> var_type;
    if (match(TokenType::Colon)) {
        var_type = parseType();
    }
    
    // Initializer
    std::unique_ptr<Expr> init;
    if (match(TokenType::Equal)) {
        init = parseExpression();
    } else {
        error("Variable declaration requires initializer");
    }
    
    // Infer type if not specified
    if (!var_type) {
        var_type = init->getType();
    }
    
    // Record variable type in local symbol table
    local_types_[name] = var_type;
    
    match(TokenType::Semicolon); // Optional semicolon
    return std::make_unique<VarDeclStmt>(name, var_type, std::move(init), is_mutable);
}

std::unique_ptr<Stmt> Parser::parseIfStatement() {
    expect(TokenType::If, "Expected 'if'");
    
    auto condition = parseExpression();
    auto thenBranch = parseBlock();
    
    std::vector<std::unique_ptr<Stmt>> elseBranch;
    if (match(TokenType::Else)) {
        if (current_token_.type == TokenType::If) {
            // else if
            elseBranch.push_back(parseIfStatement());
        } else {
            // else
            elseBranch = parseBlock();
        }
    }
    
    return std::make_unique<IfStmt>(std::move(condition), 
                                    std::move(thenBranch), 
                                    std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::parseWhileStatement() {
    expect(TokenType::While, "Expected 'while'");
    
    auto condition = parseExpression();
    auto body = parseBlock();
    
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::parseForStatement() {
    expect(TokenType::For, "Expected 'for'");
    
    // Parse loop variable: for i in ...
    if (current_token_.type != TokenType::Identifier) {
        error("Expected loop variable name after 'for'");
    }
    std::string varName = current_token_.value;
    advance();
    
    expect(TokenType::In, "Expected 'in' after loop variable");
    
    // Parse start expression
    auto startExpr = parseExpression();
    
    // Expect '..' for range
    expect(TokenType::DotDot, "Expected '..' for range in for loop");
    
    // Parse end expression
    auto endExpr = parseExpression();
    
    // Optional step (not implemented yet, defaults to 1)
    std::unique_ptr<Expr> stepExpr = nullptr;
    
    // Parse body
    auto body = parseBlock();
    
    return std::make_unique<ForStmt>(varName, std::move(startExpr), 
                                     std::move(endExpr), std::move(stepExpr), 
                                     std::move(body));
}

std::unique_ptr<Stmt> Parser::parseLoopStatement() {
    expect(TokenType::Loop, "Expected 'loop'");
    
    auto body = parseBlock();
    
    return std::make_unique<LoopStmt>(std::move(body));
}

std::unique_ptr<Stmt> Parser::parseBreakStatement() {
    expect(TokenType::Break, "Expected 'break'");
    match(TokenType::Semicolon); // Optional semicolon
    
    return std::make_unique<BreakStmt>();
}

std::unique_ptr<Stmt> Parser::parseContinueStatement() {
    expect(TokenType::Continue, "Expected 'continue'");
    match(TokenType::Semicolon); // Optional semicolon
    
    return std::make_unique<ContinueStmt>();
}

std::unique_ptr<Expr> Parser::parseExpression() {
    auto expr = parseLogicalOr();
    
    // Check for ternary operator (? :)
    if (current_token_.type == TokenType::Question) {
        advance(); // consume '?'
        auto trueExpr = parseExpression();
        expect(TokenType::Colon, "Expected ':' in ternary expression");
        auto falseExpr = parseExpression();
        return std::make_unique<TernaryExpr>(std::move(expr), std::move(trueExpr), std::move(falseExpr));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parseLogicalOr() {
    auto left = parseLogicalAnd();
    
    while (current_token_.type == TokenType::Or) {
        advance(); // consume '||'
        auto right = parseLogicalAnd();
        left = std::make_unique<BinaryExpr>(BinaryExpr::Op::Or, 
                                           std::move(left), 
                                           std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expr> Parser::parseLogicalAnd() {
    auto left = parseComparison();
    
    while (current_token_.type == TokenType::And) {
        advance(); // consume '&&'
        auto right = parseComparison();
        left = std::make_unique<BinaryExpr>(BinaryExpr::Op::And, 
                                           std::move(left), 
                                           std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expr> Parser::parseComparison() {
    auto left = parseBitwise();
    
    while (current_token_.type == TokenType::Less ||
           current_token_.type == TokenType::Greater ||
           current_token_.type == TokenType::LessEq ||
           current_token_.type == TokenType::GreaterEq ||
           current_token_.type == TokenType::EqualEqual ||
           current_token_.type == TokenType::NotEqual) {
        
        BinaryExpr::Op op;
        if (match(TokenType::Less)) {
            op = BinaryExpr::Op::Less;
        } else if (match(TokenType::Greater)) {
            op = BinaryExpr::Op::Greater;
        } else if (match(TokenType::LessEq)) {
            op = BinaryExpr::Op::LessEq;
        } else if (match(TokenType::GreaterEq)) {
            op = BinaryExpr::Op::GreaterEq;
        } else if (match(TokenType::EqualEqual)) {
            op = BinaryExpr::Op::Equal;
        } else {
            match(TokenType::NotEqual);
            op = BinaryExpr::Op::NotEqual;
        }
        
        auto right = parseBitwise();
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }
    
    return left;
}

// Parse bitwise operators (&, |, ^, <<, >>)
std::unique_ptr<Expr> Parser::parseBitwise() {
    auto left = parseAdditive();
    
    while (current_token_.type == TokenType::Ampersand ||
           current_token_.type == TokenType::Pipe ||
           current_token_.type == TokenType::Caret ||
           current_token_.type == TokenType::LeftShift ||
           current_token_.type == TokenType::RightShift) {
        
        BinaryExpr::Op op;
        if (match(TokenType::Ampersand)) {
            op = BinaryExpr::Op::BitwiseAnd;
        } else if (match(TokenType::Pipe)) {
            op = BinaryExpr::Op::BitwiseOr;
        } else if (match(TokenType::Caret)) {
            op = BinaryExpr::Op::BitwiseXor;
        } else if (match(TokenType::LeftShift)) {
            op = BinaryExpr::Op::LeftShift;
        } else {
            match(TokenType::RightShift);
            op = BinaryExpr::Op::RightShift;
        }
        
        auto right = parseAdditive();
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expr> Parser::parseAdditive() {
    auto left = parseMultiplicative();
    
    while (current_token_.type == TokenType::Plus || 
           current_token_.type == TokenType::Minus) {
        BinaryExpr::Op op = match(TokenType::Plus) ? 
            BinaryExpr::Op::Add : BinaryExpr::Op::Sub;
        if (op == BinaryExpr::Op::Sub) advance();
        
        auto right = parseMultiplicative();
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expr> Parser::parseMultiplicative() {
    auto left = parseUnary();
    
    while (current_token_.type == TokenType::Star || 
           current_token_.type == TokenType::Slash ||
           current_token_.type == TokenType::Percent) {
        BinaryExpr::Op op;
        if (match(TokenType::Star)) {
            op = BinaryExpr::Op::Mul;
        } else if (match(TokenType::Slash)) {
            op = BinaryExpr::Op::Div;
        } else {
            match(TokenType::Percent);
            op = BinaryExpr::Op::Mod;
        }
        
        auto right = parseUnary();
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }
    
    return left;
}

std::unique_ptr<Expr> Parser::parseUnary() {
    if (current_token_.type == TokenType::Not || 
        current_token_.type == TokenType::Minus ||
        current_token_.type == TokenType::Tilde) {
        UnaryExpr::Op op;
        if (match(TokenType::Not)) {
            op = UnaryExpr::Op::Not;
        } else if (match(TokenType::Minus)) {
            op = UnaryExpr::Op::Neg;
        } else {
            match(TokenType::Tilde);
            op = UnaryExpr::Op::BitwiseNot;
        }
        
        auto expr = parseUnary();
        return std::make_unique<UnaryExpr>(op, std::move(expr));
    }
    
    return parsePostfix();
}

// Parse postfix expressions (array indexing, etc.)
std::unique_ptr<Expr> Parser::parsePostfix() {
    auto expr = parsePrimary();
    
    while (true) {
        if (current_token_.type == TokenType::LeftBracket) {
            // Array indexing
            advance(); // consume '['
            auto index = parseExpression();
            expect(TokenType::RightBracket, "Expected ']' after array index");
            expr = std::make_unique<ArrayIndexExpr>(std::move(expr), std::move(index));
        } else {
            break;
        }
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    // Number literal (int or double)
    if (current_token_.type == TokenType::IntLiteral) {
        // Integer literal
        int64_t value = std::stoll(current_token_.value);
        advance();
        return std::make_unique<IntLiteralExpr>(value);
    }
    
    if (current_token_.type == TokenType::DoubleLiteral) {
        // Double literal
        double value = std::stod(current_token_.value);
        advance();
        return std::make_unique<DoubleLiteralExpr>(value);
    }
    
    // String literal
    if (current_token_.type == TokenType::StringLiteral) {
        std::string value = current_token_.value;
        advance();
        return std::make_unique<StringExpr>(value);
    }
    
    // Boolean literal
    if (current_token_.type == TokenType::True || 
        current_token_.type == TokenType::False) {
        bool value = current_token_.type == TokenType::True;
        advance();
        return std::make_unique<BoolExpr>(value);
    }
    
    // Null literal
    if (current_token_.type == TokenType::Null) {
        advance();
        return std::make_unique<NullExpr>();
    }
    
    // 'this' keyword
    if (current_token_.type == TokenType::This) {
        advance();
        auto& registry = TypeRegistry::instance();
        // Type will be determined at context
        std::unique_ptr<Expr> expr = std::make_unique<ThisExpr>(registry.getClassType("Unknown"));
        
        // Check for member access
        while (current_token_.type == TokenType::Dot) {
            advance();
            if (current_token_.type != TokenType::Identifier) {
                error("Expected member name after '.'");
            }
            std::string memberName = current_token_.value;
            advance();
            
            // Check if it's a method call
            if (current_token_.type == TokenType::LeftParen) {
                advance();
                std::vector<std::unique_ptr<Expr>> args;
                if (current_token_.type != TokenType::RightParen) {
                    do {
                        args.push_back(parseExpression());
                    } while (match(TokenType::Comma));
                }
                expect(TokenType::RightParen, "Expected ')' after arguments");
                
                auto returnType = inferMethodReturnType(expr->getType(), memberName);
                expr = std::make_unique<MemberCallExpr>(std::move(expr), memberName, 
                                                       std::move(args), returnType);
            } else {
                // It's a field access
                expr = std::make_unique<MemberAccessExpr>(std::move(expr), memberName, 
                                                         registry.getDoubleType());
            }
        }
        return expr;
    }
    
    // Identifier or function call
    if (current_token_.type == TokenType::Identifier) {
        std::string name = current_token_.value;
        advance();
        
        if (current_token_.type == TokenType::LeftParen) {
            std::unique_ptr<Expr> callExpr = parseCall(name);
            
            // Check for member access chaining
            while (current_token_.type == TokenType::Dot) {
                advance();
                if (current_token_.type != TokenType::Identifier) {
                    error("Expected member name after '.'");
                }
                std::string memberName = current_token_.value;
                advance();
                
                auto& registry = TypeRegistry::instance();
                if (current_token_.type == TokenType::LeftParen) {
                    advance();
                    std::vector<std::unique_ptr<Expr>> args;
                    if (current_token_.type != TokenType::RightParen) {
                        do {
                            args.push_back(parseExpression());
                        } while (match(TokenType::Comma));
                    }
                    expect(TokenType::RightParen, "Expected ')' after arguments");
                    
                    auto returnType = inferMethodReturnType(callExpr->getType(), memberName);
                    callExpr = std::make_unique<MemberCallExpr>(std::move(callExpr), memberName,
                                                                std::move(args), returnType);
                } else {
                    callExpr = std::make_unique<MemberAccessExpr>(std::move(callExpr), memberName,
                                                                  registry.getDoubleType());
                }
            }
            return callExpr;
        } else {
            // Variable reference - try to find type from local symbol table
            auto& registry = TypeRegistry::instance();
            std::shared_ptr<Type> varType = registry.getIntType(); // Default placeholder
            
            auto it = local_types_.find(name);
            if (it != local_types_.end()) {
                varType = it->second;
            }
            
            std::unique_ptr<Expr> varExpr = std::make_unique<VariableExpr>(name, varType);
            
            // Check for member access
            while (current_token_.type == TokenType::Dot) {
                advance();
                if (current_token_.type != TokenType::Identifier) {
                    error("Expected member name after '.'");
                }
                std::string memberName = current_token_.value;
                advance();
                
                if (current_token_.type == TokenType::LeftParen) {
                    advance();
                    std::vector<std::unique_ptr<Expr>> args;
                    if (current_token_.type != TokenType::RightParen) {
                        do {
                            args.push_back(parseExpression());
                        } while (match(TokenType::Comma));
                    }
                    expect(TokenType::RightParen, "Expected ')' after arguments");
                    
                    auto returnType = inferMethodReturnType(varExpr->getType(), memberName);
                    varExpr = std::make_unique<MemberCallExpr>(std::move(varExpr), memberName,
                                                               std::move(args), returnType);
                } else {
                    varExpr = std::make_unique<MemberAccessExpr>(std::move(varExpr), memberName,
                                                                 registry.getDoubleType());
                }
            }
            return varExpr;
        }
    }
    
    // Array literal [1, 2, 3]
    if (current_token_.type == TokenType::LeftBracket) {
        advance(); // consume '['
        
        std::vector<std::unique_ptr<Expr>> elements;
        
        if (current_token_.type != TokenType::RightBracket) {
            do {
                elements.push_back(parseExpression());
            } while (match(TokenType::Comma));
        }
        
        expect(TokenType::RightBracket, "Expected ']' after array elements");
        
        // Infer element type from first element
        auto& registry = TypeRegistry::instance();
        std::shared_ptr<Type> elemType;
        if (!elements.empty()) {
            elemType = elements[0]->getType();
        } else {
            elemType = registry.getIntType(); // Default to int for empty arrays
        }
        
        auto arrayType = registry.getArrayType(elemType);
        return std::make_unique<ArrayLiteralExpr>(std::move(elements), arrayType);
    }
    
    // Parenthesized expression
    if (match(TokenType::LeftParen)) {
        auto expr = parseExpression();
        expect(TokenType::RightParen, "Expected ')' after expression");
        return expr;
    }
    
    error("Expected expression");
    return nullptr;
}

std::unique_ptr<Expr> Parser::parseCall(std::string callee) {
    expect(TokenType::LeftParen, "Expected '('");
    
    std::vector<std::unique_ptr<Expr>> args;
    if (current_token_.type != TokenType::RightParen) {
        do {
            args.push_back(parseExpression());
        } while (match(TokenType::Comma));
    }
    
    expect(TokenType::RightParen, "Expected ')' after arguments");
    
    auto& registry = TypeRegistry::instance();
    
    // Check if this is a class constructor call
    if (registry.hasClassType(callee)) {
        return std::make_unique<NewExpr>(callee, std::move(args), 
                                        registry.getClassType(callee));
    }
    
    // Otherwise it's a regular function call
    std::shared_ptr<Type> return_type = registry.getDoubleType();
    return std::make_unique<CallExpr>(callee, std::move(args), return_type);
}

std::shared_ptr<Type> Parser::parseType() {
    auto& registry = TypeRegistry::instance();
    
    // Handle array types [ElementType]
    if (current_token_.type == TokenType::LeftBracket) {
        advance(); // consume '['
        std::shared_ptr<Type> elementType = parseType(); // recursively parse element type
        expect(TokenType::RightBracket, "Expected ']' after array element type");
        
        bool is_optional = match(TokenType::Question);
        std::shared_ptr<Type> arrayType = registry.getArrayType(elementType);
        return is_optional ? std::shared_ptr<Type>(registry.getOptionalType(arrayType)) : arrayType;
    }
    
    // Handle built-in types
    if (current_token_.type == TokenType::TypeInt) {
        advance();
        bool is_optional = match(TokenType::Question);
        std::shared_ptr<Type> type = registry.getIntType();
        return is_optional ? std::shared_ptr<Type>(registry.getOptionalType(type)) : type;
    }
    if (current_token_.type == TokenType::TypeDouble) {
        advance();
        bool is_optional = match(TokenType::Question);
        std::shared_ptr<Type> type = registry.getDoubleType();
        return is_optional ? std::shared_ptr<Type>(registry.getOptionalType(type)) : type;
    }
    if (current_token_.type == TokenType::TypeBool) {
        advance();
        bool is_optional = match(TokenType::Question);
        std::shared_ptr<Type> type = registry.getBoolType();
        return is_optional ? std::shared_ptr<Type>(registry.getOptionalType(type)) : type;
    }
    if (current_token_.type == TokenType::TypeString) {
        advance();
        bool is_optional = match(TokenType::Question);
        std::shared_ptr<Type> type = registry.getStringType();
        return is_optional ? std::shared_ptr<Type>(registry.getOptionalType(type)) : type;
    }
    if (current_token_.type == TokenType::TypeVoid) {
        advance();
        return registry.getVoidType();
    }
    
    // Handle function types fn(ParamType) -> ReturnType
    if (current_token_.type == TokenType::Fn) {
        advance(); // consume 'fn'
        expect(TokenType::LeftParen, "Expected '(' after 'fn'");
        
        std::vector<std::shared_ptr<Type>> paramTypes;
        if (current_token_.type != TokenType::RightParen) {
            do {
                paramTypes.push_back(parseType());
            } while (match(TokenType::Comma));
        }
        
        expect(TokenType::RightParen, "Expected ')' after function parameters");
        
        std::shared_ptr<Type> returnType;
        if (match(TokenType::Arrow)) {
            returnType = parseType();
        } else {
            returnType = registry.getVoidType();
        }
        
        return registry.getFunctionType(returnType, paramTypes);
    }
    
    // Handle identifiers (could be class names)
    if (current_token_.type == TokenType::Identifier) {
        std::string type_name = current_token_.value;
        advance();
        
        // Check for optional marker (?)
        bool is_optional = match(TokenType::Question);
        
        // Try class type
        std::shared_ptr<Type> type = registry.getClassType(type_name);
        return is_optional ? std::shared_ptr<Type>(registry.getOptionalType(type)) : type;
    }
    
    error("Expected type name");
    return nullptr;
}

// ===== Object-Oriented Parsing =====

std::unique_ptr<ClassDecl> Parser::parseClass() {
    return parseClassOrObject(false);
}

std::unique_ptr<ClassDecl> Parser::parseObject() {
    return parseClassOrObject(true);
}

std::unique_ptr<ClassDecl> Parser::parseClassOrObject(bool isSingleton) {
    if (isSingleton) {
        expect(TokenType::Object, "Expected 'object'");
    } else {
        expect(TokenType::Class, "Expected 'class'");
    }
    
    if (current_token_.type != TokenType::Identifier) {
        error("Expected class/object name");
    }
    std::string className = current_token_.value;
    advance();
    
    // Register class type early so it can be used in later function declarations
    auto& registry = TypeRegistry::instance();
    auto classType = registry.getClassType(className);
    
    std::vector<FieldDecl> fields;
    std::vector<MethodDecl> methods;
    
    // Check for primary constructor: class Name(val x: Type, var y: Type)
    if (match(TokenType::LeftParen)) {
        std::vector<Parameter> primaryCtorParams;
        
        if (current_token_.type != TokenType::RightParen) {
            do {
                bool isPublic = true;
                
                // Check for visibility modifiers
                if (current_token_.type == TokenType::Pub) {
                    isPublic = true;
                    advance();
                } else if (current_token_.type == TokenType::Priv) {
                    isPublic = false;
                    advance();
                }
                
                // Expect let or var
                bool isMutable = false;
                if (current_token_.type == TokenType::Var) {
                    isMutable = true;
                    advance();
                } else if (current_token_.type == TokenType::Let) {
                    advance();
                } else {
                    error("Expected 'let' or 'var' in primary constructor parameter");
                }
                
                // Get field name
                if (current_token_.type != TokenType::Identifier) {
                    error("Expected parameter name in primary constructor");
                }
                std::string fieldName = current_token_.value;
                advance();
                
                // Expect colon and type
                expect(TokenType::Colon, "Expected ':' after parameter name");
                std::shared_ptr<Type> fieldType = parseType();
                
                // Add as field
                fields.push_back(FieldDecl(fieldName, fieldType, isPublic, nullptr));
                
                // Add as constructor parameter
                primaryCtorParams.push_back(Parameter(fieldName, fieldType));
                
            } while (match(TokenType::Comma));
        }
        
        expect(TokenType::RightParen, "Expected ')' after primary constructor parameters");
        
        // Generate primary constructor if we have parameters
        if (!primaryCtorParams.empty()) {
            std::vector<std::unique_ptr<Stmt>> ctorBody;
            
            // Generate assignment for each parameter: this.field = param
            for (const auto& param : primaryCtorParams) {
                auto thisExpr = std::make_unique<ThisExpr>(classType);
                auto memberAccess = std::make_unique<MemberAccessExpr>(
                    std::move(thisExpr), param.name, param.type);
                auto paramExpr = std::make_unique<VariableExpr>(param.name, param.type);
                auto assignment = std::make_unique<AssignStmt>(
                    std::move(memberAccess), std::move(paramExpr));
                ctorBody.push_back(std::move(assignment));
            }
            
            // Create primary constructor method
            MethodDecl primaryCtor(
                "constructor",
                std::move(primaryCtorParams),
                registry.getVoidType(),
                std::move(ctorBody),
                true,   // isPublic
                false,  // isStatic
                true    // isConstructor
            );
            
            methods.push_back(std::move(primaryCtor));
        }
    }
    
    expect(TokenType::LeftBrace, "Expected '{' after class/object name");
    
    // Parse class members
    while (current_token_.type != TokenType::RightBrace && current_token_.type != TokenType::Eof) {
        bool isPublic = true;  // Default to public
        
        // Check for visibility modifiers
        if (current_token_.type == TokenType::Pub) {
            isPublic = true;
            advance();
        } else if (current_token_.type == TokenType::Priv) {
            isPublic = false;
            advance();
        }
        
        // Check if it's a constructor
        if (current_token_.type == TokenType::Constructor) {
            methods.push_back(parseMethod(isPublic));
        }
        // Check if it's a method (fn keyword)
        else if (current_token_.type == TokenType::Fn) {
            methods.push_back(parseMethod(isPublic));
        }
        // Otherwise it's a field
        else if (current_token_.type == TokenType::Let || current_token_.type == TokenType::Var) {
            fields.push_back(parseField(isPublic));
        }
        else {
            error("Expected field or method declaration in class/object");
        }
    }
    
    expect(TokenType::RightBrace, "Expected '}' after class/object body");
    
    // Create class declaration with singleton flag
    auto classDecl = std::make_unique<ClassDecl>(className, std::move(fields), std::move(methods), isSingleton);
    
    // Generate implicit constructor if no explicit constructor found
    classDecl->generateImplicitConstructor();
    
    // Link class type to declaration
    classType->setDecl(classDecl.get());
    
    return classDecl;
}

FieldDecl Parser::parseField(bool isPublic) {
    bool isMutable = false;
    if (current_token_.type == TokenType::Var) {
        isMutable = true;
        advance();
    } else {
        expect(TokenType::Let, "Expected 'let' or 'var' for field");
    }
    
    if (current_token_.type != TokenType::Identifier) {
        error("Expected field name");
    }
    std::string fieldName = current_token_.value;
    advance();
    
    expect(TokenType::Colon, "Expected ':' after field name");
    
    auto fieldType = parseType();
    
    std::unique_ptr<Expr> initializer = nullptr;
    if (match(TokenType::Equal)) {
        initializer = parseExpression();
    }
    
    // Optional semicolon
    match(TokenType::Semicolon);
    
    return FieldDecl(fieldName, fieldType, isPublic, std::move(initializer));
}

std::shared_ptr<Type> Parser::inferMethodReturnType(const std::shared_ptr<Type>& objType, 
                                                     const std::string& methodName) {
    auto& registry = TypeRegistry::instance();
    
    // Check if the object type is a class type
    auto classType = std::dynamic_pointer_cast<ClassType>(objType);
    if (!classType) {
        return registry.getDoubleType(); // Fallback
    }
    
    ClassDecl* classDecl = classType->getDecl();
    if (!classDecl) {
        return registry.getDoubleType(); // Fallback
    }
    
    // Find the method in the class
    MethodDecl* method = classDecl->findMethod(methodName);
    if (method && method->returnType) {
        return method->returnType;
    }
    
    return registry.getDoubleType(); // Fallback
}

MethodDecl Parser::parseMethod(bool isPublic) {
    bool isConstructor = false;
    bool isStatic = false;
    
    // Check for static
    if (current_token_.type == TokenType::Static) {
        isStatic = true;
        advance();
    }
    
    // Check for constructor
    if (current_token_.type == TokenType::Constructor) {
        isConstructor = true;
        advance();
    } else {
        expect(TokenType::Fn, "Expected 'fn' for method");
    }
    
    std::string methodName;
    if (!isConstructor) {
        if (current_token_.type != TokenType::Identifier) {
            error("Expected method name");
        }
        methodName = current_token_.value;
        advance();
    }
    
    expect(TokenType::LeftParen, "Expected '(' after method name");
    
    // Parse parameters
    std::vector<Parameter> params;
    if (current_token_.type != TokenType::RightParen) {
        do {
            if (current_token_.type != TokenType::Identifier) {
                error("Expected parameter name");
            }
            std::string paramName = current_token_.value;
            advance();
            
            std::shared_ptr<Type> paramType;
            if (match(TokenType::Colon)) {
                paramType = parseType();
            } else {
                // Default to double for compatibility
                paramType = TypeRegistry::instance().getDoubleType();
            }
            
            params.push_back(Parameter(paramName, paramType));
        } while (match(TokenType::Comma));
    }
    
    expect(TokenType::RightParen, "Expected ')' after parameters");

    if (isConstructor) {
        methodName = "constructor";
    }
    
    // Parse return type
    std::shared_ptr<Type> returnType;
    if (match(TokenType::Arrow)) {
        returnType = parseType();
    } else {
        // Default to void for methods without explicit return type
        returnType = TypeRegistry::instance().getVoidType();
    }
    
    // Clear local types for new method scope
    local_types_.clear();
    
    // Add method parameters to local types
    for (const auto& param : params) {
        local_types_[param.name] = param.type;
    }
    
    // Parse method body
    auto body = parseBlock();
    
    return MethodDecl(methodName, std::move(params), returnType, 
                     std::move(body), isPublic, isStatic, isConstructor);
}

} // namespace aurora
