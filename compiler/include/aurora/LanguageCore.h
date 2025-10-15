#pragma once

#include "aurora/Lexer.h"
#include "aurora/Parser.h"
#include "aurora/AST.h"
#include "aurora/Type.h"
#include "aurora/Diagnostic.h"
#include <memory>
#include <vector>
#include <string>
#include <map>

namespace aurora {

/// Symbol information for IDE features
struct SymbolInfo {
    std::string name;
    std::string type;
    std::string containerName;  // class/namespace
    SourceLocation location;
    enum class Kind {
        Function,
        Variable,
        Class,
        Method,
        Field,
        Parameter,
        Import,
        Package
    } kind;
    
    bool isPublic = true;
    bool isStatic = false;
};

/// Hover information
struct HoverInfo {
    std::string content;      // Documentation/signature
    SourceLocation location;
    bool isValid = false;
};

/// Reference location
struct ReferenceLocation {
    SourceLocation location;
    bool isDefinition = false;
};

/// Completion item
struct CompletionItem {
    std::string label;
    std::string detail;
    std::string documentation;
    enum class Kind {
        Function,
        Variable,
        Class,
        Method,
        Field,
        Keyword,
        Module,
        Snippet
    } kind;
    
    std::string insertText;
};

/// Signature help parameter
struct ParameterInfo {
    std::string label;
    std::string documentation;
};

/// Signature information
struct SignatureInfo {
    std::string label;
    std::string documentation;
    std::vector<ParameterInfo> parameters;
    size_t activeParameter = 0;
};

/// Language Core - Reusable language analysis library
class LanguageCore {
public:
    LanguageCore();
    ~LanguageCore();
    
    // Source management
    void setSource(const std::string& filename, const std::string& source);
    void clearSource(const std::string& filename);
    
    // Analysis
    bool analyze(const std::string& filename);
    std::vector<Diagnostic> getDiagnostics(const std::string& filename) const;
    
    // Symbol indexing
    std::vector<SymbolInfo> getSymbols(const std::string& filename) const;
    std::vector<SymbolInfo> getWorkspaceSymbols(const std::string& query) const;
    
    // Navigation
    HoverInfo getHover(const std::string& filename, size_t line, size_t column) const;
    std::vector<ReferenceLocation> getDefinition(const std::string& filename, 
                                                   size_t line, size_t column) const;
    std::vector<ReferenceLocation> getReferences(const std::string& filename, 
                                                   size_t line, size_t column) const;
    
    // Code intelligence
    std::vector<CompletionItem> getCompletions(const std::string& filename, 
                                                size_t line, size_t column) const;
    std::vector<SignatureInfo> getSignatureHelp(const std::string& filename,
                                                 size_t line, size_t column) const;
    
    // Formatting
    std::string formatDocument(const std::string& filename) const;
    std::string formatRange(const std::string& filename, 
                            size_t startLine, size_t startCol,
                            size_t endLine, size_t endCol) const;
    
private:
    struct FileData {
        std::string source;
        std::unique_ptr<Lexer> lexer;
        std::vector<std::unique_ptr<Function>> functions;
        std::vector<std::unique_ptr<ClassDecl>> classes;
        std::vector<std::unique_ptr<ImportDecl>> imports;
        std::unique_ptr<PackageDecl> package;
        std::vector<Diagnostic> diagnostics;
        std::vector<SymbolInfo> symbols;
    };
    
    std::map<std::string, FileData> files_;
    
    // Helper methods
    void indexSymbols(const std::string& filename, FileData& data);
    SymbolInfo* findSymbolAt(const std::string& filename, 
                             size_t line, size_t column) const;
    void collectFunctionSymbols(const std::string& filename,
                                const Function* func, 
                                const std::string& container,
                                std::vector<SymbolInfo>& symbols);
    void collectClassSymbols(const std::string& filename,
                            const ClassDecl* cls,
                            std::vector<SymbolInfo>& symbols);
    std::vector<CompletionItem> getKeywordCompletions() const;
    std::vector<CompletionItem> getContextCompletions(
        const std::string& filename, size_t line, size_t column) const;
};

} // namespace aurora


