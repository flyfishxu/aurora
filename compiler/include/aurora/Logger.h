#pragma once

#include <string>
#include <chrono>

namespace aurora {

/// Log levels for different types of messages
enum class LogLevel {
    Trace,      // Very detailed information, typically only for diagnosis
    Debug,      // Detailed information useful for debugging
    Info,       // General informational messages
    Warning,    // Warning messages that don't stop compilation
    Error,      // Error messages
    Fatal,      // Fatal errors that cause immediate termination
    Off         // No logging
};

/// Logger configuration
struct LoggerConfig {
    LogLevel level = LogLevel::Off;  // Default: no output unless explicitly enabled
    bool show_timestamps = false;
    bool show_colors = false;  // Disable colors by default
    bool show_source_location = false;
    bool compact_mode = false;  // Compact output for CI/CD
};

/// Centralized logging system for AuroraLang compiler
class Logger {
public:
    static Logger& instance();
    
    // Configuration
    void setLevel(LogLevel level) { config_.level = level; }
    void setConfig(const LoggerConfig& config) { config_ = config; }
    LogLevel getLevel() const { return config_.level; }
    
    // Check if a log level is enabled
    bool isEnabled(LogLevel level) const {
        return level >= config_.level;
    }
    
    // Logging methods
    void trace(const std::string& message, const std::string& component = "");
    void debug(const std::string& message, const std::string& component = "");
    void info(const std::string& message, const std::string& component = "");
    void warning(const std::string& message, const std::string& component = "");
    void error(const std::string& message, const std::string& component = "");
    void fatal(const std::string& message, const std::string& component = "");
    
    // Specialized logging for compiler phases
    void phaseStart(const std::string& phase_name);
    void phaseEnd(const std::string& phase_name, bool success = true);
    
    // Performance tracking
    class Timer {
    public:
        Timer(const std::string& name, Logger& logger);
        ~Timer();
        
    private:
        std::string name_;
        Logger& logger_;
        std::chrono::time_point<std::chrono::high_resolution_clock> start_;
    };
    
    Timer startTimer(const std::string& name) {
        return Timer(name, *this);
    }
    
    // Structured data logging
    void logAST(const std::string& ast_dump);
    void logToken(const std::string& token_info);
    void logLLVMIR(const std::string& ir_dump);
    void logStatistics(const std::string& stats);
    
private:
    Logger() = default;
    LoggerConfig config_;
    
    void log(LogLevel level, const std::string& message, const std::string& component);
    std::string getLevelString(LogLevel level) const;
    std::string getLevelColor(LogLevel level) const;
    std::string getCurrentTime() const;
};

// Convenience macros for logging
#define LOG_TRACE(msg) aurora::Logger::instance().trace(msg, __func__)
#define LOG_DEBUG(msg) aurora::Logger::instance().debug(msg, __func__)
#define LOG_INFO(msg) aurora::Logger::instance().info(msg)
#define LOG_WARN(msg) aurora::Logger::instance().warning(msg)
#define LOG_ERROR(msg) aurora::Logger::instance().error(msg)
#define LOG_FATAL(msg) aurora::Logger::instance().fatal(msg)

// Scoped timer for automatic performance measurement
#define SCOPED_TIMER(name) auto _timer_##__LINE__ = aurora::Logger::instance().startTimer(name)

// Component-specific logging
#define LOG_LEXER_DEBUG(msg) if (aurora::Logger::instance().isEnabled(aurora::LogLevel::Debug)) \
    aurora::Logger::instance().debug(msg, "Lexer")
#define LOG_PARSER_DEBUG(msg) if (aurora::Logger::instance().isEnabled(aurora::LogLevel::Debug)) \
    aurora::Logger::instance().debug(msg, "Parser")
#define LOG_CODEGEN_DEBUG(msg) if (aurora::Logger::instance().isEnabled(aurora::LogLevel::Debug)) \
    aurora::Logger::instance().debug(msg, "CodeGen")

} // namespace aurora

