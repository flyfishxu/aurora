#include "LSPServer.h"
#include "aurora/Logger.h"
#include <iostream>

int main(int argc, char** argv) {
    // Check if --stdio flag is present
    bool stdio_mode = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--stdio") {
            stdio_mode = true;
            break;
        }
    }
    
    if (!stdio_mode) {
        std::cerr << "Aurora Language Server\n";
        std::cerr << "Usage: aurora-lsp --stdio\n";
        return 1;
    }
    
    try {
        // Initialize logger for LSP (disabled by default)
        aurora::Logger::instance().setLevel(aurora::LogLevel::Off);
        
        // Create and run LSP server
        aurora::lsp::LSPServer server;
        server.run();
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}

