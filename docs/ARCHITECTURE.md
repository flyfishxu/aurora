# AuroraLang Architecture

**Design Philosophy**: Strict separation of LLVM, Compiler, Runtime, and Standard Library layers to avoid semantic leakage and inter-layer coupling.

---

## 🎯 Core Principles

### 1. Four-Layer Architecture
```
┌─────────────────────────────────────────────────┐
│            Standard Library (Std)               │  Written in AuroraLang
│     (Collections, Algorithms, I/O, etc.)        │  Depends on Runtime API
├─────────────────────────────────────────────────┤
│             Runtime Layer                       │  Written in C
│  (Memory Management, Arrays, Objects, etc.)     │  Provides C API
├─────────────────────────────────────────────────┤
│           Compiler Layer (Frontend)             │  Written in C++
│    (Lexer, Parser, Type System, CodeGen)        │  Generates LLVM IR
├─────────────────────────────────────────────────┤
│             LLVM Layer (Backend)                │  LLVM Library
│        (IR, Optimization, Code Generation)      │  Independent Infrastructure
└─────────────────────────────────────────────────┘
```

### 2. Clear Responsibilities
- **LLVM**: Low-level IR, optimization, machine code generation
- **Compiler**: High-level language semantics → LLVM IR translation
- **Runtime**: Memory management, data structures, runtime checks
- **Std**: High-level APIs written in AuroraLang itself

### 3. Zero Coupling
- Each layer only knows about layers below it
- No layer should know implementation details of layers above
- Clean API boundaries between all layers

---

## 📂 Project Structure

### Directory Organization

AuroraLang follows standard C++ project organization with clear separation between interface (headers) and implementation (source files).

```
AuroraLang/
├── compiler/                  # Compiler Layer (C++)
│   ├── include/aurora/        # Public compiler headers
│   │   ├── AST.h             # AST node definitions
│   │   ├── CodeGen.h         # Code generation context
│   │   ├── Diagnostic.h      # Error reporting
│   │   ├── Lexer.h           # Tokenization interface
│   │   ├── Logger.h          # Logging system
│   │   ├── Parser.h          # Parsing interface
│   │   ├── Type.h            # Type system interface
│   │   ├── Colors.h          # ANSI colors
│   │   └── CrashHandler.h    # Crash handling
│   ├── src/                  # Compiler implementation
│   │   ├── lexer/
│   │   │   └── Lexer.cpp
│   │   ├── parser/
│   │   │   └── Parser.cpp
│   │   ├── types/
│   │   │   └── Type.cpp
│   │   ├── ast/
│   │   │   ├── AST.cpp
│   │   │   └── ModuleLoader.cpp
│   │   ├── codegen/          # Modular code generation
│   │   │   ├── CodeGen.cpp
│   │   │   ├── ExprCodeGen.cpp
│   │   │   ├── StmtCodeGen.cpp
│   │   │   ├── ClassCodeGen.cpp
│   │   │   └── ArrayCodeGen.cpp
│   │   ├── diagnostic/
│   │   │   └── Diagnostic.cpp
│   │   ├── logger/
│   │   │   └── Logger.cpp
│   │   └── utils/
│   │       └── CrashHandler.cpp
│   └── CMakeLists.txt        # Compiler build config
│
├── runtime/                   # Runtime Layer (C)
│   ├── include/
│   │   └── aurora_runtime.h  # Runtime API definitions
│   ├── src/
│   │   └── aurora_runtime.c  # Runtime implementation
│   └── CMakeLists.txt        # Runtime build config
│
├── stdlib/                    # Standard Library Layer
│   ├── include/aurorax/       # C++ stdlib headers
│   │   └── StdLib.h          # C++ stdlib interface
│   ├── src/                  # C++ stdlib implementation
│   │   └── StdLib.cpp        # I/O, math, strings, etc.
│   ├── aurora/               # Aurora stdlib source code
│   │   ├── core/             # Core utilities
│   │   │   ├── prelude.aur
│   │   │   └── assert.aur
│   │   ├── collections/      # Data structures
│   │   │   └── array_utils.aur
│   │   ├── math/             # Mathematical functions
│   │   │   └── math.aur
│   │   ├── string/           # String utilities
│   │   │   └── string_utils.aur
│   │   └── io/               # I/O operations (future)
│   └── CMakeLists.txt        # Stdlib build config
│
├── tools/                     # Tools
│   └── aurora-cli/           # Command-line interface
│       ├── src/
│       │   └── main.cpp      # Main entry point
│       └── CMakeLists.txt
│
├── tests/                     # Test Suite
│   ├── basic/                # Basic language tests
│   ├── arrays/               # Array tests
│   ├── oop/                  # OOP tests
│   ├── control_flow/         # Control flow tests
│   ├── type_system/          # Type system tests
│   ├── operators/            # Operator tests
│   ├── modules/              # Module system tests
│   ├── errors/               # Error handling tests
│   └── README.md
│
├── docs/                      # Documentation
│   ├── GETTING_STARTED.md
│   ├── TUTORIAL.md
│   ├── LANGUAGE_SPEC.md
│   ├── ARCHITECTURE.md
│   ├── OOP_FEATURES.md
│   ├── DEVELOPER_GUIDE.md
│   ├── WHY_AURORALANG.md
│   ├── ROADMAP.md
│   └── EXAMPLES.md
│
├── vscode-extension/          # VSCode Extension
│   ├── syntaxes/             # Syntax highlighting
│   ├── snippets/             # Code snippets
│   └── src/                  # Extension source
│
├── build/                     # Build artifacts (generated)
├── CMakeLists.txt            # Main build configuration
└── README.md                 # Project overview
```

