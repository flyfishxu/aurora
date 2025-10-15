#include "aurora/AST.h"
#include "aurora/Lexer.h"
#include "aurora/Parser.h"
#include "aurora/CodeGen.h"
#include "aurora/Logger.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <set>
#include <algorithm>

namespace aurora {

// Keep track of already loaded modules to prevent circular imports
static std::set<std::string> loadedModules;

// Global registry for package source directories
static std::vector<std::string> packageSearchPaths = {".", "src", "stdlib/aurora"};

std::string PackageDecl::toPath() const {
    // Convert package name to file system path
    // com.example.myapp -> com/example/myapp
    std::string path = packageName;
    std::replace(path.begin(), path.end(), '.', '/');
    return path;
}

bool ImportDecl::load(const std::string& currentFile, const std::string& currentPackage) {
    auto& logger = Logger::instance();
    
    // Check if module is already loaded
    if (loadedModules.count(modulePath) > 0) {
        logger.debug("Module already loaded: " + modulePath, "Modules");
        return true;
    }
    
    // Resolve module path (.aur file)
    std::string filePath;
    bool isPackageImport = false;
    
    // Determine if this is a package-style import (contains dots but no slashes)
    if (modulePath.find('.') != std::string::npos && 
        modulePath.find('/') == std::string::npos && 
        modulePath.find('\\') == std::string::npos) {
        // Package-style import: com.example.MyClass
        isPackageImport = true;
        
        // Convert package path to file path
        std::string packagePath = modulePath;
        std::replace(packagePath.begin(), packagePath.end(), '.', '/');
        filePath = packagePath + ".aur";
        
        logger.debug("Package import detected: " + modulePath + " -> " + filePath, "Modules");
    } 
    // If path contains '/', treat as relative path
    else if (modulePath.find('/') != std::string::npos || modulePath.find('\\') != std::string::npos) {
        filePath = modulePath;
        // Check if path ends with .aur
        if (filePath.length() < 4 || filePath.substr(filePath.length() - 4) != ".aur") {
            filePath += ".aur";
        }
    } else {
        // Simple name - look in current directory or package-relative
        filePath = modulePath + ".aur";
    }
    
    // Try to resolve the file path
    std::string resolvedPath;
    
    // Try different resolution strategies
    if (isPackageImport) {
        // For package imports, search in package search paths
        bool found = false;
        
        // First try relative to current file (for test organization)
        if (!currentFile.empty()) {
            std::filesystem::path currentPath(currentFile);
            std::filesystem::path candidate = currentPath.parent_path() / filePath;
            if (std::filesystem::exists(candidate)) {
                resolvedPath = candidate.string();
                found = true;
                logger.debug("Found package file relative to current: " + resolvedPath, "Modules");
            }
        }
        
        // Then try package search paths
        if (!found) {
            for (const auto& searchPath : packageSearchPaths) {
                std::string candidate = searchPath + "/" + filePath;
                if (std::filesystem::exists(candidate)) {
                    resolvedPath = candidate;
                    found = true;
                    logger.debug("Found package file in search path: " + resolvedPath, "Modules");
                    break;
                }
            }
        }
        
        // Finally try current directory
        if (!found && std::filesystem::exists(filePath)) {
            resolvedPath = filePath;
            found = true;
            logger.debug("Found package file in current dir: " + resolvedPath, "Modules");
        }
        
        if (!found) {
            logger.error("Package file not found: " + modulePath + " (searched in package paths and relative to " + currentFile + ")");
            return false;
        }
    } else {
        // For regular imports, try relative to current file first
        resolvedPath = filePath;
        if (!currentFile.empty() && !std::filesystem::exists(filePath)) {
            std::filesystem::path currentPath(currentFile);
            std::filesystem::path moduleFilePath(filePath);
            resolvedPath = (currentPath.parent_path() / moduleFilePath).string();
        }
        
        // Check if file exists
        if (!std::filesystem::exists(resolvedPath)) {
            logger.error("Module file not found: " + resolvedPath + " (imported from: " + currentFile + ")");
            return false;
        }
    }
    
    filePath = resolvedPath;
    
    // Load file contents
    std::ifstream file(filePath);
    if (!file) {
        logger.error("Cannot open module file: " + filePath);
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    
    logger.debug("Loaded module source: " + filePath + " (" + std::to_string(source.length()) + " bytes)", "Modules");
    
    try {
        // Parse the module
        Lexer lexer(source);
        Parser parser(lexer);
        auto functions = parser.parseProgram();
        auto& classes = parser.getClasses();
        auto& subImports = parser.getImports();
        
        logger.debug("Module contains " + std::to_string(functions.size()) + 
                    " function(s) and " + std::to_string(classes.size()) + " class(es)", "Modules");
        
        // Get package name from parsed module
        std::string modulePackage;
        if (parser.getPackage()) {
            modulePackage = parser.getPackage()->getPackageName();
            logger.debug("Module package: " + modulePackage, "Modules");
        }
        
        // Process sub-imports recursively
        for (auto& subImport : subImports) {
            if (!subImport->load(filePath, modulePackage)) {
                logger.error("Failed to load sub-import: " + subImport->getModulePath());
                return false;
            }
        }
        
        // Generate code for classes
        for (auto& classDecl : classes) {
            llvm::Type* classType = classDecl->codegen();
            if (!classType) {
                logger.error("Failed to generate struct type for class: " + classDecl->getName());
                return false;
            }
        }
        
        for (auto& classDecl : classes) {
            classDecl->codegenMethods();
        }
        
        // Generate code for functions
        for (auto& func : functions) {
            llvm::Function* llvm_func = func->codegen();
            if (!llvm_func) {
                logger.error("Failed to generate code for function: " + func->getProto()->getName());
                return false;
            }
        }
        
        // Mark module as loaded
        loadedModules.insert(modulePath);
        
        logger.debug("Successfully loaded module: " + modulePath, "Modules");
        return true;
        
    } catch (const std::exception& e) {
        logger.error("Error loading module " + modulePath + ": " + std::string(e.what()));
        return false;
    }
}

} // namespace aurora

