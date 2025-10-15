# AuroraLang Development Roadmap

**Version**: 0.6.2  
**Last Updated**: 2025-10-15  
**Status**: Alpha - Active Development

---

## Overview

### Vision 🎯

AuroraLang aims to be the world's best programming language, combining:
- Modern, clean syntax inspired by Kotlin and Swift
- Native performance through LLVM compilation
- Memory safety without complexity
- Zero runtime dependencies

**Target Users**: Backend developers, systems programmers, and teams needing high-performance native applications.

### Current Status 📍

**Completion**: ~40%

**What Works**:
- ✅ Complete compilation pipeline (Lexer → Parser → AST → LLVM IR → JIT/AOT)
- ✅ Four-layer architecture (Compiler, LLVM, Runtime, Stdlib)
- ✅ Modern type system with null safety and type inference
- ✅ Object-oriented programming (classes, methods, constructors)
- ✅ Arrays with runtime bounds checking
- ✅ Control flow (if/else, loops, break/continue)
- ✅ Module system with imports
- ✅ Diagnostic system with debug mode
- ✅ Standard library foundation (C++ implementations for I/O, math, strings, time, files)

**Ready For**:
- Internal tools and scripts
- Algorithm prototyping
- Educational projects
- Early adopters

**Current Limitations**:
- ⚠️ Limited string operations  
- ⚠️ No inheritance or polymorphism
- ⚠️ No generics
- ⚠️ No error handling types (Result/Option)
- ⚠️ Object keyword exists but no static members yet (singleton requires manual instance management)

### Component Status by Layer

| Layer | Completion | Key Features |
|-------|-----------|--------------|
| 🔧 **Compiler** | ~35% | ✅ Lexer, Parser, AST, Type System, OOP basics<br>❌ Generics, Traits, Advanced types |
| ⚙️ **LLVM** | ~40% | ✅ IR generation, JIT compilation, Memory allocation, ARC<br>❌ Optimization pipeline, Vtables |
| 🏗️ **Runtime** | ~70% | ✅ Array ops, Object ops, String ops, ARC (Automatic Reference Counting)<br>❌ Advanced memory management |
| 📚 **Stdlib** | ~40% | ✅ Clean API design (Kotlin-inspired), Function overloading, Auto-import prelude<br>✅ Math utils, Array utils (sumOf, minOf), Conversion<br>❌ String methods, Collections (generics required) |
| 🛠️ **Tooling** | ~10% | ✅ CLI, VSCode syntax highlighting<br>❌ LSP, Package manager, Debugger |

---

## Completed Features ✅

### Core Language
- **Lexer & Parser**: Full tokenization, recursive descent parser, AST
- **Type System**: Primitives (int, double, bool, string, void), optional types (T?), type inference, null safety
- **Optional Semantics**: Null literals now produce well-typed optional values; equality/inequality with `null` yields proper boolean results
- **Control Flow**: if/else, while, for (range-based), loop, break, continue
- **Functions**: Definitions, calls, parameters, return values, built-in functions
- **Operators**: Arithmetic, comparison, logical, bitwise, ternary (`?:`)
- **Variables**: let/var declarations, optional semicolons
- **Memory Management**: Automatic Reference Counting (ARC) for heap-allocated objects
- **Package System**: Hierarchical package organization (Java/Kotlin style), package declarations, package-path imports

### Object-Oriented Programming
- **Classes**: Declaration, fields, methods, constructors
- **Multiple Constructors**: Constructor overloading based on parameter types
- **Primary Constructor**: Kotlin/Swift style syntax `class Point(let x: double, let y: double)`
- **Objects**: Creation with class name, member access, `this` keyword
- **Object Keyword**: `object` declarations for singleton pattern (syntactic foundation)
- **Access Modifiers**: pub/priv syntax (not enforced yet)

### Data Structures
- **Arrays**: Declaration `[1, 2, 3]`, indexing `arr[i]`, nested indexing `matrix[i][j]`
- **Runtime**: Bounds checking, array creation/access via C runtime

