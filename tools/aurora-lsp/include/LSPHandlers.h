#pragma once

#include "aurora/LanguageCore.h"
#include <nlohmann/json.hpp>

namespace aurora {
namespace lsp {

using json = nlohmann::json;

/// LSP request handlers
class LSPHandlers {
public:
    explicit LSPHandlers(LanguageCore& core) : core_(core) {}
    
    // Lifecycle
    json handleInitialize(const json& params);
    json handleShutdown(const json& params);
    
    // Language features
    json handleHover(const json& params);
    json handleDefinition(const json& params);
    json handleReferences(const json& params);
    json handleCompletion(const json& params);
    json handleSignatureHelp(const json& params);
    json handleDocumentSymbol(const json& params);
    json handleWorkspaceSymbol(const json& params);
    json handleFormatting(const json& params);
    json handleRangeFormatting(const json& params);
    
private:
    LanguageCore& core_;
};

} // namespace lsp
} // namespace aurora