### Why Separate include/ and src/?

This organization provides significant benefits:

#### 1. Clean API Separation ✅

Headers define the **public interface**, implementation files contain **private details**.

```cpp
// include/aurora/Lexer.h - Public Interface
class Lexer {
public:
    Token nextToken();  // What users need
private:
    char current();     // Hidden implementation
};

// src/lexer/Lexer.cpp - Implementation
Token Lexer::nextToken() {
    // Implementation details
}
```

#### 2. Faster Compilation ⚡

- Headers included in multiple files
- Separating interface from implementation reduces recompilation
- Changes to .cpp don't trigger recompilation of dependents (if interface unchanged)

#### 3. Better Code Organization 📁

- Clear distinction between API and implementation
- Easier to understand what's public vs private
- Standard practice in C++ projects

#### 4. Library Packaging 📦

- If AuroraLang becomes a library, only `include/` needs distribution
- Users don't need source code, just headers and compiled library

#### 5. IDE Support 🔧

- IDEs use headers for autocomplete and type checking
- Separate headers make it easier for tools to parse the codebase

### Header Organization

#### What Goes in Headers (.h)

✅ **Should Include:**
- Class declarations
- Function declarations
- Enum definitions
- Template definitions (must be in header)
- Inline functions
- Constants (inline constexpr)

❌ **Should NOT Include:**
- Function implementations (except inline/template)
- Static variable definitions
- Implementation details
- Unnecessary #includes

#### Include Rules

1. **Use `#pragma once`** instead of include guards
2. **Include only what you use directly** in the header
3. **Prefer forward declarations** over #include when possible
4. **Include in .cpp files** what you need for implementation
5. **Order includes**: Own header first, project headers, system headers

### Build System

The project uses **modular CMake** with separate build configs for each layer:

```cmake
CMakeLists.txt                # Main build config
├── runtime/CMakeLists.txt    # Builds aurora_runtime (static lib)
├── compiler/CMakeLists.txt   # Builds aurora_compiler (static lib)
├── stdlib/CMakeLists.txt     # Builds aurora_stdlib (static lib)
└── tools/aurora-cli/CMakeLists.txt  # Builds aurora executable
```

**Build Order**:
1. Runtime Layer → `libaurora_runtime.a`
2. Compiler Layer → `libaurora_compiler.a`
3. Stdlib Layer → `libaurora_stdlib.a`
4. Aurora CLI → `aurora` (executable)

**Output**: `build/tools/aurora-cli/aurora` (symlinked to `build/aurora` for backward compatibility)

---

## 1️⃣ LLVM Layer (Backend)

### Purpose
- Provide LLVM IR instruction set
- Execute optimization passes
- Generate target machine code
- JIT execution engine

### What LLVM Should NOT Know
- ❌ Aurora language semantics
- ❌ Aurora type system details
- ❌ Aurora memory management strategies