### Module & Package System
- **Package Declaration**: `package com.example.myapp` with hierarchical namespace organization
- **Package Imports**: `import com.example.MyClass` with automatic path resolution
- **Legacy Imports**: `import "module"` or `import "path/to/module"` (still supported)
- **Multi-file**: Compilation across files, circular import protection, module caching
- **Search Paths**: Configurable package search paths (`.`, `src`, `stdlib/aurora`)

### Standard Library Foundation
- **Auto-Import Prelude**: 
  - ✅ Prelude automatically loaded (like Kotlin stdlib)
  - ✅ No manual imports needed for core functions
  - ✅ Camel case naming convention

- **C++ Built-in Functions** (via runtime):
  - Print functions (printd, printi, printb, prints)
  - Math (sin, cos, tan, exp, log, pow, sqrt, floor, ceil, round)
  - Random (int, double, seed)
  - Time (now, nowMillis, sleep)
  - File I/O (readAll, write, append, exists, delete)
  - String ops (concat, compare, substring, conversions)
  - System (exit, getEnv, args)

- **Aurora Stdlib Modules**:
  - **Core**: assert, range, conversion utilities
  - **Math**: absInt/absDouble, minInt/maxInt, factorial, fibonacci, gcd, lcm, isPrime, sqrtApprox, clamp, sign
  - **Trigonometry**: degree/radian conversion, angle normalization, distance calculation
  - **Arrays**: sum, average, min/max, reverse, contains, indexOf, count, sort, binarySearch, fill, copy
  - **Bitwise**: popCount, bit manipulation, power of 2 checks, bit rotation

### Developer Experience
- **Diagnostics**: Rust-style error formatting, source snippets, color output, error codes
- **Debug Mode**: `--debug` flag for compiler internals
- **Testing**: Comprehensive test suite (~30 tests)
- **Build System**: Modular CMake with separate layer compilation

### Architecture
- **Four-Layer Separation**: Compiler, LLVM, Runtime (C), Stdlib (C++/Aurora)
- **Clean APIs**: Strict boundaries, zero-cost abstractions
- **Runtime Layer**: Dedicated C runtime for performance-critical operations

---

## In Progress 🚧

### Phase: Stdlib Integration (v0.6.x - v0.7)
**Priority**: 🔴 CRITICAL  
**Timeline**: 2-3 months

**Goals**:
1. ✅ Integrate stdlib functions as auto-imported prelude
2. ✅ Camel case naming convention for all stdlib functions
3. ✅ Comprehensive math, array, bitwise, and utility functions
4. Complete string type implementation with operations
5. Array property access (`arr.length`)

**Completed** (v0.6.1):
- ✅ Auto-import prelude (like Kotlin)
- ✅ Renamed all stdlib functions to camelCase
- ✅ Extended math utilities (fibonacci, clamp, sign)
- ✅ Extended array utilities (sum, average, fill, copy)
- ✅ Added trigonometry module
- ✅ Added bitwise operations module
- ✅ Added range and conversion utilities
- ✅ Updated all tests to use new naming

**Current Work**:
- [ ] String concatenation (`"Hello" + " World"`)
- [ ] String methods (split, trim, substring)
- [ ] Array length property
- [ ] Type system improvements for stdlib types

---

## Planned Features 📋

### v0.7-v0.8: Core Completeness (🔴 CRITICAL)
**Timeline**: 4-6 months

**Must Have**:
1. ✅ **~~Memory Management~~** (COMPLETED):
   - **Chosen**: Automatic Reference Counting (ARC)
   - **Status**: Fully implemented and working
   - **Features**:
     - Automatic retain/release inserted by compiler
     - Type-tagged objects for proper cleanup
     - Scope-based lifetime management
     - Zero manual memory management

2. **String Type** (Stdlib):
   - String indexing, length, comparison
   - String interpolation: `"Hello {name}"`
   - Full escape sequence support

