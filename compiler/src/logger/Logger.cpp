#include "aurora/Logger.h"
#include "aurora/Colors.h"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace aurora {

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

std::string Logger::getLevelString(LogLevel level) const {
    switch (level) {
        case LogLevel::Trace:   return "TRACE";
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO ";
        case LogLevel::Warning: return "WARN ";
        case LogLevel::Error:   return "ERROR";
        case LogLevel::Fatal:   return "FATAL";
        case LogLevel::Off:     return "OFF  ";
    }
    return "?????";
}

std::string Logger::getLevelColor(LogLevel level) const {
    if (!config_.show_colors) return "";
    
    switch (level) {
        case LogLevel::Trace:   return Color::Gray;
        case LogLevel::Debug:   return Color::Cyan;
        case LogLevel::Info:    return Color::Green;
        case LogLevel::Warning: return Color::Yellow;
        case LogLevel::Error:   return Color::Red;
        case LogLevel::Fatal:   return Color::Red;
        case LogLevel::Off:     return Color::Reset;
    }
    return Color::Reset;
}

std::string Logger::getCurrentTime() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

void Logger::log(LogLevel level, const std::string& message, const std::string& component) {
    if (level < config_.level) return;
    
    std::ostream& out = (level >= LogLevel::Error) ? std::cerr : std::cout;
    
    // Compact mode for CI/CD
    if (config_.compact_mode) {
        out << "[" << getLevelString(level) << "] " << message << "\n";
        return;
    }
    
    // Colorized output
    if (config_.show_colors) {
        out << getLevelColor(level) << Color::Bold;
    }
    
    // Timestamp
    if (config_.show_timestamps) {
        out << "[" << getCurrentTime() << "] ";
    }
    
    // Level
    out << "[" << getLevelString(level) << "]";
    
    if (config_.show_colors) {
        out << Color::Reset << getLevelColor(level);
    }
    
    // Component
    if (!component.empty()) {
        out << " [" << component << "]";
    }
    
    // Message
    out << " " << message;
    
    if (config_.show_colors) {
        out << Color::Reset;
    }
    
    out << "\n";
}

void Logger::trace(const std::string& message, const std::string& component) {
    log(LogLevel::Trace, message, component);
}

void Logger::debug(const std::string& message, const std::string& component) {
    log(LogLevel::Debug, message, component);
}

void Logger::info(const std::string& message, const std::string& component) {
    log(LogLevel::Info, message, component);
}

void Logger::warning(const std::string& message, const std::string& component) {
    log(LogLevel::Warning, message, component);
}

void Logger::error(const std::string& message, const std::string& component) {
    log(LogLevel::Error, message, component);
}

void Logger::fatal(const std::string& message, const std::string& component) {
    log(LogLevel::Fatal, message, component);
}

void Logger::phaseStart(const std::string& phase_name) {
    if (config_.level <= LogLevel::Info) {
        if (config_.show_colors) {
            std::cout << Color::Blue << Color::Bold << "▶ " << Color::Reset;
        }
        std::cout << "[Aurora] Phase: " << phase_name << "\n";
    }
}

void Logger::phaseEnd(const std::string& phase_name, bool success) {
    if (config_.level <= LogLevel::Debug) {
        if (config_.show_colors) {
            std::cout << (success ? Color::Green : Color::Red) << Color::Bold;
            std::cout << (success ? "✓" : "✗") << Color::Reset << " ";
        }
        std::cout << "Phase " << phase_name << " " 
                  << (success ? "completed" : "failed") << "\n";
    }
}

void Logger::logAST(const std::string& ast_dump) {
    if (config_.level > LogLevel::Trace) return;
    
    std::cout << Color::Magenta << "=== AST Dump ===" << Color::Reset << "\n";
    std::cout << ast_dump << "\n";
    std::cout << Color::Magenta << "================" << Color::Reset << "\n";
}

void Logger::logToken(const std::string& token_info) {
    if (config_.level > LogLevel::Trace) return;
    
    trace(token_info, "Lexer");
}

void Logger::logLLVMIR(const std::string& ir_dump) {
    if (config_.level > LogLevel::Trace) return;
    
    std::cout << Color::Cyan << "=== LLVM IR ===" << Color::Reset << "\n";
    std::cout << ir_dump << "\n";
    std::cout << Color::Cyan << "===============" << Color::Reset << "\n";
}

void Logger::logStatistics(const std::string& stats) {
    if (config_.level > LogLevel::Info) return;
    
    std::cout << Color::Yellow << "=== Statistics ===" << Color::Reset << "\n";
    std::cout << stats << "\n";
    std::cout << Color::Yellow << "==================" << Color::Reset << "\n";
}

// Timer implementation
Logger::Timer::Timer(const std::string& name, Logger& logger)
    : name_(name), logger_(logger), 
      start_(std::chrono::high_resolution_clock::now()) {
    if (logger_.isEnabled(LogLevel::Debug)) {
        logger_.debug("Starting: " + name_, "Timer");
    }
}

Logger::Timer::~Timer() {
    if (!logger_.isEnabled(LogLevel::Debug)) return;
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
    
    std::stringstream ss;
    ss << "Completed: " << name_ << " in ";
    
    if (duration.count() < 1000) {
        ss << duration.count() << " μs";
    } else if (duration.count() < 1000000) {
        ss << std::fixed << std::setprecision(2) 
           << (duration.count() / 1000.0) << " ms";
    } else {
        ss << std::fixed << std::setprecision(2) 
           << (duration.count() / 1000000.0) << " s";
    }
    
    logger_.debug(ss.str(), "Timer");
}

} // namespace aurora

