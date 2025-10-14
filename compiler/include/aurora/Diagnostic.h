#pragma once

#include <string>
#include <vector>

namespace aurora {

/// Diagnostic severity levels
enum class DiagnosticLevel {
    Note,       // Informational note
    Warning,    // Warning, doesn't stop compilation
    Error,      // Error, stops compilation
    Fatal       // Fatal error, immediate termination
};

/// Source location information
struct SourceLocation {
    std::string filename;
    size_t line;
    size_t column;
    size_t length;  // Length of the problematic token/expression
    
    SourceLocation(std::string file = "<input>", size_t l = 0, size_t c = 0, size_t len = 1)
        : filename(std::move(file)), line(l), column(c), length(len) {}
    
    bool isValid() const { return line > 0 && column > 0; }
};

/// Diagnostic message with error code
class Diagnostic {
public:
    Diagnostic(DiagnosticLevel level, std::string code, std::string message, 
               SourceLocation loc)
        : level_(level), code_(std::move(code)), message_(std::move(message)), 
          location_(std::move(loc)) {}
    
    // Add a note to provide additional context
    void addNote(const std::string& note, SourceLocation loc = SourceLocation()) {
        notes_.push_back({note, loc});
    }
    
    // Add a suggestion for fixing the error
    void addSuggestion(const std::string& suggestion) {
        suggestions_.push_back(suggestion);
    }
    
    DiagnosticLevel getLevel() const { return level_; }
    const std::string& getCode() const { return code_; }
    const std::string& getMessage() const { return message_; }
    const SourceLocation& getLocation() const { return location_; }
    const std::vector<std::pair<std::string, SourceLocation>>& getNotes() const { 
        return notes_; 
    }
    const std::vector<std::string>& getSuggestions() const { return suggestions_; }
    
private:
    DiagnosticLevel level_;
    std::string code_;
    std::string message_;
    SourceLocation location_;
    std::vector<std::pair<std::string, SourceLocation>> notes_;
    std::vector<std::string> suggestions_;
};

/// Diagnostic engine for collecting and reporting errors
class DiagnosticEngine {
public:
    DiagnosticEngine(bool debug_mode = false) 
        : debug_mode_(debug_mode), use_colors_(false), error_count_(0), warning_count_(0) {}
    
    // Report diagnostics
    void report(const Diagnostic& diag);
    
    // Convenience methods for creating diagnostics
    void reportError(const std::string& code, const std::string& message, 
                     const SourceLocation& loc);
    void reportWarning(const std::string& code, const std::string& message, 
                       const SourceLocation& loc);
    void reportNote(const std::string& message, const SourceLocation& loc);
    
    // Set source code for better error display
    void setSourceCode(const std::string& source) { source_code_ = source; }
    void setFilename(const std::string& filename) { current_filename_ = filename; }
    
    // Query state
    bool hasErrors() const { return error_count_ > 0; }
    size_t getErrorCount() const { return error_count_; }
    size_t getWarningCount() const { return warning_count_; }
    
    // Debug mode
    void setDebugMode(bool enable) { debug_mode_ = enable; }
    bool isDebugMode() const { return debug_mode_; }
    
    // Color control
    void setUseColors(bool enable) { use_colors_ = enable; }
    bool useColors() const { return use_colors_; }
    
    // Print summary
    void printSummary() const;
    
    // Clear all diagnostics
    void clear() {
        diagnostics_.clear();
        error_count_ = 0;
        warning_count_ = 0;
    }
    
private:
    bool debug_mode_;
    bool use_colors_;
    size_t error_count_;
    size_t warning_count_;
    std::string source_code_;
    std::string current_filename_;
    std::vector<Diagnostic> diagnostics_;
    
    // Helper methods for formatted output
    void printDiagnostic(const Diagnostic& diag) const;
    void printSourceSnippet(const SourceLocation& loc) const;
    std::string getLevelString(DiagnosticLevel level) const;
    std::string getLevelColor(DiagnosticLevel level) const;
};

// Global diagnostic engine instance
DiagnosticEngine& getDiagnosticEngine();

} // namespace aurora

