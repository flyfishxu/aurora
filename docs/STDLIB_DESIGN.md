# AuroraLang Standard Library Design

**Version**: 1.0  
**Status**: Implemented  
**Goal**: Build a world-class standard library inspired by Kotlin and Swift

---

## Design Philosophy

### Core Principles

1. **Intuitive and Natural**: APIs should feel natural, like Kotlin/Swift
2. **Type-Driven**: Leverage the type system for safety and clarity
3. **Minimal Boilerplate**: Reduce repetitive code through smart design
4. **Performance-Conscious**: Zero-cost abstractions where possible
5. **Future-Proof**: Design for generics and extension methods

### Layered Architecture

```
┌─────────────────────────────────────────────────┐
│         User Code (Aurora)                      │
│  - Natural API usage                            │
│  - Auto-imported prelude                        │
├─────────────────────────────────────────────────┤
│    Aurora Standard Library (*.aur)              │
│  - High-level algorithms                        │
│  - Type-safe wrappers                           │
│  - Elegant, composable functions                │
├─────────────────────────────────────────────────┤
│    Native Extensions (C++/C)                    │
│  - Performance-critical operations              │
│  - System integration                           │
│  - Clean, minimal C API                         │
├─────────────────────────────────────────────────┤
│         Runtime (C)                             │
│  - Memory management (ARC)                      │
│  - Core data structures                         │
└─────────────────────────────────────────────────┘
```

---

## Module Organization

### Current Structure

```
stdlib/
├── include/aurorax/          # Native C++ interfaces
│   └── StdLib.h             # Core native functions
├── src/
│   └── StdLib.cpp           # Native implementations
└── aurora/                   # Aurora standard library
    ├── core/                # Core utilities (auto-imported)
    │   ├── prelude.aur     # Auto-imported prelude
    │   ├── types.aur       # Type utilities
    │   └── result.aur      # Result/Option types
    ├── collections/         # Data structures
    │   ├── array.aur       # Array extensions
    │   ├── list.aur        # List<T> (future)
    │   └── map.aur         # Map<K,V> (future)
    ├── math/                # Mathematical functions
    │   ├── basic.aur       # Basic operations
    │   └── advanced.aur    # Trigonometry, etc.
    ├── text/                # String operations
    │   └── string.aur      # String extensions
    ├── io/                  # Input/Output
    │   ├── console.aur     # Console I/O
    │   └── file.aur        # File operations
    └── time/                # Time and date
        └── duration.aur    # Time utilities
```

---

## Naming Conventions

### Native Functions (C++)

**OLD (Bad)**:
```cpp
auroraStdPrintInt(int64_t value)
auroraStdStringLength(const char* str)
auroraStdRandomInt(int64_t min, int64_t max)
```

**NEW (Good)**:
```cpp
// Format: aurora_category_action
aurora_print_int(int64_t value)
aurora_string_length(const char* str)
aurora_random_int(int64_t min, int64_t max)

// Or even simpler when category is obvious:
print_int(int64_t value)
string_length(const char* str)
random_int(int64_t min, int64_t max)
```

**Rationale**: 
- Shorter, cleaner names
- Snake_case for C functions (industry standard)
- Still namespaced to avoid conflicts

### Aurora Functions

**Current Issues**:
```aurora
absInt(x: int)           // Type suffix is ugly
absDouble(x: double)     // Duplicate logic
sumIntArray(arr, len)    // Manual length passing
```

**Improved Design** (Phase 1 - Before generics):
```aurora
// Use overloading instead of type suffixes
fn abs(x: int) -> int
fn abs(x: double) -> double

// Still need length parameter for now, but better naming
fn sum(arr: [int], length: int) -> int
fn sum(arr: [double], length: int) -> double
```

**Future Design** (Phase 2 - With generics and extensions):
```aurora
// Generic functions
fn abs<T: Numeric>(x: T) -> T

// Extension methods (like Kotlin/Swift)
extension Array<T> {
    fn size() -> int           // No manual length!
    fn sum() -> T where T: Numeric
    fn map<R>(transform: (T) -> R) -> Array<R>
}

// Usage:
let arr = [1, 2, 3]
arr.size()        // Returns 3
arr.sum()         // Returns 6
arr.map(x => x * 2)  // [2, 4, 6]
```

---

## Function Organization Strategy

### Principle 1: Type-Centric Organization

**Bad** (Function-centric):
```aurora
// All array functions in one file
fn sumIntArray(...)
fn sumDoubleArray(...)
fn reverseIntArray(...)
fn reverseDoubleArray(...)
```