### Example
```llvm
; LLVM layer only deals with IR like this:
define i64 @compute(i64 %x, i64 %y) {
    %result = add i64 %x, %y
    ret i64 %result
}
```

### Integration
- Used through LLVM C++ API
- IR Builder for code generation
- OrcJIT for just-in-time execution

---

## 2️⃣ Compiler Layer (Frontend)

### Components

#### Lexer (`src/lexer/`)
- Tokenization of source code
- Keyword recognition
- Operator parsing
- Comment handling

#### Parser (`src/parser/`)
- Recursive descent parsing
- Operator precedence climbing
- AST construction
- Error recovery

#### Type System (`src/types/`)
- Primitive types: `int`, `double`, `bool`, `string`, `void`
- Composite types: Arrays `[T]`, Optional `T?`, Classes
- Type checking and inference
- Type conversions

#### AST (`src/ast/`)
- AST node class definitions
- Type interfaces
- Minimal infrastructure code

#### CodeGen (`src/codegen/`)
- **Modular code generation architecture**
- **ExprCodeGen.cpp**: All expression code generation
  - Literals (int, double, bool, string, null)
  - Binary and unary operations
  - Function calls
  - Ternary expressions
  - Type conversions
- **StmtCodeGen.cpp**: All statement code generation
  - Control flow (if, while, for, loop)
  - Variable declarations
  - Return statements
  - Assignments
  - Break/continue
- **ClassCodeGen.cpp**: Object-oriented code generation
  - Member access expressions
  - Method calls
  - Object instantiation (new)
  - Class and method definitions
- **ArrayCodeGen.cpp**: Array operations
  - Array literal creation
  - Array indexing
  - Runtime API integration
- **CodeGen.cpp**: Code generation context
  - LLVM builder management
  - Symbol table
  - Loop context tracking

### Key Principle: Compiler Calls Runtime

**WRONG ❌** - Direct memory management in compiler:
```cpp
// Bad: Compiler generating malloc calls
llvm::Function* mallocFunc = module.getFunction("malloc");
llvm::Value* ptr = builder.CreateCall(mallocFunc, {size});
// ... manual memory layout ...
```

**CORRECT ✅** - Compiler calls Runtime API:
```cpp
// Good: Compiler calls Runtime function
llvm::Function* arrayCreateFunc = 
    module.getFunction("aurora_array_create");
llvm::Value* array = builder.CreateCall(arrayCreateFunc, 
                                       {elemSize, elemCount});
```

### Responsibilities
- ✅ Translate Aurora semantics to IR
- ✅ Call Runtime API functions
- ✅ Type checking and validation
- ❌ **Never** implement runtime logic directly
- ❌ **Never** manually manage complex memory layouts

---

## 2️⃣.5 Memory Management: Automatic Reference Counting (ARC)

### Overview

AuroraLang uses **Automatic Reference Counting (ARC)** for memory management, inspired by Swift and Objective-C. This provides deterministic, predictable memory management without garbage collection.

### Key Benefits

- ✅ **Zero Manual Memory Management**: Compiler automatically inserts retain/release
- ✅ **Predictable Performance**: No GC pauses, deterministic cleanup
- ✅ **Memory Safety**: No use-after-free or memory leaks
- ✅ **Zero Runtime Dependencies**: No garbage collector required

### Architecture

```
┌────────────────────────────────────────┐
│      Compiler (Automatic)              │
│  - Inserts retain on assignment        │
│  - Inserts release on scope exit       │
│  - Tracks variable lifetimes           │
├────────────────────────────────────────┤
│      Runtime (C Functions)             │
│  - aurora_retain(ptr)                  │
│  - aurora_release(ptr)                 │
│  - Type-based cleanup                  │
├────────────────────────────────────────┤
│   Heap Objects (with ref count)        │
│  - AuroraObject, AuroraArray, String   │
│  - All contain AuroraRefCountHeader    │
└────────────────────────────────────────┘
```

### How It Works

#### 1. Object Creation
```aurora
let obj = new Counter(0)
```
- Object created with `ref_count = 1`
- Compiler tracks `obj` for scope-based cleanup

#### 2. Assignment
```aurora
var x = new Counter(10)  // ref_count = 1
x = new Counter(20)      // Old: release → ref_count=0 → freed
                          // New: ref_count = 1
```
- Compiler inserts `release` for old value
- Compiler inserts `retain` for new value

