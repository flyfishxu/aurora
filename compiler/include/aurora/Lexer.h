#pragma once

#include <string>

namespace aurora {

/// Token types
enum class TokenType {
    // End of file
    Eof,
    
    // Keywords
    Fn,           // fn
    Extern,       // extern (deprecated - for backward compatibility)
    Return,       // return
    Let,          // let
    Var,          // var
    If,           // if
    Else,         // else
    While,        // while
    For,          // for
    Loop,         // loop
    Break,        // break
    Continue,     // continue
    In,           // in
    Match,        // match
    True,         // true
    False,        // false
    Null,         // null
    Import,       // import
    Package,      // package
    
    // Object-Oriented Keywords
    Class,        // class
    Object,       // object (singleton class)
    This,         // this
    Pub,          // pub (public)
    Priv,         // priv (private)
    Static,       // static
    Constructor,  // constructor
    
    // Types
    TypeInt,      // int
    TypeDouble,   // double
    TypeBool,     // bool
    TypeString,   // string
    TypeVoid,     // void
    
    // Identifiers and literals
    Identifier,   // variable/function names
    IntLiteral,   // 123
    DoubleLiteral,// 3.14
    StringLiteral,// "hello"
    
    // Operators
    Plus,         // +
    Minus,        // -
    Star,         // *
    Slash,        // /
    Percent,      // %
    
    // Bitwise operators
    Ampersand,    // &
    Pipe,         // |
    Caret,        // ^
    Tilde,        // ~
    LeftShift,    // <<
    RightShift,   // >>
    
    // Comparison
    Less,         // <
    Greater,      // >
    LessEq,       // <=
    GreaterEq,    // >=
    EqualEqual,   // ==
    NotEqual,     // !=
    
    // Logical
    And,          // &&
    Or,           // ||
    Not,          // !
    
    // Nullable operators
    QuestionDot,      // ?. (safe navigation)
    QuestionQuestion, // ?? (null coalescing)
    Exclaim,          // ! (force unwrap, also used as Not in expressions)
    
    // Assignment
    Equal,        // =
    
    // Delimiters
    LeftParen,    // (
    RightParen,   // )
    LeftBrace,    // {
    RightBrace,   // }
    LeftBracket,  // [
    RightBracket, // ]
    Comma,        // ,
    Semicolon,    // ;
    Colon,        // :
    Question,     // ?
    Arrow,        // ->
    DotDot,       // .. (range operator)
    Dot,          // .
};

/// Token structure
struct Token {
    TokenType type;
    std::string value;
    size_t line;
    size_t column;
    
    Token(TokenType t, std::string v, size_t l, size_t c)
        : type(t), value(std::move(v)), line(l), column(c) {}
    
    std::string toString() const;
};

/// Lexer - Tokenizes source code
class Lexer {
public:
    explicit Lexer(std::string source);
    
    /// Get next token
    Token nextToken();
    
    /// Peek at next token without consuming it
    Token peekToken();
    
    /// Get current position info
    size_t getLine() const { return line; }
    size_t getColumn() const { return column; }

private:
    std::string source;
    size_t pos;
    size_t line;
    size_t column;
    
    // Helper methods
    char current() const;
    char peek(size_t offset = 1) const;
    void advance();
    void skipWhitespace();
    void skipComment();
    
    Token makeToken(TokenType type, std::string value);
    Token readIdentifierOrKeyword();
    Token readNumber();
    Token readString();
    
    bool isAtEnd() const { return pos >= source.size(); }
    bool isDigit(char c) const { return c >= '0' && c <= '9'; }
    bool isAlpha(char c) const { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
    bool isAlphaNumeric(char c) const { return isAlpha(c) || isDigit(c); }
};

} // namespace aurora
