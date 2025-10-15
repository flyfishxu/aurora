#include "aurora/Type.h"
#include "aurora/Lexer.h"
#include "aurora/Parser.h"
#include "aurora/CodeGen.h"
#include "aurora/AST.h"
#include "aurora/Diagnostic.h"
#include "aurora/Logger.h"
#include "aurora/CrashHandler.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <llvm/Support/raw_ostream.h>

using namespace aurora;

// Version information
#define AURORA_VERSION_MAJOR 0
#define AURORA_VERSION_MINOR 6
#define AURORA_VERSION_PATCH 2
#define AURORA_VERSION "0.6.2"

std::string loadFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        auto& diag = getDiagnosticEngine();
        SourceLocation loc(path, 0, 0, 0);
        diag.reportError("E0001", "Cannot open file: " + path, loc);
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void demonstrateTypeSystem() {
    std::cout << "=== AuroraLang Type System Demo ===" << std::endl;
    
    auto& registry = TypeRegistry::instance();
    
    // Basic types
    auto intType = registry.getIntType();
    auto doubleType = registry.getDoubleType();
    auto boolType = registry.getBoolType();
    auto stringType = registry.getStringType();
    
    std::cout << "\nBasic Types:" << std::endl;
    std::cout << "  - " << intType->toString() << std::endl;
    std::cout << "  - " << doubleType->toString() << std::endl;
    std::cout << "  - " << boolType->toString() << std::endl;
    std::cout << "  - " << stringType->toString() << std::endl;
    
    // Optional types (for null safety)
    auto optionalInt = registry.getOptionalType(intType);
    auto optionalString = registry.getOptionalType(stringType);
    
    std::cout << "\nOptional Types (Null-Safe):" << std::endl;
    std::cout << "  - " << optionalInt->toString() << std::endl;
    std::cout << "  - " << optionalString->toString() << std::endl;
    
    // Function types
    std::vector<std::shared_ptr<Type>> params = {intType, intType};
    auto funcType = registry.getFunctionType(intType, params);
    
    std::cout << "\nFunction Type:" << std::endl;
    std::cout << "  - " << funcType->toString() << std::endl;
    
    std::cout << "\nType Properties:" << std::endl;
    std::cout << "  - int is nullable: " << (intType->isNullable() ? "yes" : "no") << std::endl;
    std::cout << "  - int? is nullable: " << (optionalInt->isNullable() ? "yes" : "no") << std::endl;
}

void demonstrateLexer(const std::string& source) {
    std::cout << "\n=== Lexer Demo ===" << std::endl;
    std::cout << "Source code:\n" << source << std::endl;
    std::cout << "\nTokens:" << std::endl;
    
    Lexer lexer(source);
    Token token = lexer.nextToken();
    
    while (token.type != TokenType::Eof) {
        std::cout << "  " << token.toString() << std::endl;
        token = lexer.nextToken();
    }
}

// External function that can be called from Aurora code
extern "C" double printd(double x) {
    std::cout << x << std::endl;
    return 0;
}