**Good** (Type-centric):
```aurora
// arrays/int_array.aur
fn sum(arr: [int], length: int) -> int
fn reverse(arr: [int], length: int) -> void
fn sorted(arr: [int], length: int) -> [int]

// arrays/double_array.aur
fn sum(arr: [double], length: int) -> double
fn reverse(arr: [double], length: int) -> void
fn sorted(arr: [double], length: int) -> [double]
```

**Rationale**: Easier to find and understand, maps to future extension methods

### Principle 2: Overloading Over Suffixes

**Bad**:
```aurora
fn minInt(a: int, b: int) -> int
fn minDouble(a: double, b: double) -> double
fn maxInt(a: int, b: int) -> int
fn maxDouble(a: double, b: double) -> double
```

**Good**:
```aurora
fn min(a: int, b: int) -> int
fn min(a: double, b: double) -> double
fn max(a: int, b: int) -> int
fn max(a: double, b: double) -> double
```

**Rationale**: Cleaner, more natural, prepares for generics

### Principle 3: Consistent Parameter Order

```aurora
// Always: (primary data, length, additional params...)
fn sum(arr: [int], length: int) -> int
fn contains(arr: [int], length: int, value: int) -> bool
fn indexOf(arr: [int], length: int, value: int) -> int
```

---

## Prelude Design

### What Gets Auto-Imported

The prelude should include only essential, frequently-used functions:

```aurora
// core/prelude.aur
import "types"           // Basic type utilities
import "../math/basic"   // abs, min, max, etc.
import "../collections/array"  // Array utilities
import "../io/console"   // print, println
```

### What Users Import Explicitly

```aurora
import "io/file"      // File operations
import "time"         // Time utilities
import "collections/list"  // Advanced collections
```

---

## Migration Path

### Phase 1: Current State (v0.6.x) ✅
- Basic stdlib with function overloading
- Manual length parameters for arrays
- Type-specific functions where needed
- Clean C++ function names

### Phase 2: Extensions (v0.8-v0.9)
- Extension methods for built-in types
- `array.size()`, `string.length()`
- Better ergonomics

### Phase 3: Generics (v1.0+)
- Generic collections: `List<T>`, `Map<K,V>`
- Generic algorithms: `sort<T>()`, `map<T,R>()`
- Full type safety

### Phase 4: Advanced (v1.5+)
- Traits/Protocols
- Operator overloading
- Advanced metaprogramming

---

## Comparison with Target Languages

### Kotlin Stdlib

**Strengths**:
- Extension functions everywhere
- Excellent collection APIs
- `map`, `filter`, `reduce` are first-class
- Nullable types well-integrated

**Aurora's Approach**:
- Phase 1: Function-based APIs
- Phase 2: Extension methods (like Kotlin)
- Phase 3: Generic collections
- Already have: Nullable types `T?`

### Swift Standard Library

**Strengths**:
- Protocol-oriented programming
- Value semantics
- Powerful generics
- Clean, minimal API surface

**Aurora's Approach**:
- Similar value semantics (immutable by default)
- Future: Protocol/trait system
- Clean APIs inspired by Swift
- Performance-conscious (ARC like Swift)

---

## Best Practices

### For Native Functions (C++)

1. **Keep it minimal**: Only performance-critical or system operations
2. **Use simple types**: `int64_t`, `double`, `const char*`
3. **No complex logic**: Move complex algorithms to Aurora layer
4. **Memory safety**: Clear ownership semantics

### For Aurora Functions

1. **Type safety first**: Use the type system to prevent errors
2. **Composability**: Small, composable functions
3. **Consistency**: Similar operations should work similarly
4. **Documentation**: Clear, concise comments

### For Prelude

1. **Only essentials**: Don't pollute the global namespace
2. **No surprises**: Users should expect what's included
3. **Fast to load**: Keep it lightweight

---

## Implementation Status

### ✅ Completed
- Clean C++ function naming
- Function overloading for basic types
- Organized module structure
- Auto-imported prelude

### 🚧 In Progress
- Comprehensive stdlib coverage
- Improved array APIs
- String operations

### 📋 Planned
- Extension methods (needs compiler support)
- Generic collections (needs generics support)
- Advanced algorithms
- Full Kotlin/Swift parity

---

## Conclusion

This design provides:
1. **Immediate benefits**: Cleaner, more intuitive APIs
2. **Future-proof**: Prepared for generics and extensions
3. **World-class**: Inspired by the best modern languages
4. **Pragmatic**: Implementable in phases

**The goal is not to copy Kotlin or Swift, but to learn from them and build something even better.**