3. **Collections** (Requires Generics):
   - Vec<T> - dynamic arrays
   - HashMap<K, V>
   - HashSet<T>
   - LinkedList<T>, Queue, Stack

4. **Error Handling** (Compiler + Stdlib):
   - Result<T, E> type
   - Option<T> refinement (already have T?)
   - Error propagation operator (`?`)
   - try/catch or equivalent
   - Stack traces

**Outcome**: Language usable for small personal projects

---

### v0.9-v1.0: Production Readiness (🟡 HIGH)
**Timeline**: 8-12 months

**Developer Experience**:
1. **Package Manager** (Tooling):
   - Package manifest (aurora.toml)
   - Dependency resolution
   - Semantic versioning
   - Package repository

2. **LSP & IDE Support** (Tooling):
   - Autocomplete, go to definition, find references
   - Hover documentation, signature help
   - Diagnostics, code actions, refactoring

3. **Testing Framework** (Tooling + Stdlib):
   - Test syntax: `#[test] fn test_name() { ... }`
   - Assertion macros, test runner
   - Code coverage, benchmarking

**Advanced Language Features**:
4. **Generics** (Compiler + LLVM):
   - Generic functions: `fn max<T>(a: T, b: T) -> T`
   - Generic classes: `class Box<T> { ... }`
   - Type constraints: `<T: Comparable>`
   - Monomorphization

5. **Traits/Interfaces** (Compiler + LLVM):
   - Trait declaration: `trait Drawable { ... }`
   - Implementation: `impl Drawable for Circle { ... }`
   - Default implementations, trait bounds
   - Operator overloading via traits

6. **OOP Enhancements** (Compiler + LLVM):
   - ✅ Object keyword (syntactic support for singleton pattern)
   - ✅ Multiple constructors (constructor overloading)
   - ✅ Primary constructor syntax (Kotlin/Swift style)
   - Class inheritance: `class Dog extends Animal`
   - Virtual methods, abstract classes
   - Destructors
   - Access modifier enforcement
   - Static members (required for true singleton behavior)

**Outcome**: Language suitable for serious production projects

---

### v1.1-v2.0: Maturity & Advanced Features (🟢 MEDIUM)
**Timeline**: 12-18 months

1. **Concurrency** (Mixed):
   - Thread spawning, channels, mutexes
   - Async/await syntax
   - Futures and promises
   - Thread-safe collections

2. **Pattern Matching** (Compiler):
   - Match expressions with exhaustiveness checking
   - Pattern guards, destructuring

3. **Advanced Stdlib** (Stdlib):
   - Regular expressions
   - JSON parsing/serialization
   - HTTP client/server
   - Advanced data structures

4. **Advanced Features** (🔵 LOW):
   - Macros (procedural and declarative)
   - Reflection/introspection
   - Inline assembly
   - Compile-time execution (const fn)
   - Custom allocators

**Outcome**: Feature-complete, production-ready language

---

## Timeline & Milestones

### Short-Term (Next 3 months)
1. ✅ ~~Complete stdlib integration~~ (v0.6.1)
2. ✅ ~~Auto-import prelude~~ (v0.6.1)
3. ✅ ~~Comprehensive stdlib utilities~~ (v0.6.1)
4. ✅ ~~Clean stdlib API design~~ (v0.6.2) - Kotlin-inspired naming
5. ✅ ~~Package system implementation~~ (v0.6.2) - Hierarchical package organization
6. [ ] String operations (in progress)
7. [ ] Array properties (planned - needs compiler support)

### Medium-Term (3-12 months)
5. ✅ ~~Memory management implementation~~ (ARC completed)
6. Collections (Vec, HashMap)
7. Error handling (Result/Option)
8. Generics system
9. Package manager basics

### Long-Term (1-3 years)
10. Traits/Interfaces
11. OOP inheritance
12. LSP and tooling
13. Concurrency (async/await)
14. Production case studies

