#include "aurora/LanguageCore.h"
#include "aurora/Parser.h"
#include "aurora/Logger.h"

namespace aurora {

LanguageCore::LanguageCore() {
    Logger::instance().debug("LanguageCore initialized");
}

LanguageCore::~LanguageCore() = default;

void LanguageCore::setSource(const std::string& filename, const std::string& source) {
    auto& data = files_[filename];
    data.source = source;
    data.diagnostics.clear();
    data.symbols.clear();
    Logger::instance().debug("Source set for file: " + filename);
}

void LanguageCore::clearSource(const std::string& filename) {
    files_.erase(filename);
    Logger::instance().debug("Source cleared for file: " + filename);
}

bool LanguageCore::analyze(const std::string& filename) {
    auto it = files_.find(filename);
    if (it == files_.end()) {
        Logger::instance().error("File not found: " + filename);
        return false;
    }
    
    auto& data = it->second;
    data.diagnostics.clear();
    data.symbols.clear();
    
    try {
        // Lexical analysis
        data.lexer = std::make_unique<Lexer>(data.source);
        
        // Parse
        Parser parser(*data.lexer);
        getDiagnosticEngine().setFilename(filename);
        getDiagnosticEngine().setSourceCode(data.source);
        getDiagnosticEngine().clear();
        
        data.functions = parser.parseProgram();
        data.classes = std::move(parser.getClasses());
        data.imports = std::move(parser.getImports());
        data.package = std::move(parser.getPackage());
        
        // Collect diagnostics
        if (getDiagnosticEngine().hasErrors()) {
            // Convert internal diagnostics to our format
            // For now, we'll create a generic error
            Diagnostic diag(DiagnosticLevel::Error, "PARSE_ERROR", 
                          "Failed to parse file", SourceLocation(filename, 0, 0, 0));
            data.diagnostics.push_back(diag);
        }
        
        // Index symbols
        indexSymbols(filename, data);
        
        Logger::instance().debug("Analysis completed for: " + filename);
        return !getDiagnosticEngine().hasErrors();
        
    } catch (const std::exception& e) {
        Logger::instance().error("Analysis failed for " + filename + ": " + e.what());
        Diagnostic diag(DiagnosticLevel::Error, "INTERNAL_ERROR",
                       std::string("Internal error: ") + e.what(),
                       SourceLocation(filename, 0, 0, 0));
        data.diagnostics.push_back(diag);
        return false;
    }
}

std::vector<Diagnostic> LanguageCore::getDiagnostics(const std::string& filename) const {
    auto it = files_.find(filename);
    if (it == files_.end()) {
        return {};
    }
    return it->second.diagnostics;
}

std::vector<SymbolInfo> LanguageCore::getSymbols(const std::string& filename) const {
    auto it = files_.find(filename);
    if (it == files_.end()) {
        return {};
    }
    return it->second.symbols;
}

std::vector<SymbolInfo> LanguageCore::getWorkspaceSymbols(const std::string& query) const {
    std::vector<SymbolInfo> results;
    
    for (const auto& [filename, data] : files_) {
        for (const auto& symbol : data.symbols) {
            if (query.empty() || symbol.name.find(query) != std::string::npos) {
                results.push_back(symbol);
            }
        }
    }
    
    return results;
}

HoverInfo LanguageCore::getHover(const std::string& filename, 
                                  size_t line, size_t column) const {
    HoverInfo info;
    
    auto* symbol = findSymbolAt(filename, line, column);
    if (!symbol) {
        return info;
    }
    
    info.isValid = true;
    info.location = symbol->location;
    
    // Build hover content
    std::string content;
    switch (symbol->kind) {
        case SymbolInfo::Kind::Function:
            content = "**Function** `" + symbol->name + "`\n\n";
            content += "Type: `" + symbol->type + "`";
            break;
        case SymbolInfo::Kind::Variable:
            content = "**Variable** `" + symbol->name + "`\n\n";
            content += "Type: `" + symbol->type + "`";
            break;
        case SymbolInfo::Kind::Class:
            content = "**Class** `" + symbol->name + "`";
            break;
        case SymbolInfo::Kind::Method:
            content = "**Method** `" + symbol->name + "`\n\n";
            content += "Type: `" + symbol->type + "`\n";
            if (!symbol->containerName.empty()) {
                content += "Container: `" + symbol->containerName + "`";
            }
            break;
        case SymbolInfo::Kind::Field:
            content = "**Field** `" + symbol->name + "`\n\n";
            content += "Type: `" + symbol->type + "`\n";
            if (!symbol->containerName.empty()) {
                content += "Container: `" + symbol->containerName + "`";
            }
            break;
        default:
            content = symbol->name + ": " + symbol->type;
    }
    
    info.content = content;
    return info;
}

std::vector<ReferenceLocation> LanguageCore::getDefinition(
    const std::string& filename, size_t line, size_t column) const {
    
    std::vector<ReferenceLocation> results;
    
    auto* symbol = findSymbolAt(filename, line, column);
    if (!symbol) {
        return results;
    }
    
    ReferenceLocation ref;
    ref.location = symbol->location;
    ref.isDefinition = true;
    results.push_back(ref);
    
    return results;
}

std::vector<ReferenceLocation> LanguageCore::getReferences(
    const std::string& filename, size_t line, size_t column) const {
    
    std::vector<ReferenceLocation> results;
    
    auto* symbol = findSymbolAt(filename, line, column);
    if (!symbol) {
        return results;
    }
    
    // Find all references across all files
    for (const auto& [fname, data] : files_) {
        for (const auto& sym : data.symbols) {
            if (sym.name == symbol->name && sym.type == symbol->type) {
                ReferenceLocation ref;
                ref.location = sym.location;
                ref.isDefinition = (sym.location.filename == symbol->location.filename &&
                                   sym.location.line == symbol->location.line);
                results.push_back(ref);
            }
        }
    }
    
    return results;
}

std::vector<CompletionItem> LanguageCore::getCompletions(
    const std::string& filename, size_t line, size_t column) const {
    
    std::vector<CompletionItem> items;
    
    // Add keywords
    auto keywords = getKeywordCompletions();
    items.insert(items.end(), keywords.begin(), keywords.end());
    
    // Add context-based completions
    auto contextItems = getContextCompletions(filename, line, column);
    items.insert(items.end(), contextItems.begin(), contextItems.end());
    
    return items;
}

std::vector<SignatureInfo> LanguageCore::getSignatureHelp(
    const std::string& filename, size_t line, size_t column) const {
    
    std::vector<SignatureInfo> results;
    
    // TODO: Implement signature help
    // This requires parsing the current expression to find the function call
    
    return results;
}

std::string LanguageCore::formatDocument(const std::string& filename) const {
    auto it = files_.find(filename);
    if (it == files_.end()) {
        return "";
    }
    
    // TODO: Implement formatting
    // For now, return original source
    return it->second.source;
}

std::string LanguageCore::formatRange(const std::string& filename,
                                      size_t startLine, size_t startCol,
                                      size_t endLine, size_t endCol) const {
    // TODO: Implement range formatting
    return "";
}

void LanguageCore::indexSymbols(const std::string& filename, FileData& data) {
    data.symbols.clear();
    
    // Index package
    if (data.package) {
        SymbolInfo info;
        info.name = data.package->getPackageName();
        info.kind = SymbolInfo::Kind::Package;
        info.location = SourceLocation(filename, 1, 1);
        data.symbols.push_back(info);
    }
    
    // Index imports
    for (const auto& import : data.imports) {
        SymbolInfo info;
        info.name = import->getModulePath();
        info.kind = SymbolInfo::Kind::Import;
        info.location = SourceLocation(filename, 1, 1);
        data.symbols.push_back(info);
    }
    
    // Index functions
    for (const auto& func : data.functions) {
        collectFunctionSymbols(filename, func.get(), "", data.symbols);
    }
    
    // Index classes
    for (const auto& cls : data.classes) {
        collectClassSymbols(filename, cls.get(), data.symbols);
    }
    
    Logger::instance().debug("Indexed " + std::to_string(data.symbols.size()) + 
                  " symbols for: " + filename);
}

SymbolInfo* LanguageCore::findSymbolAt(const std::string& filename,
                                        size_t line, size_t column) const {
    auto it = files_.find(filename);
    if (it == files_.end()) {
        return nullptr;
    }
    
    const auto& data = it->second;
    for (auto& symbol : const_cast<std::vector<SymbolInfo>&>(data.symbols)) {
        if (symbol.location.line == line && 
            symbol.location.column <= column &&
            column < symbol.location.column + symbol.name.length()) {
            return &symbol;
        }
    }
    
    return nullptr;
}

void LanguageCore::collectFunctionSymbols(const std::string& filename,
                                          const Function* func,
                                          const std::string& container,
                                          std::vector<SymbolInfo>& symbols) {
    if (!func) return;
    
    auto* proto = func->getProto();
    if (!proto) return;
    
    SymbolInfo info;
    info.name = proto->getName();
    info.containerName = container;
    info.kind = SymbolInfo::Kind::Function;
    info.isPublic = true;
    
    // Build function signature
    std::string sig = proto->getName() + "(";
    const auto& params = proto->getParams();
    for (size_t i = 0; i < params.size(); ++i) {
        if (i > 0) sig += ", ";
        sig += params[i].name + ": ";
        if (params[i].type) {
            sig += params[i].type->toString();
        }
    }
    sig += ")";
    if (proto->getReturnType()) {
        sig += " -> " + proto->getReturnType()->toString();
    }
    info.type = sig;
    
    // Use actual location from AST node
    info.location = SourceLocation(filename, proto->getLine(), proto->getColumn(), proto->getName().length());
    symbols.push_back(info);
    
    // Index parameters as symbols
    for (const auto& param : params) {
        SymbolInfo paramInfo;
        paramInfo.name = param.name;
        paramInfo.containerName = proto->getName();
        paramInfo.kind = SymbolInfo::Kind::Parameter;
        if (param.type) {
            paramInfo.type = param.type->toString();
        }
        // Parameters use same line as function, approximate column
        paramInfo.location = SourceLocation(filename, proto->getLine(), proto->getColumn(), param.name.length());
        symbols.push_back(paramInfo);
    }
}

void LanguageCore::collectClassSymbols(const std::string& filename,
                                       const ClassDecl* cls,
                                       std::vector<SymbolInfo>& symbols) {
    if (!cls) return;
    
    SymbolInfo info;
    info.name = cls->getName();
    info.kind = SymbolInfo::Kind::Class;
    info.location = SourceLocation(filename, cls->getLine(), cls->getColumn(), cls->getName().length());
    symbols.push_back(info);
    
    // Index fields
    for (const auto& field : cls->getFields()) {
        SymbolInfo fieldInfo;
        fieldInfo.name = field.name;
        fieldInfo.containerName = cls->getName();
        fieldInfo.kind = SymbolInfo::Kind::Field;
        fieldInfo.isPublic = field.isPublic;
        if (field.type) {
            fieldInfo.type = field.type->toString();
        }
        // Fields use same line as class (approximate), could be improved with field-level location tracking
        fieldInfo.location = SourceLocation(filename, cls->getLine(), cls->getColumn(), field.name.length());
        symbols.push_back(fieldInfo);
    }
    
    // Index methods
    for (const auto& method : cls->getMethods()) {
        SymbolInfo methodInfo;
        methodInfo.name = method.name;
        methodInfo.containerName = cls->getName();
        methodInfo.kind = SymbolInfo::Kind::Method;
        methodInfo.isPublic = method.isPublic;
        methodInfo.isStatic = method.isStatic;
        
        // Build method signature
        std::string sig = method.name + "(";
        for (size_t i = 0; i < method.params.size(); ++i) {
            if (i > 0) sig += ", ";
            sig += method.params[i].name + ": ";
            if (method.params[i].type) {
                sig += method.params[i].type->toString();
            }
        }
        sig += ")";
        if (method.returnType) {
            sig += " -> " + method.returnType->toString();
        }
        methodInfo.type = sig;
        
        // Methods use same line as class (approximate), could be improved with method-level location tracking
        methodInfo.location = SourceLocation(filename, cls->getLine(), cls->getColumn(), method.name.length());
        symbols.push_back(methodInfo);
    }
}

std::vector<CompletionItem> LanguageCore::getKeywordCompletions() const {
    std::vector<CompletionItem> items;
    
    std::vector<std::string> keywords = {
        "fn", "return", "let", "var", "if", "else", "while", "for", "loop",
        "break", "continue", "class", "object", "this", "pub", "priv",
        "static", "constructor", "import", "package", "match", "in",
        "int", "double", "bool", "string", "void", "true", "false", "null"
    };
    
    for (const auto& kw : keywords) {
        CompletionItem item;
        item.label = kw;
        item.kind = CompletionItem::Kind::Keyword;
        item.insertText = kw;
        items.push_back(item);
    }
    
    return items;
}

std::vector<CompletionItem> LanguageCore::getContextCompletions(
    const std::string& filename, size_t line, size_t column) const {
    
    std::vector<CompletionItem> items;
    
    auto it = files_.find(filename);
    if (it == files_.end()) {
        return items;
    }
    
    const auto& data = it->second;
    
    // Add all visible symbols
    for (const auto& symbol : data.symbols) {
        CompletionItem item;
        item.label = symbol.name;
        item.detail = symbol.type;
        
        switch (symbol.kind) {
            case SymbolInfo::Kind::Function:
                item.kind = CompletionItem::Kind::Function;
                break;
            case SymbolInfo::Kind::Variable:
                item.kind = CompletionItem::Kind::Variable;
                break;
            case SymbolInfo::Kind::Class:
                item.kind = CompletionItem::Kind::Class;
                break;
            case SymbolInfo::Kind::Method:
                item.kind = CompletionItem::Kind::Method;
                break;
            case SymbolInfo::Kind::Field:
                item.kind = CompletionItem::Kind::Field;
                break;
            default:
                item.kind = CompletionItem::Kind::Variable;
        }
        
        item.insertText = symbol.name;
        items.push_back(item);
    }
    
    return items;
}

} // namespace aurora