#### 3. Scope Exit
```aurora
fn test() {
    let obj = new Counter(0)  // ref_count = 1
    // ... use obj ...
}  // Scope end: compiler inserts release → freed
```

#### 4. Function Return
```aurora
fn create() -> Counter {
    let obj = new Counter(0)  // ref_count = 1
    return obj  // Ownership transferred to caller
                // Other locals released
}
```

### Implementation Details

**Compiler Responsibilities**:
- Track variable scopes
- Insert `aurora_retain()` on assignment
- Insert `aurora_release()` on scope exit
- Handle return value ownership

**Runtime Responsibilities**:
- Maintain reference counts
- Free objects when `ref_count` reaches 0
- Handle type-specific cleanup

### Performance Characteristics

| Operation | Cost |
|-----------|------|
| Retain | O(1) - Single increment |
| Release | O(1) - Decrement + conditional free |
| Object Creation | O(1) - malloc + initialize |
| Memory Overhead | 16 bytes per object (header) |

### Limitations

1. **Circular References**: Not currently handled (future: weak references)
2. **Thread Safety**: Not yet atomic (future: atomic ref counts)
3. **Optimization**: No retain/release elision yet

### Future Improvements

- Weak references to break cycles
- Compiler optimization to eliminate redundant retain/release
- Atomic reference counting for thread safety

---

## 3️⃣ Runtime Layer

### Purpose
The Runtime Layer provides **fundamental services** that the compiled code needs at runtime.

### Implementation Language
- **C Language** for:
  - High performance
  - Stable ABI
  - Easy linking
  - Zero overhead abstractions (in Release mode)

### Core APIs

#### Array Operations
```c
// Create a new array
AuroraArray* aurora_array_create(int64_t elem_size, int64_t count);

// Free array memory
void aurora_array_free(AuroraArray* array);

// Get array length
int64_t aurora_array_length(AuroraArray* array);

// Get pointer to element
void* aurora_array_get_ptr(AuroraArray* array, int64_t index, int64_t elem_size);

// Set element value
void aurora_array_set(AuroraArray* array, int64_t index, void* element, int64_t elem_size);

// Bounds checking (debug mode)
void aurora_array_bounds_check(AuroraArray* array, int64_t index);
```

#### Object Operations
```c
// Allocate object memory
AuroraObject* aurora_object_create(size_t size);

// Free object memory  
void aurora_object_free(AuroraObject* obj);
```

#### String Operations
```c
// Create string
AuroraString* aurora_string_create(const char* str);

// Free string
void aurora_string_free(AuroraString* str);

// Get string length
int64_t aurora_string_length(AuroraString* str);
```

#### Memory Management (ARC)
```c
// General allocation
void* aurora_malloc(size_t size);

// Free memory
void aurora_free(void* ptr);

// Automatic Reference Counting
void* aurora_retain(void* ptr);           // Increment ref_count
void aurora_release(void* ptr);           // Decrement ref_count, free if 0
int64_t aurora_get_ref_count(void* ptr);  // Get current ref_count (debugging)
```

#### Debug Support
```c
// Assert condition
void aurora_assert(int condition, const char* message);

// Panic and abort
void aurora_panic(const char* message);
```

### Design Principles
- ✅ Simple, clean C API
- ✅ Clear ownership semantics
- ✅ Switchable debug/release checks
- ✅ Zero overhead in release mode
- ✅ No dependence on compiler internals

### Data Structures

#### AuroraRefCountHeader
```c
typedef enum {
    AURORA_TYPE_OBJECT = 1,
    AURORA_TYPE_ARRAY = 2,
    AURORA_TYPE_STRING = 3
} AuroraObjectType;

typedef struct {
    int64_t ref_count;        // Reference count
    AuroraObjectType type;    // Type tag for proper cleanup
} AuroraRefCountHeader;
```

#### AuroraArray
```c
typedef struct {
    AuroraRefCountHeader header;  // ARC metadata
    int64_t length;                // Number of elements
    void* data;                    // Pointer to elements
} AuroraArray;
```

#### AuroraObject
```c
typedef struct {
    AuroraRefCountHeader header;  // ARC metadata
    void* vtable;                  // Virtual method table (future)
    void* data;                    // Actual object data
    size_t size;                   // Size of object
} AuroraObject;
```

