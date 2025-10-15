#include "aurora/Lexer.h"
#include "aurora/Diagnostic.h"
#include <unordered_map>

namespace aurora {

static const std::unordered_map<std::string, TokenType> keywords = {
    {"fn", TokenType::Fn},
    {"extern", TokenType::Extern},
    {"return", TokenType::Return},
    {"let", TokenType::Let},
    {"var", TokenType::Var},
    {"if", TokenType::If},
    {"else", TokenType::Else},
    {"while", TokenType::While},
    {"for", TokenType::For},
    {"loop", TokenType::Loop},
    {"break", TokenType::Break},
    {"continue", TokenType::Continue},
    {"in", TokenType::In},
    {"match", TokenType::Match},
    {"true", TokenType::True},
    {"false", TokenType::False},
    {"null", TokenType::Null},
    {"import", TokenType::Import},
    {"package", TokenType::Package},
    {"class", TokenType::Class},
    {"object", TokenType::Object},
    {"this", TokenType::This},
    {"pub", TokenType::Pub},
    {"priv", TokenType::Priv},
    {"static", TokenType::Static},
    {"constructor", TokenType::Constructor},
    {"int", TokenType::TypeInt},
    {"double", TokenType::TypeDouble},
    {"bool", TokenType::TypeBool},
    {"string", TokenType::TypeString},
    {"void", TokenType::TypeVoid},
};

std::string Token::toString() const {
    return "Token(" + value + ", line: " + std::to_string(line) + 
           ", col: " + std::to_string(column) + ")";
}

Lexer::Lexer(std::string src) 
    : source(std::move(src)), pos(0), line(1), column(1) {}

char Lexer::current() const {
    if (isAtEnd()) return '\0';
    return source[pos];
}

char Lexer::peek(size_t offset) const {
    if (pos + offset >= source.size()) return '\0';
    return source[pos + offset];
}

void Lexer::advance() {
    if (!isAtEnd()) {
        if (source[pos] == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        pos++;
    }
}

void Lexer::skipWhitespace() {
    while (!isAtEnd() && std::isspace(current())) {
        advance();
    }
}

void Lexer::skipComment() {
    // Single-line comment: //
    if (current() == '/' && peek() == '/') {
        while (!isAtEnd() && current() != '\n') {
            advance();
        }
        return;
    }
    
    // Multi-line comment: /* */
    if (current() == '/' && peek() == '*') {
        advance(); // consume '/'
        advance(); // consume '*'
        
        while (!isAtEnd()) {
            if (current() == '*' && peek() == '/') {
                advance(); // consume '*'
                advance(); // consume '/'
                break;
            }
            advance();
        }
        return;
    }
}

Token Lexer::makeToken(TokenType type, std::string value) {
    return Token(type, std::move(value), line, column);
}

Token Lexer::readIdentifierOrKeyword() {
    size_t start = pos;
    size_t startCol = column;
    
    while (isAlphaNumeric(current())) {
        advance();
    }
    
    std::string text = source.substr(start, pos - start);
    
    // Check if it's a keyword
    auto it = keywords.find(text);
    if (it != keywords.end()) {
        return Token(it->second, text, line, startCol);
    }
    
    return Token(TokenType::Identifier, text, line, startCol);
}

Token Lexer::readNumber() {
    size_t start = pos;
    size_t startCol = column;
    bool isDouble = false;
    
    while (isDigit(current())) {
        advance();
    }
    
    // Check for decimal point
    if (current() == '.' && isDigit(peek())) {
        isDouble = true;
        advance(); // consume '.'
        while (isDigit(current())) {
            advance();
        }
    }
    
    std::string text = source.substr(start, pos - start);
    TokenType type = isDouble ? TokenType::DoubleLiteral : TokenType::IntLiteral;
    
    return Token(type, text, line, startCol);
}

Token Lexer::readString() {
    size_t startCol = column;
    advance(); // consume opening quote
    
    std::string value;
    while (!isAtEnd() && current() != '"') {
        if (current() == '\\' && peek() == '"') {
            advance(); // consume backslash
            value += '"';
            advance();
        } else if (current() == '\\' && peek() == 'n') {
            advance();
            value += '\n';
            advance();
        } else if (current() == '\\' && peek() == 't') {
            advance();
            value += '\t';
            advance();
        } else if (current() == '\\' && peek() == '\\') {
            advance();
            value += '\\';
            advance();
        } else {
            value += current();
            advance();
        }
    }
    
    if (isAtEnd()) {
        // Unterminated string - report error using diagnostic system
        auto& diag = getDiagnosticEngine();
        SourceLocation loc("<input>", line, startCol, 1);
        diag.reportError("E1001", "Unterminated string literal", loc);
        return Token(TokenType::Eof, "", line, startCol);
    }
    
    advance(); // consume closing quote
    return Token(TokenType::StringLiteral, value, line, startCol);
}

Token Lexer::nextToken() {
    skipWhitespace();
    
    // Check for comments and skip them
    while (current() == '/' && (peek() == '/' || peek() == '*')) {
        skipComment();
        skipWhitespace();
    }
    
    if (isAtEnd()) {
        return makeToken(TokenType::Eof, "");
    }
    
    size_t startCol = column;
    char c = current();
    
    // Identifiers and keywords
    if (isAlpha(c)) {
        return readIdentifierOrKeyword();
    }
    
    // Numbers
    if (isDigit(c)) {
        return readNumber();
    }
    
    // String literals
    if (c == '"') {
        return readString();
    }
    
    // Two-character operators
    if (c == '=' && peek() == '=') {
        advance(); advance();
        return Token(TokenType::EqualEqual, "==", line, startCol);
    }
    if (c == '!' && peek() == '=') {
        advance(); advance();
        return Token(TokenType::NotEqual, "!=", line, startCol);
    }
    if (c == '<' && peek() == '=') {
        advance(); advance();
        return Token(TokenType::LessEq, "<=", line, startCol);
    }
    if (c == '>' && peek() == '=') {
        advance(); advance();
        return Token(TokenType::GreaterEq, ">=", line, startCol);
    }
    if (c == '&' && peek() == '&') {
        advance(); advance();
        return Token(TokenType::And, "&&", line, startCol);
    }
    if (c == '|' && peek() == '|') {
        advance(); advance();
        return Token(TokenType::Or, "||", line, startCol);
    }
    if (c == '-' && peek() == '>') {
        advance(); advance();
        return Token(TokenType::Arrow, "->", line, startCol);
    }
    if (c == '.' && peek() == '.') {
        advance(); advance();
        return Token(TokenType::DotDot, "..", line, startCol);
    }
    if (c == '?' && peek() == '.') {
        advance(); advance();
        return Token(TokenType::QuestionDot, "?.", line, startCol);
    }
    if (c == '?' && peek() == '?') {
        advance(); advance();
        return Token(TokenType::QuestionQuestion, "??", line, startCol);
    }
    if (c == '<' && peek() == '<') {
        advance(); advance();
        return Token(TokenType::LeftShift, "<<", line, startCol);
    }
    if (c == '>' && peek() == '>') {
        advance(); advance();
        return Token(TokenType::RightShift, ">>", line, startCol);
    }
    
    // Single-character tokens
    advance();
    switch (c) {
        case '+': return Token(TokenType::Plus, "+", line, startCol);
        case '-': return Token(TokenType::Minus, "-", line, startCol);
        case '*': return Token(TokenType::Star, "*", line, startCol);
        case '/': return Token(TokenType::Slash, "/", line, startCol);
        case '%': return Token(TokenType::Percent, "%", line, startCol);
        case '&': return Token(TokenType::Ampersand, "&", line, startCol);
        case '|': return Token(TokenType::Pipe, "|", line, startCol);
        case '^': return Token(TokenType::Caret, "^", line, startCol);
        case '~': return Token(TokenType::Tilde, "~", line, startCol);
        case '<': return Token(TokenType::Less, "<", line, startCol);
        case '>': return Token(TokenType::Greater, ">", line, startCol);
        case '=': return Token(TokenType::Equal, "=", line, startCol);
        case '!': return Token(TokenType::Not, "!", line, startCol);
        case '(': return Token(TokenType::LeftParen, "(", line, startCol);
        case ')': return Token(TokenType::RightParen, ")", line, startCol);
        case '{': return Token(TokenType::LeftBrace, "{", line, startCol);
        case '}': return Token(TokenType::RightBrace, "}", line, startCol);
        case '[': return Token(TokenType::LeftBracket, "[", line, startCol);
        case ']': return Token(TokenType::RightBracket, "]", line, startCol);
        case ',': return Token(TokenType::Comma, ",", line, startCol);
        case ';': return Token(TokenType::Semicolon, ";", line, startCol);
        case ':': return Token(TokenType::Colon, ":", line, startCol);
        case '?': return Token(TokenType::Question, "?", line, startCol);
        case '.': return Token(TokenType::Dot, ".", line, startCol);
        default:
            return Token(TokenType::Eof, std::string(1, c), line, startCol);
    }
}

Token Lexer::peekToken() {
    size_t savedPos = pos;
    size_t savedLine = line;
    size_t savedColumn = column;
    
    Token token = nextToken();
    
    pos = savedPos;
    line = savedLine;
    column = savedColumn;
    
    return token;
}

} // namespace aurora
