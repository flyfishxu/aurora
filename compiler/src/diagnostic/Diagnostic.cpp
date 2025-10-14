#include "aurora/Diagnostic.h"
#include "aurora/Colors.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace aurora {

static std::unique_ptr<DiagnosticEngine> g_diagnostic_engine;

DiagnosticEngine& getDiagnosticEngine() {
    if (!g_diagnostic_engine) {
        g_diagnostic_engine = std::make_unique<DiagnosticEngine>();
    }
    return *g_diagnostic_engine;
}

std::string DiagnosticEngine::getLevelString(DiagnosticLevel level) const {
    switch (level) {
        case DiagnosticLevel::Note:    return "Note";
        case DiagnosticLevel::Warning: return "Warning";
        case DiagnosticLevel::Error:   return "Error";
        case DiagnosticLevel::Fatal:   return "Fatal Error";
    }
    return "Unknown";
}

std::string DiagnosticEngine::getLevelColor(DiagnosticLevel level) const {
    if (!use_colors_) return "";
    
    switch (level) {
        case DiagnosticLevel::Note:    return Color::Cyan;
        case DiagnosticLevel::Warning: return Color::Yellow;
        case DiagnosticLevel::Error:   return Color::Red;
        case DiagnosticLevel::Fatal:   return Color::Red;
    }
    return Color::Reset;
}

void DiagnosticEngine::printSourceSnippet(const SourceLocation& loc) const {
    if (!loc.isValid() || source_code_.empty()) {
        return;
    }
    
    // Split source into lines
    std::vector<std::string> lines;
    std::istringstream stream(source_code_);
    std::string line;
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    
    if (loc.line == 0 || loc.line > lines.size()) {
        return;
    }
    
    // Print line number and source line
    size_t line_idx = loc.line - 1;
    if (use_colors_) {
        std::cerr << Color::Blue << std::setw(4) << loc.line << " |" << Color::Reset;
    } else {
        std::cerr << std::setw(4) << loc.line << " |";
    }
    std::cerr << " " << lines[line_idx] << "\n";
    
    // Print the caret pointing to the error location
    if (use_colors_) {
        std::cerr << Color::Blue << "     |" << Color::Reset << " ";
    } else {
        std::cerr << "     | ";
    }
    
    // Add spaces to align with the error column
    for (size_t i = 1; i < loc.column; ++i) {
        std::cerr << " ";
    }
    
    // Print carets (^^) under the problematic code
    if (use_colors_) std::cerr << Color::Red;
    for (size_t i = 0; i < loc.length; ++i) {
        std::cerr << "^";
    }
    if (use_colors_) std::cerr << Color::Reset;
    std::cerr << "\n";
}

void DiagnosticEngine::printDiagnostic(const Diagnostic& diag) const {
    const auto& loc = diag.getLocation();
    
    // Print error header: Error[E0001]: Message
    if (use_colors_) {
        std::cerr << getLevelColor(diag.getLevel()) << Color::Bold 
                  << getLevelString(diag.getLevel());
    } else {
        std::cerr << getLevelString(diag.getLevel());
    }
    
    if (!diag.getCode().empty()) {
        std::cerr << "[" << diag.getCode() << "]";
    }
    
    if (use_colors_) {
        std::cerr << ": " << Color::Reset << Color::Bold 
                  << diag.getMessage() << Color::Reset << "\n";
    } else {
        std::cerr << ": " << diag.getMessage() << "\n";
    }
    
    // Print location: --> filename:line:column
    if (loc.isValid()) {
        if (use_colors_) {
            std::cerr << Color::Blue << "  --> " << Color::Reset;
        } else {
            std::cerr << "  --> ";
        }
        std::cerr << loc.filename << ":" << loc.line << ":" << loc.column << "\n";
        
        // Print separator
        if (use_colors_) {
            std::cerr << Color::Blue << "   |" << Color::Reset << "\n";
        } else {
            std::cerr << "   |\n";
        }
        
        // Print source snippet with caret
        printSourceSnippet(loc);
    }
    
    // Print notes
    for (const auto& note : diag.getNotes()) {
        if (use_colors_) {
            std::cerr << Color::Cyan << "   = note: " << Color::Reset;
        } else {
            std::cerr << "   = note: ";
        }
        std::cerr << note.first << "\n";
        if (note.second.isValid()) {
            printSourceSnippet(note.second);
        }
    }
    
    // Print suggestions
    for (const auto& suggestion : diag.getSuggestions()) {
        if (use_colors_) {
            std::cerr << Color::Green << "   = help: " << Color::Reset;
        } else {
            std::cerr << "   = help: ";
        }
        std::cerr << suggestion << "\n";
    }
    
    std::cerr << "\n";
}

void DiagnosticEngine::report(const Diagnostic& diag) {
    diagnostics_.push_back(diag);
    
    switch (diag.getLevel()) {
        case DiagnosticLevel::Error:
        case DiagnosticLevel::Fatal:
            error_count_++;
            break;
        case DiagnosticLevel::Warning:
            warning_count_++;
            break;
        default:
            break;
    }
    
    // Print immediately
    printDiagnostic(diag);
    
    // In debug mode, print stack trace hint
    if (debug_mode_ && diag.getLevel() >= DiagnosticLevel::Error) {
        if (use_colors_) {
            std::cerr << Color::Gray << "   [Debug mode enabled - compiler internal state preserved]" 
                      << Color::Reset << "\n\n";
        } else {
            std::cerr << "   [Debug mode enabled - compiler internal state preserved]\n\n";
        }
    }
}

void DiagnosticEngine::reportError(const std::string& code, const std::string& message, 
                                   const SourceLocation& loc) {
    Diagnostic diag(DiagnosticLevel::Error, code, message, loc);
    report(diag);
}

void DiagnosticEngine::reportWarning(const std::string& code, const std::string& message, 
                                     const SourceLocation& loc) {
    Diagnostic diag(DiagnosticLevel::Warning, code, message, loc);
    report(diag);
}

void DiagnosticEngine::reportNote(const std::string& message, const SourceLocation& loc) {
    Diagnostic diag(DiagnosticLevel::Note, "", message, loc);
    report(diag);
}

void DiagnosticEngine::printSummary() const {
    if (error_count_ > 0 || warning_count_ > 0) {
        if (use_colors_) std::cerr << Color::Bold;
        if (error_count_ > 0) {
            if (use_colors_) std::cerr << Color::Red;
            std::cerr << "✗ " << error_count_ << " error(s)";
            if (warning_count_ > 0) {
                std::cerr << ", ";
            }
        }
        if (warning_count_ > 0) {
            if (use_colors_) std::cerr << Color::Yellow;
            std::cerr << warning_count_ << " warning(s)";
        }
        if (use_colors_) std::cerr << Color::Reset;
        std::cerr << "\n";
    } else {
        if (use_colors_) {
            std::cerr << Color::Green << Color::Bold 
                      << "✓ No errors or warnings" 
                      << Color::Reset << "\n";
        } else {
            std::cerr << "✓ No errors or warnings\n";
        }
    }
}

} // namespace aurora