### Critical Milestones
- **v0.8**: Basic usability (arrays, memory, strings) - ~6 months
- **v1.0**: Production-ready (generics, stdlib, tooling) - ~18 months
- **v1.5**: Feature-complete (concurrency, advanced features) - ~30 months
- **v2.0**: Mature ecosystem - ~36 months

---

## Technical Debt & Known Issues

### 🔴 Critical Issues

1. ✅ **~~Memory Leaks~~** (RESOLVED in v0.6.1)
   - **Solution**: Implemented Automatic Reference Counting (ARC)
   - Objects now automatically freed when ref_count reaches 0
   - Retain/Release calls inserted by compiler
   - No manual memory management needed

2. **Limited String Operations**
   - String type exists but no methods
   - **Fix**: Implement string stdlib
   - **Timeline**: v0.7

3. ✅ **~~Stdlib Not Integrated~~** (RESOLVED in v0.6.1)
   - **Solution**: Implemented auto-import prelude mechanism
   - Stdlib now automatically available without manual imports
   - All functions follow camelCase naming convention
   - Comprehensive utility modules added

4. **No Collections**
   - Cannot use Vec, HashMap, etc.
   - **Fix**: Implement generics + collections
   - **Blocker**: Requires generics (v0.8)

### 🟡 High Priority Issues

5. **Access Modifiers Not Enforced**
   - pub/priv syntax exists but not checked
   - **Fix**: Implement access control
   - **Timeline**: v0.9

6. **Limited Type Checking**
   - Some runtime type errors possible
   - **Fix**: Strengthen type checker
   - **Timeline**: v0.8

7. **No Optimization**
   - Code not optimized
   - **Fix**: Add LLVM optimization passes
   - **Timeline**: v0.9

8. **No Error Handling**
   - No Result/Option types
   - **Fix**: Implement error handling system
   - **Timeline**: v0.8

### 🟢 Medium Priority Issues

9. **No Operator Overloading** (needs traits) - v1.0
10. **VSCode Extension Basic** (needs LSP) - v1.0
11. **No Documentation Comments** - v0.9
12. **No Static Analysis/Linter** - v1.0

---

## Architecture Layers

### 🔧 Compiler (Front-end)
**Responsibility**: Language syntax, semantics, type checking
- Lexer, Parser, AST
- Type system, type inference
- Symbol table, scope management
- Semantic analysis
- Error diagnostics

### ⚙️ LLVM (Back-end)
**Responsibility**: Code generation, optimization
- LLVM IR generation
- JIT/AOT compilation
- Optimization passes
- Machine code generation
- Platform-specific code

### 🏗️ Runtime (C Layer)
**Responsibility**: Low-level operations
- Memory allocation (malloc/free)
- Array operations (create, get, set, bounds check)
- Object operations (create, free)
- String operations (create, free, length)
- Debug support (assert, panic)

### 📚 Standard Library (User-facing)
**Responsibility**: High-level APIs and utilities
- Collections (Vec, HashMap, etc.)
- I/O (console, file, network)
- String manipulation
- Math, time, date
- Error types (Result, Option)
- Concurrency primitives

---

## Conclusion

### Strengths
- ✅ Solid architectural foundation
- ✅ Clean, modern syntax
- ✅ Working compilation pipeline
- ✅ Basic OOP support
- ✅ Automatic memory management (ARC)
- ✅ Good developer experience (diagnostics, debug mode)

### Critical Next Steps
1. **Immediate** (v0.7): Stdlib integration, string operations
2. **Next 6 months** (v0.8): Collections, error handling, generics foundation
3. **Next year** (v1.0): Generics, traits, package manager, LSP

### Realistic Expectations
- **Now**: Good for learning, prototyping, algorithms
- **6 months**: Usable for hobby projects
- **18 months**: Ready for small production apps
- **3 years**: Mature, production-ready with ecosystem

**Current Focus**: We are in the v0.6-v0.7 transition, focusing on stdlib integration and string operations. The goal is to reach v0.8 (basic usability) within 6 months.