#### AuroraString
```c
typedef struct {
    AuroraRefCountHeader header;  // ARC metadata
    int64_t length;                // String length
    char* data;                    // String data (null-terminated)
} AuroraString;
```

---

## 4️⃣ Standard Library Layer (Future)

### Purpose
Provide high-level data structures and algorithms written in Aurora itself.

### Implementation Language
- **AuroraLang** for most code
- **C Runtime** for performance-critical parts

### Planned Modules

#### Collections
```aurora
# std/collections/list.aur
class List<T> {
    priv var data: [T]
    priv var size: int
    priv var capacity: int
    
    pub fn new() -> List<T> { ... }
    pub fn push(item: T) { ... }
    pub fn pop() -> T? { ... }
    pub fn get(index: int) -> T? { ... }
}

# std/collections/map.aur
class HashMap<K, V> {
    pub fn new() -> HashMap<K, V> { ... }
    pub fn insert(key: K, value: V) { ... }
    pub fn get(key: K) -> V? { ... }
}
```

#### Algorithms
```aurora
# std/algorithms/sort.aur
fn quicksort<T>(arr: [T], compare: fn(T, T) -> int) { ... }
fn binary_search<T>(arr: [T], target: T) -> int? { ... }
```

#### I/O
```aurora
# std/io/file.aur
class File {
    pub fn open(path: string) -> File? { ... }
    pub fn read() -> string? { ... }
    pub fn write(content: string) { ... }
}
```

---

## 🔄 Data Flow Example: Array Creation

### Aurora Source Code
```aurora
let arr = [1, 2, 3, 4, 5]
```

### Compiler Layer (Generates IR)
```llvm
; Step 1: Call Runtime to create array
%array_ptr = call ptr @aurora_array_create(i64 8, i64 5)

; Step 2: Call Runtime to initialize elements
call void @aurora_array_set(ptr %array_ptr, i64 0, ptr %elem0, i64 8)
call void @aurora_array_set(ptr %array_ptr, i64 1, ptr %elem1, i64 8)
call void @aurora_array_set(ptr %array_ptr, i64 2, ptr %elem2, i64 8)
call void @aurora_array_set(ptr %array_ptr, i64 3, ptr %elem3, i64 8)
call void @aurora_array_set(ptr %array_ptr, i64 4, ptr %elem4, i64 8)

; Step 3: Get array metadata
%length = call i64 @aurora_array_length(ptr %array_ptr)
%data = load ptr, ptr %data_field_ptr
```

### Runtime Layer (C Implementation)
```c
AuroraArray* aurora_array_create(int64_t elem_size, int64_t count) {
    AuroraArray* arr = malloc(sizeof(AuroraArray));
    arr->length = count;
    arr->data = malloc(elem_size * count);
    memset(arr->data, 0, elem_size * count);
    return arr;
}

void aurora_array_set(AuroraArray* arr, int64_t idx, void* elem, int64_t size) {
    void* ptr = (char*)arr->data + (idx * size);
    memcpy(ptr, elem, size);
}
```

### LLVM Layer
- Optimize the IR
- Generate machine code
- JIT execute

---

## 🔧 Modular Code Generation Architecture

### Design Rationale

The compiler's code generation layer has been designed with **single responsibility principle** and **modularity** in mind. Instead of having a single monolithic file (previously `AST.cpp` with 1665 lines), the code generation is split into specialized modules.

### Module Breakdown

#### 1. ExprCodeGen.cpp (~500 lines)
**Responsibility**: Generate LLVM IR for all expression nodes

```cpp
// Expression types handled:
- IntLiteralExpr, DoubleLiteralExpr, BoolExpr, StringExpr, NullExpr
- VariableExpr (variable lookup and load)
- BinaryExpr (arithmetic, comparison, logical, bitwise)
- UnaryExpr (negation, not, bitwise not)
- CallExpr (function calls with type conversion)
- TernaryExpr (conditional expressions)
- NullCheckExpr
```

**Key Features**:
- Short-circuit evaluation for logical operators (`&&`, `||`)
- Automatic type promotion (int ↔ double)
- Boolean conversion helpers

#### 2. StmtCodeGen.cpp (~450 lines)
**Responsibility**: Generate LLVM IR for all statement nodes

