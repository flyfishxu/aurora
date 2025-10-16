#include "LSPHandlers.h"
#include "LSPProtocol.h"

namespace aurora {
namespace lsp {

json LSPHandlers::handleInitialize(const json& params) {
    return {
        {"capabilities", {
            {"textDocumentSync", {
                {"openClose", true},
                {"change", 2}, // Incremental
                {"save", true}
            }},
            {"hoverProvider", true},
            {"definitionProvider", true},
            {"referencesProvider", true},
            {"completionProvider", {
                {"triggerCharacters", json::array({".", ":", ">"})}
            }},
            {"signatureHelpProvider", {
                {"triggerCharacters", json::array({"(", ","})}
            }},
            {"documentSymbolProvider", true},
            {"workspaceSymbolProvider", true},
            {"documentFormattingProvider", true},
            {"documentRangeFormattingProvider", true}
        }},
        {"serverInfo", {
            {"name", "aurora-lsp"},
            {"version", "0.6.3"}
        }}
    };
}

json LSPHandlers::handleShutdown(const json& params) {
    return nullptr;
}

json LSPHandlers::handleHover(const json& params) {
    std::string uri = params["textDocument"]["uri"];
    size_t line = params["position"]["line"];
    size_t character = params["position"]["character"];
    
    std::string path = LSPProtocol::uriToPath(uri);
    auto hover = core_.getHover(path, line + 1, character);
    
    if (!hover.isValid) {
        return nullptr;
    }
    
    return {
        {"contents", {
            {"kind", "markdown"},
            {"value", hover.content}
        }},
        {"range", LSPProtocol::rangeToLSP(hover.location)}
    };
}

json LSPHandlers::handleDefinition(const json& params) {
    std::string uri = params["textDocument"]["uri"];
    size_t line = params["position"]["line"];
    size_t character = params["position"]["character"];
    
    std::string path = LSPProtocol::uriToPath(uri);
    auto refs = core_.getDefinition(path, line + 1, character);
    
    if (refs.empty()) {
        return nullptr;
    }
    
    json locations = json::array();
    for (const auto& ref : refs) {
        locations.push_back({
            {"uri", LSPProtocol::pathToUri(ref.location.filename)},
            {"range", LSPProtocol::rangeToLSP(ref.location)}
        });
    }
    
    return locations;
}

json LSPHandlers::handleReferences(const json& params) {
    std::string uri = params["textDocument"]["uri"];
    size_t line = params["position"]["line"];
    size_t character = params["position"]["character"];
    
    std::string path = LSPProtocol::uriToPath(uri);
    auto refs = core_.getReferences(path, line + 1, character);
    
    json locations = json::array();
    for (const auto& ref : refs) {
        locations.push_back({
            {"uri", LSPProtocol::pathToUri(ref.location.filename)},
            {"range", LSPProtocol::rangeToLSP(ref.location)}
        });
    }
    
    return locations;
}

json LSPHandlers::handleCompletion(const json& params) {
    std::string uri = params["textDocument"]["uri"];
    size_t line = params["position"]["line"];
    size_t character = params["position"]["character"];
    
    std::string path = LSPProtocol::uriToPath(uri);
    auto items = core_.getCompletions(path, line + 1, character);
    
    json completions = json::array();
    for (const auto& item : items) {
        completions.push_back(LSPProtocol::completionItemToLSP(item));
    }
    
    return {{"isIncomplete", false}, {"items", completions}};
}

json LSPHandlers::handleSignatureHelp(const json& params) {
    std::string uri = params["textDocument"]["uri"];
    size_t line = params["position"]["line"];
    size_t character = params["position"]["character"];
    
    std::string path = LSPProtocol::uriToPath(uri);
    auto sigs = core_.getSignatureHelp(path, line + 1, character);
    
    json signatures = json::array();
    for (const auto& sig : sigs) {
        json params_json = json::array();
        for (const auto& param : sig.parameters) {
            params_json.push_back({
                {"label", param.label},
                {"documentation", param.documentation}
            });
        }
        
        signatures.push_back({
            {"label", sig.label},
            {"documentation", sig.documentation},
            {"parameters", params_json},
            {"activeParameter", sig.activeParameter}
        });
    }
    
    return {
        {"signatures", signatures},
        {"activeSignature", 0},
        {"activeParameter", sigs.empty() ? 0 : sigs[0].activeParameter}
    };
}

json LSPHandlers::handleDocumentSymbol(const json& params) {
    std::string uri = params["textDocument"]["uri"];
    std::string path = LSPProtocol::uriToPath(uri);
    
    auto symbols = core_.getSymbols(path);
    
    json result = json::array();
    for (const auto& symbol : symbols) {
        result.push_back(LSPProtocol::symbolInfoToLSP(symbol));
    }
    
    return result;
}

json LSPHandlers::handleWorkspaceSymbol(const json& params) {
    std::string query = params.value("query", "");
    
    auto symbols = core_.getWorkspaceSymbols(query);
    
    json result = json::array();
    for (const auto& symbol : symbols) {
        json item = LSPProtocol::symbolInfoToLSP(symbol);
        item["location"] = LSPProtocol::locationToLSP(symbol.location);
        result.push_back(item);
    }
    
    return result;
}

json LSPHandlers::handleFormatting(const json& params) {
    std::string uri = params["textDocument"]["uri"];
    std::string path = LSPProtocol::uriToPath(uri);
    
    std::string formatted = core_.formatDocument(path);
    
    if (formatted.empty()) {
        return json::array();
    }
    
    return json::array({{
        {"range", {
            {"start", {{"line", 0}, {"character", 0}}},
            {"end", {{"line", 999999}, {"character", 0}}}
        }},
        {"newText", formatted}
    }});
}

json LSPHandlers::handleRangeFormatting(const json& params) {
    std::string uri = params["textDocument"]["uri"];
    json range = params["range"];
    std::string path = LSPProtocol::uriToPath(uri);
    
    size_t startLine = range["start"]["line"];
    size_t startChar = range["start"]["character"];
    size_t endLine = range["end"]["line"];
    size_t endChar = range["end"]["character"];
    
    std::string formatted = core_.formatRange(path, startLine, startChar, 
                                               endLine, endChar);
    
    if (formatted.empty()) {
        return json::array();
    }
    
    return json::array({{
        {"range", range},
        {"newText", formatted}
    }});
}

} // namespace lsp
} // namespace aurora