int compileAndRun(const std::string& source, const std::string& filename, bool emit_llvm = false, const std::string& output_file = "") {
    auto& diag = getDiagnosticEngine();
    auto& logger = Logger::instance();
    
    diag.setSourceCode(source);
    diag.setFilename(filename);
    
    try {
        logger.info("Starting compilation...");
        logger.debug("Source file: " + filename, "Compiler");
        logger.debug("Source length: " + std::to_string(source.length()) + " bytes", "Compiler");
        
        // Lexing & Parsing
        logger.phaseStart("Lexical analysis");
        SCOPED_TIMER("Lexical & Parsing");
        Lexer lexer(source);
        logger.phaseEnd("Lexical analysis");
        
        logger.phaseStart("Parsing");
        Parser parser(lexer);
        auto functions = parser.parseProgram();
        auto& classes = parser.getClasses();
        auto& imports = parser.getImports();
        logger.phaseEnd("Parsing");
        
        logger.info("Parsed " + std::to_string(functions.size()) + " function(s), " + 
                   std::to_string(classes.size()) + " class(es), and " +
                   std::to_string(imports.size()) + " import(s)");
        logger.debug("Functions: " + std::to_string(functions.size()), "Parser");
        logger.debug("Classes: " + std::to_string(classes.size()), "Parser");
        logger.debug("Imports: " + std::to_string(imports.size()), "Parser");
        
        // Get current package (if any)
        std::string currentPackage;
        if (parser.getPackage()) {
            currentPackage = parser.getPackage()->getPackageName();
            logger.info("Package: " + currentPackage);
        }
        
        // Auto-import prelude (like Kotlin's stdlib)
        logger.phaseStart("Prelude loading");
        auto preludeImport = std::make_unique<ImportDecl>("stdlib/aurora/core/prelude");
        logger.debug("Auto-loading prelude...", "Modules");
        if (!preludeImport->load(filename, currentPackage)) {
            logger.warning("Failed to auto-load prelude - stdlib functions may not be available");
        }
        logger.phaseEnd("Prelude loading");
        
        // Process imports
        if (!imports.empty()) {
            logger.phaseStart("Module loading");
            for (auto& import : imports) {
                logger.debug("Loading module: " + import->getModulePath(), "Modules");
                if (!import->load(filename, currentPackage)) {
                    logger.error("Failed to load module: " + import->getModulePath());
                    return 1;
                }
            }
            logger.phaseEnd("Module loading");
        }
        
        // Code generation
        logger.phaseStart("Code generation");
        auto& ctx = getGlobalContext();
        
        // Register built-in functions automatically (no need for extern declarations)
        logger.debug("Registering built-in functions...", "Codegen");
        
        // printd(x: double) -> double - Print a double value
        {
            std::vector<llvm::Type*> printd_args = {llvm::Type::getDoubleTy(ctx.getContext())};
            llvm::FunctionType* printd_type = llvm::FunctionType::get(
                llvm::Type::getDoubleTy(ctx.getContext()),
                printd_args,
                false);
            llvm::Function* printd_func = llvm::Function::Create(
                printd_type,
                llvm::Function::ExternalLinkage,
                "printd",
                ctx.getModule());
            ctx.setFunction("printd", printd_func);
            logger.debug("Registered: printd(double) -> double", "Codegen");
        }
        
        // Generate code for all classes first (types and structures)
        if (!classes.empty()) {
            logger.debug("Generating class structures...", "Codegen");
        }
        for (auto& classDecl : classes) {
            logger.debug("Class: " + classDecl->getName() + 
                        " (" + std::to_string(classDecl->getFields().size()) + " fields, " +
                        std::to_string(classDecl->getMethods().size()) + " methods)", "Codegen");
            llvm::Type* classType = classDecl->codegen();
            if (!classType) {
                logger.error("Failed to generate struct type for class: " + classDecl->getName());
                return false;
            }
        }
        
        // Then generate code for all methods
        if (!classes.empty()) {
            logger.debug("Generating class methods...", "Codegen");
        }
        for (auto& classDecl : classes) {
            try {
                classDecl->codegenMethods();
            } catch (const std::exception& e) {
                logger.error("Failed to generate methods for class " + classDecl->getName() + 
                           ": " + std::string(e.what()));
                return false;
            }
        }
        
        // Generate code for all functions
        logger.debug("Generating " + std::to_string(functions.size()) + " function(s)...", "Codegen");
        bool codegen_success = true;
        for (auto& func : functions) {
            logger.debug("Function: " + func->getProto()->getName(), "Codegen");
            llvm::Function* llvm_func = func->codegen();
            if (!llvm_func) {
                logger.error("Failed to generate code for function: " + func->getProto()->getName());
                codegen_success = false;
                break;
            }
        }
        logger.phaseEnd("Code generation");
        
        // Check if codegen succeeded
        if (!codegen_success || diag.hasErrors()) {
            logger.error("Code generation failed");
            return 1;
        }
        
        // Verify the generated module
        logger.debug("Verifying LLVM module...", "Codegen");
        if (!verifyModule(&ctx.getModule(), false)) {
            logger.error("Module verification failed - there are errors in generated code");
            return 1;
        }
        logger.debug("Module verification passed", "Codegen");
        
        // Emit LLVM IR if requested
        if (emit_llvm) {
            std::string ll_file = output_file.empty() ? "output.ll" : output_file;
            std::error_code ec;
            llvm::raw_fd_ostream out(ll_file, ec);
            if (ec) {
                logger.error("Could not open file " + ll_file + ": " + ec.message());
                return false;
            }
            ctx.getModule().print(out, nullptr);
            logger.info("Generated LLVM IR: " + ll_file);
            return 0;
        }
        
        // Initialize JIT and run
        logger.phaseStart("JIT compilation and execution");
        try {
            ctx.initializeJIT();
        } catch (const std::exception& e) {
            logger.error("JIT initialization failed: " + std::string(e.what()));
            return 1;
        }
        
        try {
            int result = ctx.runMain();
            logger.phaseEnd("JIT compilation and execution");
            logger.info("Program completed with exit code: " + std::to_string(result));
            std::cout.flush();
            std::exit(result);  // Exit immediately to avoid any issues
        } catch (const std::exception& e) {
            logger.phaseEnd("JIT compilation and execution", false);
            logger.error("Runtime error: " + std::string(e.what()));
            return 1;
        }
        
    } catch (const std::runtime_error& e) {
        logger.error("Compilation error: " + std::string(e.what()));
        return 1;
    } catch (const std::exception& e) {
        logger.error("Unexpected error: " + std::string(e.what()));
        logger.error("This might be a compiler bug. Please report it.");
        return 1;
    } catch (...) {
        logger.error("Unknown error occurred during compilation");
        logger.error("This is likely a compiler bug. Please report it.");
        return 1;
    }
}

