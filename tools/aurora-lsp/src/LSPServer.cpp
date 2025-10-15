#include "LSPServer.h"
#include "aurora/Logger.h"
#include <iostream>
#include <sstream>

namespace aurora {
namespace lsp {

LSPServer::LSPServer() : handlers_(core_), running_(false) {
    Logger::instance().debug("LSP Server initialized");
}

LSPServer::~LSPServer() = default;

void LSPServer::run() {
    running_ = true;
    Logger::instance().debug("LSP Server started");
    
    while (running_) {
        try {
            std::string message = readMessage();
            if (message.empty()) {
                break;
            }
            
            json j = json::parse(message);
            handleMessage(j);
            
        } catch (const std::exception& e) {
            Logger::instance().error(std::string("Error processing message: ") + e.what());
        }
    }
    
    Logger::instance().debug("LSP Server stopped");
}

void LSPServer::handleMessage(const json& message) {
    if (message.contains("method")) {
        if (message.contains("id")) {
            handleRequest(message);
        } else {
            handleNotification(message);
        }
    }
}

void LSPServer::handleRequest(const json& request) {
    std::string method = request["method"];
    json id = request["id"];
    json params = request.value("params", json::object());
    
    Logger::instance().debug("Request: " + method);
    
    try {
        json result;
        
        // Delegate to handlers
        if (method == "initialize") {
            result = handlers_.handleInitialize(params);
        } else if (method == "shutdown") {
            result = handlers_.handleShutdown(params);
        } else if (method == "textDocument/hover") {
            result = handlers_.handleHover(params);
        } else if (method == "textDocument/definition") {
            result = handlers_.handleDefinition(params);
        } else if (method == "textDocument/references") {
            result = handlers_.handleReferences(params);
        } else if (method == "textDocument/completion") {
            result = handlers_.handleCompletion(params);
        } else if (method == "textDocument/signatureHelp") {
            result = handlers_.handleSignatureHelp(params);
        } else if (method == "textDocument/documentSymbol") {
            result = handlers_.handleDocumentSymbol(params);
        } else if (method == "workspace/symbol") {
            result = handlers_.handleWorkspaceSymbol(params);
        } else if (method == "textDocument/formatting") {
            result = handlers_.handleFormatting(params);
        } else if (method == "textDocument/rangeFormatting") {
            result = handlers_.handleRangeFormatting(params);
        } else {
            sendError(id, -32601, "Method not found: " + method);
            return;
        }
        
        sendResponse(id, result);
        
    } catch (const std::exception& e) {
        sendError(id, -32603, std::string("Internal error: ") + e.what());
    }
}

void LSPServer::handleNotification(const json& notification) {
    std::string method = notification["method"];
    json params = notification.value("params", json::object());
    
    Logger::instance().debug("Notification: " + method);
    
    try {
        if (method == "initialized") {
            handleInitialized(params);
        } else if (method == "exit") {
            handleExit(params);
        } else if (method == "textDocument/didOpen") {
            handleDidOpen(params);
        } else if (method == "textDocument/didChange") {
            handleDidChange(params);
        } else if (method == "textDocument/didClose") {
            handleDidClose(params);
        } else if (method == "textDocument/didSave") {
            handleDidSave(params);
        }
    } catch (const std::exception& e) {
        Logger::instance().error(std::string("Error handling notification: ") + e.what());
    }
}

void LSPServer::handleInitialized(const json& params) {
    Logger::instance().debug("Server initialized");
}

void LSPServer::handleExit(const json& params) {
    running_ = false;
}

void LSPServer::handleDidOpen(const json& params) {
    std::string uri = params["textDocument"]["uri"];
    std::string text = params["textDocument"]["text"];
    
    std::string path = LSPProtocol::uriToPath(uri);
    openDocuments_[uri] = text;
    
    core_.setSource(path, text);
    core_.analyze(path);
    
    sendDiagnostics(uri);
}

void LSPServer::handleDidChange(const json& params) {
    std::string uri = params["textDocument"]["uri"];
    json changes = params["contentChanges"];
    
    // Full document sync
    if (!changes.empty() && changes[0].contains("text")) {
        std::string text = changes[0]["text"];
        openDocuments_[uri] = text;
        
        std::string path = LSPProtocol::uriToPath(uri);
        core_.setSource(path, text);
        core_.analyze(path);
        
        sendDiagnostics(uri);
    }
}

void LSPServer::handleDidClose(const json& params) {
    std::string uri = params["textDocument"]["uri"];
    
    openDocuments_.erase(uri);
    core_.clearSource(LSPProtocol::uriToPath(uri));
}

void LSPServer::handleDidSave(const json& params) {
    std::string uri = params["textDocument"]["uri"];
    std::string path = LSPProtocol::uriToPath(uri);
    
    core_.analyze(path);
    sendDiagnostics(uri);
}

void LSPServer::sendResponse(const json& id, const json& result) {
    json response = {
        {"jsonrpc", "2.0"},
        {"id", id},
        {"result", result}
    };
    
    writeMessage(response.dump());
}

void LSPServer::sendError(const json& id, int code, const std::string& message) {
    json response = {
        {"jsonrpc", "2.0"},
        {"id", id},
        {"error", {
            {"code", code},
            {"message", message}
        }}
    };
    
    writeMessage(response.dump());
}

void LSPServer::sendNotification(const std::string& method, const json& params) {
    json notification = {
        {"jsonrpc", "2.0"},
        {"method", method},
        {"params", params}
    };
    
    writeMessage(notification.dump());
}

void LSPServer::sendDiagnostics(const std::string& uri) {
    std::string path = LSPProtocol::uriToPath(uri);
    auto diagnostics = core_.getDiagnostics(path);
    
    json diags = json::array();
    for (const auto& diag : diagnostics) {
        diags.push_back(LSPProtocol::diagnosticToLSP(diag));
    }
    
    sendNotification("textDocument/publishDiagnostics", {
        {"uri", uri},
        {"diagnostics", diags}
    });
}

std::string LSPServer::readMessage() {
    std::string headers;
    std::string line;
    size_t contentLength = 0;
    
    // Read headers
    while (std::getline(std::cin, line)) {
        if (line == "\r" || line.empty()) {
            break;
        }
        
        if (line.find("Content-Length:") == 0) {
            contentLength = std::stoul(line.substr(15));
        }
    }
    
    if (contentLength == 0) {
        return "";
    }
    
    // Read content
    std::string content(contentLength, '\0');
    std::cin.read(&content[0], contentLength);
    
    return content;
}

void LSPServer::writeMessage(const std::string& content) {
    std::ostringstream oss;
    oss << "Content-Length: " << content.length() << "\r\n";
    oss << "\r\n";
    oss << content;
    
    std::cout << oss.str() << std::flush;
}

} // namespace lsp
} // namespace aurora
