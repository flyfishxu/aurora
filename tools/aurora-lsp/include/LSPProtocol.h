#pragma once

#include "aurora/LanguageCore.h"
#include <nlohmann/json.hpp>

namespace aurora {
namespace lsp {

using json = nlohmann::json;

/// LSP Protocol conversion utilities
class LSPProtocol {
public:
    // URI conversions
    static std::string uriToPath(const std::string& uri);
    static std::string pathToUri(const std::string& path);
    
    // LSP object conversions
    static json locationToLSP(const SourceLocation& loc);
    static json rangeToLSP(const SourceLocation& loc);
    static json diagnosticToLSP(const Diagnostic& diag);
    static json symbolInfoToLSP(const SymbolInfo& symbol);
    static json completionItemToLSP(const CompletionItem& item);
};

} // namespace lsp
} // namespace aurora