void printVersion() {
    std::cout << "AuroraLang version " << AURORA_VERSION << "\n";
    std::cout << "Built with LLVM " << LLVM_VERSION_MAJOR << "." << LLVM_VERSION_MINOR << "\n";
    std::cout << "Copyright (c) 2025 AuroraLang Project\n";
}

void printUsage(const char* prog) {
    std::cerr << "AuroraLang - A Modern LLVM-Powered Language\n\n";
    std::cerr << "Usage: " << prog << " [options] <file.aur>\n\n";
    std::cerr << "Options:\n";
    std::cerr << "  -h, --help              Show this help message\n";
    std::cerr << "  -v, --version           Show version information\n";
    std::cerr << "  --debug                 Enable debug mode (same as --log-level debug)\n";
    std::cerr << "  --trace                 Enable trace mode (most verbose)\n";
    std::cerr << "  --log-level <level>     Set log level: trace|debug|info|warn|error|off\n";
    std::cerr << "  --lex                   Show lexer tokens only\n";
    std::cerr << "  --emit-llvm             Emit LLVM IR to file (output.ll)\n";
    std::cerr << "  -o <file>               Specify output file for --emit-llvm\n";
    std::cerr << "  --type-demo             Show type system demo\n";
    std::cerr << "\nLog Levels:\n";
    std::cerr << "  trace  - Show all debug information including AST and IR\n";
    std::cerr << "  debug  - Show detailed compilation steps and timing\n";
    std::cerr << "  info   - Show compilation phases\n";
    std::cerr << "  warn   - Show only warnings and errors\n";
    std::cerr << "  error  - Show only errors\n";
    std::cerr << "  off    - Suppress all log messages (default)\n";
    std::cerr << "\nExamples:\n";
    std::cerr << "  " << prog << " program.aur                     # Compile and run\n";
    std::cerr << "  " << prog << " --debug program.aur             # Compile with debug info\n";
    std::cerr << "  " << prog << " --trace program.aur             # Most verbose output\n";
    std::cerr << "  " << prog << " --log-level warn program.aur    # Only show warnings/errors\n";
    std::cerr << "  " << prog << " --emit-llvm program.aur         # Generate LLVM IR\n";
    std::cerr << "  " << prog << " --emit-llvm -o out.ll program.aur\n";
}

int main(int argc, char** argv) {
    // Setup crash handler for better error reporting
    setupCrashHandler();
    
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    // Parse command line arguments
    std::string filename;
    bool lex_only = false;
    bool emit_llvm = false;
    bool type_demo = false;
    bool debug_mode = false;
    std::string output_file;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            printVersion();
            return 0;
        } else if (arg == "--debug") {
            debug_mode = true;
            Logger::instance().setLevel(LogLevel::Debug);
            getDiagnosticEngine().setDebugMode(true);
        } else if (arg == "--trace") {
            Logger::instance().setLevel(LogLevel::Trace);
            getDiagnosticEngine().setDebugMode(true);
        } else if (arg == "--log-level") {
            if (i + 1 < argc) {
                std::string level = argv[++i];
                if (level == "trace") Logger::instance().setLevel(LogLevel::Trace);
                else if (level == "debug") Logger::instance().setLevel(LogLevel::Debug);
                else if (level == "info") Logger::instance().setLevel(LogLevel::Info);
                else if (level == "warn" || level == "warning") Logger::instance().setLevel(LogLevel::Warning);
                else if (level == "error") Logger::instance().setLevel(LogLevel::Error);
                else if (level == "off") Logger::instance().setLevel(LogLevel::Off);
                else {
                    std::cerr << "Error: Invalid log level: " << level << "\n";
                    return 1;
                }
            } else {
                std::cerr << "Error: --log-level requires an argument\n";
                return 1;
            }
        } else if (arg == "--lex") {
            lex_only = true;
        } else if (arg == "--emit-llvm") {
            emit_llvm = true;
        } else if (arg == "-o") {
            if (i + 1 < argc) {
                output_file = argv[++i];
            } else {
                std::cerr << "Error: -o requires an argument\n";
                return 1;
            }
        } else if (arg == "--type-demo") {
            type_demo = true;
        } else if (arg[0] != '-') {
            filename = arg;
        } else {
            std::cerr << "Error: Unknown option: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }
    
    if (type_demo) {
        std::cout << "=== AuroraLang Type System Demo ===\n";
        demonstrateTypeSystem();
        return 0;
    }
    
    if (filename.empty()) {
        std::cerr << "Error: No input file specified\n";
        printUsage(argv[0]);
        return 1;
    }
    
    std::string source = loadFile(filename);
    if (source.empty()) {
        return 1;
    }
    
    if (lex_only) {
        demonstrateLexer(source);
        return 0;
    }
    
    // Compile and run (or emit IR)
    int exit_code = compileAndRun(source, filename, emit_llvm, output_file);
    std::cout.flush();
    std::cerr.flush();
    
    // Print diagnostic summary
    if (debug_mode) {
        getDiagnosticEngine().printSummary();
    }
    
    std::exit(exit_code);  // Use exit() to bypass destructors
}