```cpp
// Statement types handled:
- ReturnStmt (with type conversion)
- ExprStmt (expression as statement)
- VarDeclStmt (variable declaration)
- IfStmt (if-else with proper phi nodes)
- WhileStmt, ForStmt, LoopStmt (loops)
- BreakStmt, ContinueStmt (loop control)
- AssignStmt (variable and member assignment)
- Prototype, Function (function definitions)
```

**Key Features**:
- Loop context tracking for break/continue
- Implicit return for void functions
- Proper basic block management

#### 3. ClassCodeGen.cpp (~420 lines)
**Responsibility**: Generate LLVM IR for object-oriented features

```cpp
// OOP features handled:
- MemberAccessExpr (this.field)
- MemberCallExpr (obj.method(args))
- NewExpr (object instantiation)
- ThisExpr (this reference)
- ClassDecl (struct type generation)
- MethodDecl (method definition with 'this' parameter)
- Field assignment (this.field = value)
```

**Key Features**:
- Method name mangling (`ClassName_methodName`)
- Automatic `this` parameter injection
- Field initialization in constructors

#### 4. ArrayCodeGen.cpp (~200 lines)
**Responsibility**: Generate LLVM IR for array operations

```cpp
// Array features handled:
- ArrayLiteralExpr (array creation via Runtime API)
- ArrayIndexExpr (array element access)
```

**Key Features**:
- **Full Runtime API integration**
- Calls `aurora_array_create`, `aurora_array_set`
- Proper element type handling
- Bounds checking support (in Runtime)

### Architecture Comparison

#### Before: Monolithic Design ❌
```
AST.cpp (1665 lines)
├── All expression codegen
├── All statement codegen
├── All OOP codegen
├── All array codegen
└── Mixed helper functions
```
**Problems**:
- Hard to navigate and find specific functionality
- High cognitive load
- Difficult to test individual components
- Merge conflicts in team development
- Violates single responsibility principle

#### After: Modular Design ✅
```
src/codegen/
├── CodeGen.cpp (150 lines) - Context management
├── ExprCodeGen.cpp (500 lines) - Expressions only
├── StmtCodeGen.cpp (450 lines) - Statements only
├── ClassCodeGen.cpp (420 lines) - OOP only
└── ArrayCodeGen.cpp (200 lines) - Arrays only
```
**Benefits**:
- ✅ Easy to locate functionality
- ✅ Clear separation of concerns
- ✅ Independent testing
- ✅ Parallel development possible
- ✅ Smaller, manageable files
- ✅ Follows industry best practices (Kotlin, Swift, Rust)

### Code Organization Principles

1. **One Concern Per File**: Each file handles one category of code generation
2. **~200-500 Lines Per File**: Optimal for human comprehension
3. **Clear Naming**: File name indicates responsibility
4. **Forward Declarations**: Modules can call each other when needed
5. **Shared Helpers**: Common utilities in dedicated files

### Comparison with Modern Compilers

| Compiler | Architecture Style |
|----------|-------------------|
| **AuroraLang** | ✅ Modular (ExprCodeGen, StmtCodeGen, etc.) |
| **Kotlin** | ✅ Modular (IrExpression, IrStatement, etc.) |
| **Swift** | ✅ Modular (ExprGen, StmtGen, DeclGen, etc.) |
| **Rust** | ✅ Modular (expr, stmt, item modules) |
| **Clang** | ✅ Modular (CGExpr, CGStmt, CGClass, etc.) |

---

## ✅ Architecture Benefits

### 1. Clear Responsibility Boundaries
- Each layer focuses on its own task
- Easy to understand and maintain
- Reduces cognitive load

### 2. Independent Evolution
- **Runtime** can be optimized independently (add GC, memory pools, etc.)
- **Compiler** can be improved independently (better optimization, errors, etc.)
- **Std** can be extended rapidly in Aurora itself

### 3. Testability
- **Runtime**: Unit tests in C
- **Compiler**: Test IR generation
- **Std**: Aurora test framework

### 4. Performance
- Runtime in C = excellent performance
- Release mode can inline many Runtime calls
- Debug mode keeps full checks

### 5. Portability
- Runtime is pure C = easy to port
- LLVM supports multiple platforms
- Compiler only depends on LLVM

---

## 🚫 Anti-Patterns (Avoid These)

