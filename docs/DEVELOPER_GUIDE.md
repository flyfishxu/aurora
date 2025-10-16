# AuroraLang Developer Guide

This guide is for developers who want to contribute to AuroraLang or understand its internals.

---

## Table of Contents

1. [Development Setup](#development-setup)
2. [Project Architecture](#project-architecture)
3. [LSP Architecture](#lsp-architecture)
4. [Debugging Tools](#debugging-tools)
5. [Contributing Guidelines](#contributing-guidelines)
6. [Testing](#testing)
7. [Documentation](#documentation)

---

## Development Setup

### Prerequisites

- **LLVM 14+** (tested with LLVM 21)
- **CMake 3.20+**
- **C++17 compiler** (GCC 9+, Clang 10+, MSVC 2019+)
- **Git**

### Building from Source

```bash
# Clone repository
git clone https://github.com/flyfishxu/AuroraLang.git
cd AuroraLang

# Create build directory
mkdir build && cd build

# Configure (macOS)
cmake .. -DCMAKE_PREFIX_PATH="$(brew --prefix llvm)"

# Or configure (Linux)
cmake ..

# Build with all cores
make -j$(nproc)

# Run tests
cd ..
./run_tests.sh
```

### IDE Setup

#### VSCode

Recommended extensions:
- C/C++ (Microsoft)
- CMake Tools
- LLVM (syntax highlighting for .ll files)

Generate `compile_commands.json` for IntelliSense:
```bash
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
```

#### CLion

CLion natively supports CMake projects. Just open the project root.

---

## Project Architecture

### Four-Layer Architecture

AuroraLang follows strict layer separation:

```
┌─────────────────────────────────────────────────┐
│            Standard Library (Std)               │  AuroraLang
│     (Collections, Algorithms, I/O, etc.)        │  
├─────────────────────────────────────────────────┤
│             Runtime Layer                       │  C
│  (Memory Management, Arrays, Objects, etc.)     │  
├─────────────────────────────────────────────────┤
│           Compiler Layer (Frontend)             │  C++
│    (Lexer, Parser, Type System, CodeGen)        │  
├─────────────────────────────────────────────────┤
│             LLVM Layer (Backend)                │  LLVM
│        (IR, Optimization, Code Generation)      │  
└─────────────────────────────────────────────────┘
```

**Key Principle**: Each layer only depends on layers below it. No upward dependencies.

See [ARCHITECTURE.md](ARCHITECTURE.md) for complete details.

### Modular Code Generation

The compiler's code generation is split into specialized modules:

- **CodeGen.cpp** (150 lines) - Context management
- **ExprCodeGen.cpp** (500 lines) - Expression generation
- **StmtCodeGen.cpp** (450 lines) - Statement generation
- **ClassCodeGen.cpp** (420 lines) - OOP generation
- **ArrayCodeGen.cpp** (200 lines) - Array generation

This modular design makes the codebase easier to navigate and maintain.

---

## LSP Architecture

AuroraLang implements Language Server Protocol (LSP) support using a three-layer architecture to ensure modularity, reusability, and maintainability.

### Three-Layer Design

```
┌─────────────────────────────────────────────────┐
│            IDE Client / Extension               │  TypeScript
│       (VSCode, thin client layer)               │  
├─────────────────────────────────────────────────┤
│             LSP Server (Protocol)               │  C++
│    (JSON-RPC, Protocol Adaptation Layer)        │  
├─────────────────────────────────────────────────┤
│          Language Core (Reusable)               │  C++
│  (Lexer, Parser, Semantic Analysis, Index)      │  
└─────────────────────────────────────────────────┘
```

### Layer 1: Language Core (Reusable Library)

**Location**: `compiler/src/core/LanguageCore.cpp`

**Responsibilities**:
- Lexical and syntax analysis
- Semantic analysis and type checking
- Symbol indexing
- Error diagnostics
- Code formatting (future)
- Linting (future)

**Key Features**:
- **No I/O**: All file operations abstracted
- **No Protocol**: Pure language semantics
- **Reusable**: Used by both CLI compiler and LSP server
- **Thread-safe**: Immutable data structures

**Example**:
```cpp
LanguageCore core;
core.setSource("file.aur", sourceCode);
core.analyze("file.aur");
auto diagnostics = core.getDiagnostics("file.aur");
auto symbols = core.getSymbols("file.aur");
```

### Layer 2: LSP Server (Protocol Adapter)

**Location**: `tools/aurora-lsp/src/LSPServer.cpp`

**Responsibilities**:
- JSON-RPC message handling
- LSP protocol compliance
- Document synchronization
- Capability negotiation
- Protocol-level caching

**Key Features**:
- **Standalone process**: Runs as separate process with `--stdio`
- **Incremental updates**: Efficient document synchronization
- **Cancellation**: Supports request cancellation
- **No semantics**: All language logic delegated to Core

**Supported LSP Features**:
- `textDocument/hover` - Type information
- `textDocument/definition` - Go to definition
- `textDocument/references` - Find references
- `textDocument/completion` - Auto-completion
- `textDocument/documentSymbol` - Document outline
- `workspace/symbol` - Workspace symbols
- `textDocument/publishDiagnostics` - Error reporting
- `textDocument/formatting` - Code formatting

**Example**:
```bash
# Start LSP server
aurora-lsp --stdio
```

### Layer 3: IDE Client (Thin Client)

**Location**: `vscode-extension/src/lspClient.ts`

**Responsibilities**:
- Start/stop LSP server
- Forward LSP requests
- Render results in UI
- Configuration management

**Key Features**:
- **Minimal logic**: No language semantics
- **Auto-discovery**: Finds LSP server automatically
- **Fallback mode**: Uses built-in providers if LSP unavailable
- **Configuration**: User-configurable server path

**Configuration**:
```json
{
  "auroralang.enableLSP": true,
  "auroralang.lspServerPath": "/path/to/aurora-lsp"
}
```

### Development Principles

#### 1. Strict Layer Separation

**Core Layer Rules**:
- ❌ NO file I/O (use callbacks/VFS)
- ❌ NO protocol knowledge (JSON-RPC, LSP)
- ❌ NO global state / singletons (except logger)
- ✅ Pure language semantics
- ✅ Reusable by CLI and LSP
- ✅ Thread-safe operations

**LSP Server Rules**:
- ❌ NO language semantics (delegate to Core)
- ❌ NO business logic duplication
- ✅ Protocol handling only
- ✅ Efficient caching
- ✅ Proper error handling

**Client Rules**:
- ❌ NO language analysis
- ❌ NO complex logic
- ✅ UI rendering only
- ✅ Configuration management
- ✅ Server lifecycle management

#### 2. Common Pitfalls to Avoid

**❌ Bad: Mixing I/O in Core**
```cpp
// DON'T DO THIS
class LanguageCore {
    void analyze(const std::string& filename) {
        std::ifstream file(filename);  // ❌ Direct I/O
        // ...
    }
};
```

**✅ Good: Abstract I/O**
```cpp
// DO THIS
class LanguageCore {
    void setSource(const std::string& filename, 
                   const std::string& source) {
        // ✅ Source provided by caller
    }
};
```

**❌ Bad: Duplicating Semantics in LSP**
```cpp
// DON'T DO THIS
json LSPServer::handleHover(...) {
    // ❌ Implementing type checking here
    if (isFunction()) { ... }
}
```

**✅ Good: Delegate to Core**
```cpp
// DO THIS
json LSPServer::handleHover(...) {
    auto hover = core_.getHover(...);  // ✅ Delegate
    return hoverToLSP(hover);
}
```

#### 3. Error Handling

All errors use the unified `Diagnostic` system:

```cpp
struct Diagnostic {
    DiagnosticLevel level;  // Error, Warning, Note
    std::string code;       // Error code (e.g., "E001")
    std::string message;
    SourceLocation location;
    std::vector<std::string> suggestions;
};
```

**Benefits**:
- Consistent error format
- Easy LSP conversion
- Actionable suggestions
- Structured diagnostics

#### 4. Testing Strategy

**Unit Tests** (Future):
- Test Core independently
- Mock I/O dependencies
- Fast, isolated tests

**Integration Tests**:
- Test LSP server with real messages
- Verify protocol compliance
- End-to-end validation

**IDE Tests**:
- Extension activation
- Command execution
- UI rendering

### Building and Running LSP

**Build**:
```bash
cd build
cmake ..
make -j8
# LSP server at: build/tools/aurora-lsp/aurora-lsp
```

**Run Standalone**:
```bash
./build/tools/aurora-lsp/aurora-lsp --stdio
```

**Use in VSCode**:
1. Build LSP server
2. Open workspace in VSCode
3. Extension auto-discovers server
4. LSP features activate automatically

**Verify LSP is Active**:
Check VSCode Output panel → "AuroraLang LSP"

### Future Enhancements

- [ ] Signature help completion
- [ ] Rename refactoring
- [ ] Code actions (quick fixes)
- [ ] Semantic highlighting
- [ ] Call hierarchy
- [ ] Type hierarchy
- [ ] Inlay hints
- [ ] Code lens

### References

- [LSP Specification](https://microsoft.github.io/language-server-protocol/)
- [VSCode LSP Guide](https://code.visualstudio.com/api/language-extensions/language-server-extension-guide)
- [Language Core API](../compiler/include/aurora/LanguageCore.h)

---

## Debugging Tools

### Log Levels

AuroraLang has a comprehensive logging system:

```bash
# No output (default)
aurora program.aur

# Only errors
aurora --log-level error program.aur

# Warnings and errors
aurora --log-level warn program.aur

# Compilation phases (recommended for development)
aurora --log-level info program.aur

# Detailed information + timing
aurora --debug program.aur

# Maximum verbosity (AST, IR, tokens, everything)
aurora --trace program.aur
```

### Example Debug Output

#### Info Level
```
[INFO ] Starting compilation...
▶ [Aurora] Phase: Lexical analysis
▶ [Aurora] Phase: Parsing
[INFO ] Parsed 2 function(s) and 0 class(es)
```

#### Debug Level
```
[INFO ] Starting compilation...
[DEBUG] [Compiler] Source file: test.aur
[DEBUG] [Compiler] Source length: 215 bytes
▶ [Aurora] Phase: Lexical analysis
[DEBUG] [Timer] Starting: Lexical & Parsing
✓ Phase Lexical analysis completed
[DEBUG] [Timer] Completed: Lexical & Parsing in 1.23 ms
```

#### Trace Level
```
[TRACE] [Lexer] Token: fn (line: 1, col: 1)
[TRACE] [Lexer] Token: main (line: 1, col: 4)
=== AST Dump ===
FunctionDecl: main
  ReturnType: int
  Body:
    CallExpr: printd
================
```

### Using the Logger in Code

```cpp
#include "aurora/Logger.h"

// Simple logging
Logger::instance().info("Compilation started");
Logger::instance().debug("Processing token", "Lexer");

// Convenient macros
LOG_INFO("Compilation started");
LOG_DEBUG("Processing token");
LOG_LEXER_DEBUG("Found identifier: " + name);

// Scoped timing
{
    SCOPED_TIMER("Parse Expression");
    // ... parsing code ...
} // Timer reports elapsed time automatically

// Structured logging
logger.logAST(ast.dump());
logger.logLLVMIR(module.print());

// Phase tracking
logger.phaseStart("Code Generation");
// ... work ...
logger.phaseEnd("Code Generation", success);
```

### Crash Handler

AuroraLang automatically catches crashes and provides stack traces:

```
[FATAL] ===============================================
[FATAL] FATAL: Caught signal SIGSEGV (Segmentation Fault)
[FATAL] ===============================================
[FATAL] Stack trace:
[FATAL]   0   aurora    0x00000001044042f8 _ZN6auroraL13signalHandlerEi + 524
[FATAL]   1   libsystem_platform.dylib  0x000000019cf6f744 _sigtramp + 56
[FATAL]   2   aurora    0x000000010614fcc4 _ZN4llvm21SymbolTableListTraitsINS_10BasicBlockEJEE18removeNodeFromListEPS1_ + 32
[FATAL] ===============================================
[FATAL] This is likely a bug in the Aurora compiler.
[FATAL] Please report this with the code that caused it.
[FATAL] ===============================================
```

### LLVM IR Inspection

Generate LLVM IR to inspect generated code:

```bash
# Generate IR
aurora --emit-llvm program.aur

# View IR
cat output.ll

# With debug output
aurora --debug --emit-llvm program.aur
```

### Verify LLVM Module

The compiler automatically verifies generated LLVM IR:

```cpp
// In main.cpp
verifyModule(&ctx.getModule(), false);
```

If verification fails:
```
[ERROR] LLVM Module Verification Failed!
[ERROR] ===============================================
[ERROR] Function 'Test_isPositive' has invalid return type
[ERROR]   Expected: void
[ERROR]   Got: i32
[ERROR] ===============================================
```

---

## Standard Library Architecture

### Design Philosophy

AuroraLang's standard library follows the model of mature compiled languages (C, C++, Rust, Swift, Go):

1. **Dual Library Format**: Both static (`.a`/`.lib`) and shared (`.so`/.dylib`/`.dll`) libraries
2. **Sysroot-Based Resolution**: No hardcoded paths, flexible deployment
3. **Separation of Concerns**:
   - **Runtime** (`libaurora_runtime`): Low-level operations (arrays, memory)
   - **Stdlib** (`libaurora_stdlib`): Native extensions
   - **Aurora Source** (`.aur` files): High-level standard library

### Library Build System

**CMake Configuration**:
- `BUILD_SHARED_LIBS=ON` (default): Build `.so`/`.dylib`/`.dll`
- `BUILD_STATIC_LIBS=ON` (default): Build `.a`/`.lib`

**Output**:
```
build/runtime/
├── libaurora_runtime.a              # Static library
├── libaurora_runtime.dylib          # Shared library (symlink)
├── libaurora_runtime.0.dylib        # Version symlink
└── libaurora_runtime.0.6.3.dylib    # Actual shared library

build/stdlib/
├── libaurora_stdlib.a
├── libaurora_stdlib.dylib
├── libaurora_stdlib.0.dylib
└── libaurora_stdlib.0.6.3.dylib
```

**Versioning**: Libraries follow semantic versioning (SOVERSION for ABI compatibility)

### Module Search Path Resolution

AuroraLang uses a robust, sysroot-based module resolution system that works from any directory.

#### Sysroot Priority Order

When resolving the sysroot (system root directory), the compiler searches in this order:

1. **`--sysroot` Argument** (highest priority)
   - Command line: `aurora --sysroot /usr/local program.aur`
   - Explicit override for deployment

2. **`AURORA_HOME` Environment Variable**
   - Example: `export AURORA_HOME=/usr/local`
   - Useful for custom installations

3. **Relative to Executable**
   - Searches `bin/../` relative to executable
   - Automatic for installed binaries

4. **Compile-time `AURORA_SYSROOT`**
   - Defined during build via CMake
   - Default: `${CMAKE_SOURCE_DIR}`

5. **Fallback**: Current directory (development only)

#### Module Search Within Sysroot

Once sysroot is determined, modules are located:

1. **Standard Library**: `<sysroot>/stdlib/aurora/`
   - Example: `import core.math` → `<sysroot>/stdlib/aurora/core/math.aur`

2. **Additional Module Paths**: `AURORA_MODULE_PATH`
   - Colon-separated list: `export AURORA_MODULE_PATH=/path1:/path2`
   - For custom user modules

3. **Local Directories**: `.` and `src/`
   - Always included for project-local modules

#### Implementation

**Module System Initialization** (`compiler/src/ast/ModuleLoader.cpp`):

```cpp
void initializeModuleSystem() {
    // Priority 1: Environment variable
    const char* envStdlibPath = std::getenv("AURORA_STDLIB_PATH");
    if (envStdlibPath && std::filesystem::exists(envStdlibPath)) {
        stdlibPath = envStdlibPath;
    }
    
    // Priority 2: Compile-time path
    if (stdlibPath.empty()) {
#ifdef AURORA_STDLIB_PATH
        stdlibPath = AURORA_STDLIB_PATH;
#endif
    }
    
    // Priority 3: Relative to executable
    if (stdlibPath.empty()) {
        std::string exeDir = getExecutableDirectory();
        std::filesystem::path relPath = 
            std::filesystem::path(exeDir) / ".." / "stdlib" / "aurora";
        if (std::filesystem::exists(relPath)) {
            stdlibPath = relPath.string();
        }
    }
    
    // Add all search paths
    addModuleSearchPath(stdlibPath);  // stdlib first
    addModuleSearchPath(".");         // current dir
    addModuleSearchPath("src");       // src dir
}
```

**Executable Path Detection** (`compiler/src/utils/Utils.cpp`):

```cpp
std::string getExecutablePath() {
#ifdef __APPLE__
    char path[1024];
    uint32_t size = sizeof(path);
    _NSGetExecutablePath(path, &size);
    return std::string(path);
#elif defined(_WIN32)
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return std::string(path);
#else  // Linux
    char path[1024];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    path[len] = '\0';
    return std::string(path);
#endif
}
```

### Prelude Auto-Import

The **prelude** (`stdlib/aurora/core/prelude.aur`) is automatically imported in every file:

```cpp
// In main.cpp
auto preludeImport = std::make_unique<ImportDecl>("core.prelude");
if (!preludeImport->load(filename, currentPackage)) {
    logger.warning("Failed to auto-load prelude");
}
```

**Why Package-Style Import?**
- Uses `"core.prelude"` instead of `"stdlib/aurora/core/prelude"`
- Works with module search paths
- Independent of working directory
- Similar to Kotlin's implicit stdlib imports

### Import Resolution Examples

**Package Import** (recommended):
```aurora
import core.math        # → stdlib/aurora/core/math.aur
import collections.list # → stdlib/aurora/collections/list.aur
```

**Relative Import**:
```aurora
import ./utils          # → ./utils.aur
import ../common/types  # → ../common/types.aur
```

**Package Declaration**:
```aurora
package com.example.myapp

fn main() -> int {
    // Functions from stdlib prelude work automatically
    println("Hello")
    return 0
}
```

### Testing Module Resolution

```bash
# Test from project root
cd AuroraLang
./build/aurora tests/stdlib/testMath.aur  # ✓ Works

# Test from tests directory  
cd tests
../build/aurora stdlib/testMath.aur      # ✓ Works

# Test from arbitrary directory
cd /tmp
/path/to/AuroraLang/build/aurora /path/to/test.aur  # ✓ Works

# Custom stdlib location
export AURORA_STDLIB_PATH=/custom/stdlib
aurora myprogram.aur  # ✓ Uses custom stdlib
```

### Debugging Module Resolution

Enable debug logging to see module search:

```bash
aurora --debug myprogram.aur
```

Output:
```
[DEBUG] [Modules] Initializing module system...
[DEBUG] [Modules] Using compile-time stdlib path: /path/to/stdlib/aurora
[DEBUG] [Modules] Added module search path: /path/to/stdlib/aurora
[DEBUG] [Modules] Added module search path: .
[INFO ] Module system initialized with 2 search path(s)
[INFO ] Standard library: /path/to/stdlib/aurora
```

### Common Issues and Solutions

**Issue**: `Module file not found: core.prelude`

**Solution**:
1. Check stdlib path exists:
   ```bash
   ls $AURORA_STDLIB_PATH  # or
   ls /path/to/AuroraLang/stdlib/aurora
   ```

2. Verify with debug flag:
   ```bash
   aurora --debug program.aur | grep stdlib
   ```

3. Set explicit path if needed:
   ```bash
   export AURORA_STDLIB_PATH=/correct/path/to/stdlib/aurora
   ```

**Issue**: Import works from root but not from subdirectory

**Solution**: This should no longer happen with sysroot! The system uses absolute paths. If it does:
1. Rebuild to pick up latest module system:
   ```bash
   cd build && cmake .. && make
   ```

2. Verify sysroot is resolved correctly:
   ```bash
   aurora --debug program.aur | grep Sysroot
   ```

### Library Deployment

#### Option 1: Portable Distribution (Recommended)

Create a self-contained package that works anywhere:

```bash
# Build distribution
cd build
make dist

# Distribution created in: ../dist/
# Copy to any location:
cp -r ../dist /opt/aurora
cd /opt/aurora
./bin/aurora program.aur  # Auto-detects sysroot!
```

**Distribution Structure**:
```
dist/
├── README.txt                    # Usage instructions
├── bin/
│   └── aurora                    # Compiler executable
├── lib/
│   ├── libaurora_runtime.a       # Static libraries
│   ├── libaurora_runtime.dylib   # Dynamic libraries
│   ├── libaurora_stdlib.a
│   └── libaurora_stdlib.dylib
├── stdlib/
│   └── aurora/                   # Aurora source stdlib
│       ├── core/
│       ├── collections/
│       ├── math/
│       └── string/
└── include/                      # C/C++ interop headers
    ├── aurora_runtime.h
    └── aurorax/
```

**Benefits**:
- ✅ No installation required
- ✅ Copy to USB drive, Docker container, anywhere
- ✅ Multiple versions side-by-side
- ✅ Automatic sysroot detection from `bin/` location

#### Option 2: System Installation

Install to system paths (requires sudo):

```bash
cd build
sudo cmake --install . --prefix /usr/local
```

**Installed Layout**:
```
/usr/local/
├── bin/aurora
├── lib/
│   ├── libaurora_runtime.{a,so}
│   ├── libaurora_stdlib.{a,so}
│   └── aurora/stdlib/
└── include/
```

**Setup**:
```bash
export PATH="/usr/local/bin:$PATH"
export AURORA_HOME="/usr/local"
aurora program.aur
```

#### Runtime Linking Options

- **Static linking** (default): Everything bundled in executable
- **Dynamic linking**: Requires libraries in system path
  - Linux: `LD_LIBRARY_PATH`
  - macOS: `DYLD_LIBRARY_PATH`
  - Windows: `PATH`

#### Comparison: build/ vs dist/ vs install

| Aspect | `build/` | `dist/` | System Install |
|--------|----------|---------|----------------|
| Purpose | Development | Distribution | System-wide |
| Portable | ❌ | ✅ | ❌ |
| Requires Build | ✅ | ❌ | ❌ |
| Sudo Needed | ❌ | ❌ | ✅ |
| Best For | Dev/testing | Deployment | Production servers |

---

## Contributing Guidelines

### Code Style

**C++ Code:**
- Follow existing code style
- Use meaningful names
- Keep functions focused
- Add comments for complex logic

**AuroraLang Code:**
- No semicolons (optional)
- Use 4 spaces for indentation
- Follow naming conventions

### Commit Messages

Use clear, descriptive messages:

```
Good:
✅ "Add bitwise OR operator support"
✅ "Fix null pointer dereference in parser"
✅ "Implement string concatenation in codegen"

Bad:
❌ "fix bug"
❌ "update code"
❌ "changes"
```

### Pull Request Process

1. **Fork** the repository
2. **Create a branch**: `git checkout -b feature/my-feature`
3. **Make changes** with clear commits
4. **Run tests**: `./run_tests.sh`
5. **Update documentation**
6. **Push**: `git push origin feature/my-feature`
7. **Create Pull Request** with clear description

### PR Checklist

- [ ] Code follows project style
- [ ] All tests pass
- [ ] New tests added for new features
- [ ] Documentation updated
- [ ] No merge conflicts
- [ ] Commit messages are clear

---

## Testing

### Running Tests

```bash
# Run all tests
./run_tests.sh

# Run specific test
./build/aurora tests/basic/arithmetic_operators.aur

# Run with debug output
./build/aurora --debug tests/basic/arithmetic_operators.aur
```

### Test Organization

Tests are organized by feature:

```
tests/
├── basic/           # Basic language features
├── arrays/          # Array operations
├── oop/             # Object-oriented programming
├── control_flow/    # If/while/for/loop
├── type_system/     # Type system tests
├── operators/       # All operators
└── errors/          # Error handling tests
```

### Writing Tests

Create `.aur` test files:

```aurora
# tests/basic/my_feature.aur
extern printd(x)

fn test_feature() -> int {
    # Test code
    return 0
}

fn main() -> int {
    return test_feature()
}
```

**Requirements:**
- Test must compile and run
- Expected output should be documented
- Cover edge cases
- Test error conditions

### Test Principles

1. **Comprehensive Coverage**: Test all paths
2. **Clear Intent**: Name tests descriptively
3. **Isolated**: Each test is independent
4. **Fast**: Tests should run quickly
5. **Reliable**: Tests should not be flaky

---

## Documentation

### When to Update Documentation

**Always update docs when:**
- Adding new language features
- Changing syntax or semantics
- Adding new compiler options
- Modifying architecture

### Documentation Files

- **GETTING_STARTED.md** - Quick start for new users
- **TUTORIAL.md** - Step-by-step learning
- **LANGUAGE_SPEC.md** - Complete language reference
- **ARCHITECTURE.md** - System design and internals
- **OOP_FEATURES.md** - Object-oriented programming
- **DEVELOPER_GUIDE.md** - This file
- **EXAMPLES.md** - Example programs
- **WHY_AURORALANG.md** - Value proposition

### Documentation Standards

- **Use English** for all docs and examples
- **Keep it concise** but complete
- **Include examples** for all features
- **Update related docs** when changing features

### Example Program Requirements

Example programs must:
- Actually run and produce expected output
- Be well-commented
- Demonstrate one concept clearly
- Use descriptive names
- Be under 100 lines when possible

---

## Development Workflow

### Feature Development

1. **Plan**: Read roadmap and architecture docs
2. **Design**: Consider impact on all layers
3. **Implement**: Write code with logging
4. **Test**: Add comprehensive tests
5. **Document**: Update relevant docs
6. **Review**: Self-review before PR

### Bug Fixing

1. **Reproduce**: Create minimal test case
2. **Debug**: Use `--debug` or `--trace` flags
3. **Fix**: Make minimal necessary changes
4. **Test**: Add regression test
5. **Document**: Update if behavior changed

### Debugging Process

When encountering issues:

1. **Enable Debug Mode**: Use `--debug` or `--trace`
2. **Check LLVM IR**: Use `--emit-llvm`
3. **Verify Module**: Check automatic verification output
4. **Isolate Issue**: Create minimal reproduction
5. **Check Architecture**: Ensure layer boundaries respected

---

## Best Practices

### Compiler Development

1. **Maintain Layer Separation**: Don't mix compiler/runtime/LLVM logic
2. **Use Runtime API**: Don't generate malloc directly
3. **Verify IR**: Always verify generated LLVM IR
4. **Log Appropriately**: Use correct log levels
5. **Handle Errors**: Provide clear error messages

### Code Organization

1. **One Responsibility**: Each file has one purpose
2. **Small Files**: Keep files under 500 lines
3. **Clear Names**: File names indicate content
4. **Forward Declarations**: Minimize includes
5. **Shared Helpers**: Extract common utilities

### Performance

1. **Profile First**: Measure before optimizing
2. **Zero Cost**: High-level features should compile to efficient code
3. **Optimize Compiler**: Fast compilation is important
4. **Test Performance**: Add benchmarks for critical paths

---

## Common Issues

### Build Problems

**Issue**: LLVM not found
```bash
# macOS: Set LLVM path
cmake .. -DCMAKE_PREFIX_PATH="$(brew --prefix llvm)"

# Linux: Install llvm-dev
sudo apt-get install llvm-dev
```

**Issue**: Compilation errors
```bash
# Clean and rebuild
rm -rf build
mkdir build && cd build
cmake ..
make clean && make
```

### Runtime Issues

**Issue**: Segmentation fault
- Enable crash handler (automatic)
- Use `--debug` to see execution flow
- Check generated LLVM IR with `--emit-llvm`

**Issue**: Wrong output
- Add `printd()` debugging
- Use `--trace` to see all phases
- Verify LLVM IR is correct

---

## Getting Help

### Resources

- **Architecture**: [ARCHITECTURE.md](ARCHITECTURE.md)
- **Language Spec**: [LANGUAGE_SPEC.md](LANGUAGE_SPEC.md)
- **Examples**: `tests/` directory
- **Roadmap**: [ROADMAP.md](ROADMAP.md)

### Community

- **GitHub Issues**: Bug reports and questions
- **GitHub Discussions**: General discussions
- **Pull Requests**: Code review and feedback

### Response Time

We aim to respond to:
- Bug reports: Within 48 hours
- Pull requests: Within 1 week
- Feature requests: Within 2 weeks

---

## Recognition

We value all contributions! Contributors will be:
- Listed in CONTRIBUTORS.md
- Mentioned in release notes
- Invited to join core team (for sustained contributions)

---

**Thank you for contributing to AuroraLang! 🚀**

