#include "LSPProtocol.h"

namespace aurora {
namespace lsp {

std::string LSPProtocol::uriToPath(const std::string& uri) {
    if (uri.find("file://") == 0) {
        return uri.substr(7);
    }
    return uri;
}

std::string LSPProtocol::pathToUri(const std::string& path) {
    return "file://" + path;
}

json LSPProtocol::locationToLSP(const SourceLocation& loc) {
    return {
        {"uri", pathToUri(loc.filename)},
        {"range", rangeToLSP(loc)}
    };
}

json LSPProtocol::rangeToLSP(const SourceLocation& loc) {
    return {
        {"start", {
            {"line", loc.line > 0 ? loc.line - 1 : 0},
            {"character", loc.column > 0 ? loc.column - 1 : 0}
        }},
        {"end", {
            {"line", loc.line > 0 ? loc.line - 1 : 0},
            {"character", loc.column + loc.length}
        }}
    };
}

json LSPProtocol::diagnosticToLSP(const Diagnostic& diag) {
    int severity;
    switch (diag.getLevel()) {
        case DiagnosticLevel::Error:
            severity = 1;
            break;
        case DiagnosticLevel::Warning:
            severity = 2;
            break;
        case DiagnosticLevel::Note:
            severity = 3;
            break;
        default:
            severity = 4;
    }
    
    return {
        {"range", rangeToLSP(diag.getLocation())},
        {"severity", severity},
        {"code", diag.getCode()},
        {"source", "aurora"},
        {"message", diag.getMessage()}
    };
}

json LSPProtocol::symbolInfoToLSP(const SymbolInfo& symbol) {
    int kind;
    switch (symbol.kind) {
        case SymbolInfo::Kind::Function:
            kind = 12;
            break;
        case SymbolInfo::Kind::Variable:
            kind = 13;
            break;
        case SymbolInfo::Kind::Class:
            kind = 5;
            break;
        case SymbolInfo::Kind::Method:
            kind = 6;
            break;
        case SymbolInfo::Kind::Field:
            kind = 8;
            break;
        case SymbolInfo::Kind::Parameter:
            kind = 7;
            break;
        case SymbolInfo::Kind::Import:
            kind = 9;
            break;
        case SymbolInfo::Kind::Package:
            kind = 4;
            break;
        default:
            kind = 1;
    }
    
    return {
        {"name", symbol.name},
        {"kind", kind},
        {"location", locationToLSP(symbol.location)},
        {"containerName", symbol.containerName}
    };
}

json LSPProtocol::completionItemToLSP(const CompletionItem& item) {
    int kind;
    switch (item.kind) {
        case CompletionItem::Kind::Function:
            kind = 3;
            break;
        case CompletionItem::Kind::Variable:
            kind = 6;
            break;
        case CompletionItem::Kind::Class:
            kind = 7;
            break;
        case CompletionItem::Kind::Method:
            kind = 2;
            break;
        case CompletionItem::Kind::Field:
            kind = 5;
            break;
        case CompletionItem::Kind::Keyword:
            kind = 14;
            break;
        case CompletionItem::Kind::Module:
            kind = 9;
            break;
        case CompletionItem::Kind::Snippet:
            kind = 15;
            break;
        default:
            kind = 1;
    }
    
    return {
        {"label", item.label},
        {"kind", kind},
        {"detail", item.detail},
        {"documentation", item.documentation},
        {"insertText", item.insertText}
    };
}

} // namespace lsp
} // namespace aurora