### ❌ Compiler Generates Complex Logic Directly
```cpp
// BAD: Compiler manually managing array memory
llvm::Value* malloc_call = builder.CreateCall(malloc, size);
llvm::Value* gep = builder.CreateGEP(...);
builder.CreateStore(...);
// Lots of manual memory layout code...
```

### ❌ Runtime Depends on Compiler Details
```c
// BAD: Runtime coupling with compiler
typedef struct {
    LLVMValue* llvm_value;  // ❌ Runtime shouldn't know LLVM
} AuroraArray;
```

### ❌ LLVM Layer Contains Aurora Semantics
```cpp
// BAD: LLVM Pass hardcoding Aurora semantics
if (function_name == "aurora_array_get") {
    // ❌ LLVM shouldn't know Aurora function names
}
```

### ❌ Std Layer Using Compiler Internals
```aurora
# BAD: Std trying to use compiler features
extern compiler_get_type(x)  // ❌
```

---

## 📊 Current Status (v0.6.0)

### ✅ Implemented
- **LLVM Layer**: Properly used ✓
- **Compiler Layer**: Complete modular architecture ✓
  - ✅ Modular code generation (4 specialized files)
  - ✅ Expression code generation
  - ✅ Statement code generation
  - ✅ OOP code generation
  - ✅ Array code generation
- **Runtime Layer**: Full implementation ✓
  - ✅ Array operations (full Runtime API)
  - ✅ Object operations
  - ✅ Memory management primitives
  - ✅ JIT symbol resolution
- **Standard Library Layer**: Basic implementation ✓
  - ✅ C++ stdlib foundation (I/O, math, strings, time, file operations)
  - ✅ Aurora stdlib core modules (math, assertions, array utilities)
  - ✅ Modular directory structure

### ⚠️ Partially Implemented
- **Runtime Layer**: 
  - ✅ ~~Objects (still using malloc, needs full Runtime API)~~ → Now using ARC
  - ⚠️ Strings (runtime support exists, language integration pending)
  - ✅ ~~Garbage Collection (not yet)~~ → ARC implemented

- **Standard Library Layer**:
  - ⚠️ Aurora stdlib modules (basic implementations, need language feature support)
  - ❌ Full I/O module
  - ❌ Networking module
  - ❌ Advanced collections (Vec, HashMap, etc.)

---

## 🎯 Refactoring Progress

### Phase 1: Create Runtime Layer ✅
- [x] Create `runtime/aurora_runtime.h` and `.c`
- [x] Implement array API
- [x] Implement object API
- [x] Modify CMake to compile Runtime
- [x] Link Runtime to compiler

### Phase 2: Modularize Compiler Architecture ✅
- [x] Split AST.cpp (1665 lines) into modular files
- [x] Create ExprCodeGen.cpp (expression generation)
- [x] Create StmtCodeGen.cpp (statement generation)
- [x] Create ClassCodeGen.cpp (OOP generation)
- [x] Create ArrayCodeGen.cpp (array generation)
- [x] Update CMakeLists.txt
- [x] All 30 tests passing ✓

### Phase 3: Complete Runtime Integration ✅
- [x] Modify `ArrayLiteralExpr::codegen()` to call Runtime ✅
- [x] Implement Automatic Reference Counting (ARC) ✅
- [x] Add retain/release runtime functions ✅
- [x] Compiler inserts retain/release calls automatically ✅

### Phase 4: Create Std Layer (Future)
- [ ] Design Std API
- [ ] Implement basic collections
- [ ] Implement I/O module

---

## 📚 References

- [LLVM Language Reference](https://llvm.org/docs/LangRef.html)
- [LLVM Programmer's Manual](https://llvm.org/docs/ProgrammersManual.html)
- [Rust's Design](https://doc.rust-lang.org/book/)
- [Swift Runtime Design](https://github.com/apple/swift/tree/main/stdlib/public/runtime)
- [Go Runtime Implementation](https://github.com/golang/go/tree/master/src/runtime)

---

## 🎓 Key Takeaways

1. **Separation of Concerns**: Each layer has a single, well-defined purpose
2. **API Boundaries**: Clean interfaces prevent coupling
3. **Performance**: Zero-cost abstractions through layering
4. **Maintainability**: Easy to modify one layer without affecting others
5. **Extensibility**: New features can be added at the appropriate layer

**The four-layer architecture is the foundation for building a modern, maintainable, and high-performance programming language.**
