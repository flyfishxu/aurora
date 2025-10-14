#include "aurora/CrashHandler.h"
#include "aurora/Logger.h"
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>
#include <cstdlib>

namespace aurora {

static void signalHandler(int sig) {
    auto& logger = Logger::instance();
    
    // Get signal name
    const char* signame = "UNKNOWN";
    switch(sig) {
        case SIGSEGV: signame = "SIGSEGV (Segmentation Fault)"; break;
        case SIGABRT: signame = "SIGABRT (Abort)"; break;
        case SIGFPE:  signame = "SIGFPE (Floating Point Exception)"; break;
        case SIGILL:  signame = "SIGILL (Illegal Instruction)"; break;
        case SIGBUS:  signame = "SIGBUS (Bus Error)"; break;
    }
    
    logger.fatal("===============================================");
    logger.fatal("FATAL: Caught signal " + std::string(signame));
    logger.fatal("===============================================");
    
    // Get stack trace
    void* callstack[128];
    int frames = backtrace(callstack, 128);
    char** strs = backtrace_symbols(callstack, frames);
    
    logger.fatal("Stack trace:");
    for (int i = 0; i < frames; ++i) {
        logger.fatal("  " + std::string(strs[i]));
    }
    free(strs);
    
    logger.fatal("===============================================");
    logger.fatal("This is likely a bug in the Aurora compiler.");
    logger.fatal("Please report this with the code that caused it.");
    logger.fatal("===============================================");
    
    std::exit(128 + sig);
}

void setupCrashHandler() {
    signal(SIGSEGV, signalHandler);
    signal(SIGABRT, signalHandler);
    signal(SIGFPE, signalHandler);
    signal(SIGILL, signalHandler);
    signal(SIGBUS, signalHandler);
}

bool verifyModule(void* module_ptr, bool abortOnError) {
    auto* module = static_cast<llvm::Module*>(module_ptr);
    auto& logger = Logger::instance();
    
    std::string error_msg;
    llvm::raw_string_ostream error_stream(error_msg);
    
    bool has_errors = llvm::verifyModule(*module, &error_stream);
    
    if (has_errors) {
        error_stream.flush();
        logger.error("LLVM Module Verification Failed!");
        logger.error("===============================================");
        logger.error(error_msg);
        logger.error("===============================================");
        
        if (abortOnError) {
            std::exit(1);
        }
        return false;
    }
    
    if (logger.isEnabled(LogLevel::Debug)) {
        logger.debug("Module verification passed", "Codegen");
    }
    
    return true;
}

} // namespace aurora

