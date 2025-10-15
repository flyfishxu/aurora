#pragma once

#include "aurora/LanguageCore.h"
#include "LSPHandlers.h"
#include "LSPProtocol.h"
#include <string>
#include <map>
#include <nlohmann/json.hpp>

namespace aurora {
namespace lsp {

using json = nlohmann::json;

/// LSP Server - Protocol adapter for Language Core
class LSPServer {
public:
    LSPServer();
    ~LSPServer();
    
    void run();
    
private:
    LanguageCore core_;
    LSPHandlers handlers_;
    bool running_;
    std::map<std::string, std::string> openDocuments_;
    
    // Message handling
    void handleMessage(const json& message);
    void handleRequest(const json& request);
    void handleNotification(const json& notification);
    
    // Document notifications
    void handleInitialized(const json& params);
    void handleExit(const json& params);
    void handleDidOpen(const json& params);
    void handleDidChange(const json& params);
    void handleDidClose(const json& params);
    void handleDidSave(const json& params);
    
    // Communication
    void sendResponse(const json& id, const json& result);
    void sendError(const json& id, int code, const std::string& message);
    void sendNotification(const std::string& method, const json& params);
    void sendDiagnostics(const std::string& uri);
    
    std::string readMessage();
    void writeMessage(const std::string& content);
};

} // namespace lsp
} // namespace aurora
