# AuroraLang Language Specification

This document provides a complete and formal specification of the AuroraLang programming language.

---

## Table of Contents

1. [Introduction](#introduction)
2. [Lexical Structure](#lexical-structure)
3. [Syntax](#syntax)
4. [Semantics](#semantics)
5. [Type System](#type-system)
6. [Operators](#operators)
7. [Functions](#functions)
8. [External Declarations](#external-declarations)
9. [Program Structure](#program-structure)
10. [Code Generation](#code-generation)
11. [Runtime Behavior](#runtime-behavior)

---

## 1. Introduction

AuroraLang is a modern, statically-typed systems programming language built on LLVM infrastructure. It combines the clean syntax of contemporary languages with the raw performance of native compiled code.

### Design Goals

- **Performance**: Native code execution with LLVM-level optimizations and zero-cost abstractions
- **Simplicity**: Clean, intuitive syntax that reduces cognitive load and increases productivity
- **Safety**: Strong type system with null safety and memory safety guarantees
- **Productivity**: Fast compilation, clear error messages, and excellent tooling support
- **Portability**: True cross-platform capability without runtime dependencies

### Notation

This specification uses Extended Backus-Naur Form (EBNF) for syntax definitions:

- `::=` means "is defined as"
- `|` separates alternatives
- `*` means zero or more occurrences
- `+` means one or more occurrences
- `?` means zero or one occurrence
- `()` groups items
- `[]` denotes optional items (equivalent to `()?`)
- `""` denotes literal text

---

## 2. Lexical Structure

### 2.1 Character Set

AuroraLang source files are text files encoded in UTF-8 (though the current implementation primarily uses ASCII).

### 2.2 Tokens

The lexical structure consists of the following token types:

#### Keywords

```
fn          - Function definition
extern      - External function declaration
return      - Return statement
let         - Immutable variable declaration
var         - Mutable variable declaration
if          - Conditional statement
else        - Alternative conditional branch
while       - While loop
for         - For loop with range
loop        - Infinite loop
break       - Break out of loop
continue    - Continue to next iteration
in          - Range operator keyword (for loops)
match       - Pattern matching (reserved)
true        - Boolean literal
false       - Boolean literal
null        - Null literal
```

Keywords are reserved and cannot be used as identifiers.

> ℹ️ **Null safety**
>
> Optional values (`T?`) currently support equality checks with `null` (`== null` and `!= null`). Comparisons between two non-null optionals are planned but not yet implemented.

#### Identifiers

```ebnf
identifier ::= letter (letter | digit | "_")*
letter     ::= "a".."z" | "A".."Z" | "_"
digit      ::= "0".."9"
```

**Examples:**
```
x
myVariable
calculate_sum
_private
value123
```

**Rules:**
- Must start with a letter or underscore
- Can contain letters, digits, and underscores
- Case-sensitive (`foo` and `Foo` are different)
- Cannot be a keyword

#### Numeric Literals

```ebnf
number ::= digit+ ("." digit*)?
         | "." digit+
```

All numbers are interpreted as double-precision floating-point values (IEEE 754 64-bit).

**Examples:**
```
42
3.14159
0.5
.5
100.0
```

**Note:** The current implementation accepts malformed numbers like `1.2.3`, but `strtod` handles these gracefully by parsing up to the second dot.

#### Operators

```
+     Addition
-     Subtraction / Negation
*     Multiplication
/     Division
%     Modulo
<     Less than
>     Greater than
<=    Less than or equal
>=    Greater than or equal
==    Equal to
!=    Not equal to
&&    Logical AND
||    Logical OR
!     Logical NOT
..    Range operator (for loops)
```

#### Delimiters

```
(     Left parenthesis
)     Right parenthesis
{     Left brace
}     Right brace
[     Left bracket
]     Right bracket
,     Comma
;     Semicolon
:     Colon (type annotation)
?     Question mark (optional types)
->    Arrow (return type)
.     Dot (member access - reserved)
```

### 2.3 Comments

Single-line comments begin with `#` and extend to the end of the line.

```aurora
# This is a comment
fn add(a, b) {  # This is also a comment
  return a + b;
}
```

Multi-line comments are not currently supported.

### 2.4 Whitespace

Whitespace (spaces, tabs, newlines, carriage returns) is ignored except as a token separator.

```aurora
fn    add   (   a   ,   b   )   {   return   a   +   b   ;   }
```

is equivalent to:

```aurora
fn add(a, b) { return a + b; }
```

---

## 3. Syntax

### 3.1 Program Structure

```ebnf
program ::= declaration*

declaration ::= function_definition
              | external_declaration
```

A program consists of zero or more declarations. Each declaration is either a function definition or an external function declaration.

### 3.2 Function Definition

```ebnf
function_definition ::= "fn" identifier "(" parameter_list? ")" ("->" type)? block

parameter_list ::= identifier (":" type)? ("," identifier (":" type)?)*

type ::= "int" | "double" | "bool" | "string" | "void" | identifier

block ::= "{" statement* "}"

statement ::= expression ";"
            | return_statement
```

**Examples:**

```aurora
fn add(a, b) {
  return a + b;
}

fn add_typed(a: int, b: int) -> int {
  return a + b;
}

fn no_params() {
  return 42;
}

fn void_function() -> void {
  printd(1.0);
  # No return needed for void functions
}

fn multiple_statements(x) {
  printd(x);
  return x + 1;
}
```

### 3.3 External Declaration

```ebnf
external_declaration ::= "extern" identifier "(" parameter_list? ")" ";"?
```

External declarations specify functions that are defined outside AuroraLang (e.g., in C/C++).

**Examples:**

```aurora
extern printd(x);
extern sin(angle);
extern pow(base, exponent);
```

### 3.4 Expressions

```ebnf
expression ::= primary (binary_operator expression)?

primary ::= number
          | identifier
          | "(" expression ")"
          | function_call

function_call ::= identifier "(" argument_list? ")"

argument_list ::= expression ("," expression)*

binary_operator ::= "+" | "-" | "*" | "/" | "<"
```

### 3.5 Return Statement

```ebnf
return_statement ::= "return" expression? ";"?
```

**Rules:**
- Functions with a return type (int, double, bool, etc.) must return a value
- Functions with `void` return type cannot return a value
- Empty return statements (`return;` or just `return`) are only allowed in `void` functions
- `void` functions automatically return at the end if no explicit return is present

---

## 4. Semantics

### 4.1 Operator Precedence

Operators are evaluated according to precedence levels. Higher precedence binds tighter.

| Precedence | Operators | Associativity |
|------------|-----------|---------------|
| 40         | `*` `/`   | Left          |
| 20         | `+` `-`   | Left          |
| 10         | `<`       | Left          |

**Examples:**

```aurora
2 + 3 * 4        # Evaluated as 2 + (3 * 4) = 14
10 - 5 - 2       # Evaluated as (10 - 5) - 2 = 3
1 < 2 + 3        # Evaluated as 1 < (2 + 3) = 1.0
```

### 4.2 Expression Evaluation

All expressions evaluate to a double-precision floating-point value.

#### Arithmetic Operations

- **Addition** (`a + b`): Returns the sum of `a` and `b`
- **Subtraction** (`a - b`): Returns the difference `a - b`
- **Multiplication** (`a * b`): Returns the product of `a` and `b`
- **Division** (`a / b`): Returns the quotient `a / b`
  - Division by zero behavior is undefined (typically produces `inf` or `nan`)

#### Comparison Operations

- **Less Than** (`a < b`): Returns `1.0` if `a < b`, otherwise `0.0`

### 4.3 Function Calls

```ebnf
function_call ::= identifier "(" expression_list ")"
```

Function calls evaluate arguments left-to-right, then transfer control to the callee.

**Rules:**
- Number of arguments must match the function's parameter count
- Arguments are passed by value
- Return value is the result of the expression

---

## 5. Type System

### 5.1 Types

AuroraLang currently has a single type:

**Double** - IEEE 754 double-precision floating-point (64-bit)

### 5.2 Type Representation

All values are represented as doubles, including:
- Numbers: `42`, `3.14`
- Booleans: `0.0` (false), `1.0` (true)
- Function return values

### 5.3 Type Conversions

No explicit type conversions are needed since there is only one type.

For the comparison operator `<`:
- Returns `0.0` for false
- Returns `1.0` for true
- The result is converted back to a double via `UIToFP` LLVM instruction

---

## 6. Operators

### 6.1 Binary Operators

#### Arithmetic Operators

| Operator | Name           | LLVM Instruction | Example | Result |
|----------|----------------|------------------|---------|--------|
| `+`      | Addition       | `fadd`           | `2 + 3` | `5.0`  |
| `-`      | Subtraction    | `fsub`           | `5 - 2` | `3.0`  |
| `*`      | Multiplication | `fmul`           | `3 * 4` | `12.0` |
| `/`      | Division       | `fdiv`           | `8 / 2` | `4.0`  |

#### Comparison Operators

| Operator | Name      | LLVM Instruction | Example  | Result |
|----------|-----------|------------------|----------|--------|
| `<`      | Less Than | `fcmp ult`       | `1 < 2`  | `1.0`  |
|          |           |                  | `2 < 1`  | `0.0`  |

**Note:** The comparison uses unsigned comparison (`ult` = unordered less than), which handles NaN values specially.

---

## 7. Control Flow

AuroraLang provides rich control flow constructs including conditionals, loops, and loop control statements.

### 7.1 Conditional Statements

#### If Statement

```ebnf
if_statement ::= "if" expression block ("else" (if_statement | block))?
```

**Examples:**

```aurora
# Simple if
if x > 0 {
    printd(1);
}

# If-else
if x > 0 {
    printd(1);
} else {
    printd(0);
}

# If-else-if chain
if x < 0 {
    printd(-1);
} else if x > 0 {
    printd(1);
} else {
    printd(0);
}
```

### 7.2 Loop Statements

#### While Loop

```ebnf
while_statement ::= "while" expression block
```

Executes the block repeatedly as long as the condition is true.

**Example:**

```aurora
let i = 0;
while i < 10 {
    printd(i);
    i = i + 1;
}
```

#### For Loop (Range-based)

```ebnf
for_statement ::= "for" identifier "in" expression ".." expression block
```

Iterates over a range of values from start (inclusive) to end (exclusive).

**Examples:**

```aurora
# Print 0 to 9
for i in 0..10 {
    printd(i);
}

# Iterate with expressions
let n = 5;
for i in 1..n+1 {
    printd(i);
}

# Factorial using for loop
fn factorial(n) {
    let result = 1;
    for i in 1..n+1 {
        result = result * i;
    }
    return result;
}
```

**Properties:**
- Loop variable is scoped to the loop body
- Start value is evaluated once before loop
- End value is evaluated once before loop
- Loop variable is incremented by 1 each iteration
- Range is half-open: [start, end)

#### Loop (Infinite Loop)

```ebnf
loop_statement ::= "loop" block
```

Creates an infinite loop that must be exited with `break`.

**Example:**

```aurora
let count = 0;
loop {
    printd(count);
    count = count + 1;
    if count >= 10 {
        break;
    }
}
```

### 7.3 Loop Control Statements

#### Break Statement

```ebnf
break_statement ::= "break" ";"?
```

Immediately exits the innermost enclosing loop.

**Example:**

```aurora
for i in 0..100 {
    if i % 7 == 0 {
        printd(i);
        break;  # Exit after finding first multiple of 7
    }
}
```

**Rules:**
- Can only be used inside a loop (while, for, or loop)
- Exits to the statement immediately after the loop
- Compile error if used outside a loop

#### Continue Statement

```ebnf
continue_statement ::= "continue" ";"?
```

Skips the rest of the current iteration and continues with the next.

**Example:**

```aurora
for i in 0..10 {
    if i % 2 == 0 {
        continue;  # Skip even numbers
    }
    printd(i);  # Only prints odd numbers
}
```

**Rules:**
- Can only be used inside a loop
- In while/for loops: jumps to condition check
- In loop: jumps to the beginning of the loop body
- Compile error if used outside a loop

### 7.4 Nested Loops

Loops can be nested, and `break`/`continue` affect only the innermost loop.

**Example:**

```aurora
for i in 0..3 {
    for j in 0..3 {
        printd(i * 10 + j);
        if j == 1 {
            break;  # Only breaks inner loop
        }
    }
}
```

---

## 8. Functions

### 8.1 Function Definition

```ebnf
fn identifier(param1: type1, param2: type2, ...) -> return_type {
  statement1;
  statement2;
  ...
  return expression;
}
```

**Properties:**
- Functions are globally scoped
- Functions can return `int`, `double`, `bool`, `string`, or `void`
- Parameters can have type annotations (optional for backward compatibility)
- Parameters are immutable (cannot be reassigned)
- Recursive calls are supported
- If no return type is specified, `double` is assumed (legacy behavior)

### 8.2 Function Parameters

Parameters are local to the function body and shadow any outer declarations.

```aurora
fn example(x: int, y: int) -> int {
  # x and y are parameters
  return x + y;
}
```

**Type Annotations:**
```aurora
# With type annotations (recommended)
fn add(a: int, b: int) -> int {
  return a + b;
}

# Without type annotations (legacy, defaults to double)
fn add(a, b) {
  return a + b;
}
```

### 8.3 Return Statements

Functions can have explicit `return` statements:

```aurora
fn explicit() -> int {
  return 42;
}
```

**Return Type Rules:**

1. **Non-void functions must return a value:**
```aurora
fn get_value() -> int {
  return 42;  # OK
}

fn error_example() -> int {
  return;  # ERROR: Cannot use empty return in non-void function
}
```

2. **Void functions cannot return a value:**
```aurora
fn print_message() -> void {
  printd(1.0);  # OK
  return;       # OK: Empty return is allowed
}

fn error_void() -> void {
  return 42;    # ERROR: Cannot return a value from void function
}
```

### 8.4 Void Functions

Void functions are functions that do not return a value. They are typically used for side effects like printing or modifying state.

```aurora
fn print_hello() -> void {
  printd(1.0);
  printd(2.0);
  printd(3.0);
  # No return statement needed
}

fn conditional_print(x: int) -> void {
  if x > 0 {
    printd(100.0);
    return;  # Early exit is OK
  }
  printd(200.0);
}
```

**Implicit Return:**
- Void functions automatically return at the end of their body
- No explicit return statement is required
- Empty return statements (`return;`) can be used for early exits

### 8.5 Type Conversion in Returns

The compiler automatically converts between compatible types when returning:

```aurora
fn int_to_double(x: int) -> double {
  return x;  # int is automatically converted to double
}

fn double_to_int(x: double) -> int {
  return x;  # double is automatically converted to int (truncated)
}
```

### 8.6 Entry Point

Every AuroraLang program must define a `main` function, which serves as the entry point.

**Recommended: Void Return (Modern Style)**

```aurora
fn main() {
  # Program starts here
  printd(42)
  # Automatically exits with code 0
}
```

**With Exit Code (When Needed)**

```aurora
fn main() -> int {
  if error_condition {
    return 1  # Error exit code
  }
  return 0    # Success exit code
}
```

The `main` function:
- Takes no parameters (currently)
- **Recommended**: Use no return type (defaults to `void`), exits with code 0
- Can return `int` when you need to specify an exit code explicitly
- Can also return `double` for legacy compatibility
- Is automatically executed by the JIT engine

**Best Practices:**
- Use `void main()` for simple programs that always succeed
- Use `int main()` when you need error handling with different exit codes
- Exit code 0 indicates success, non-zero indicates error

---

## 9. Built-in Functions & Package System

### 9.1 Built-in Functions

AuroraLang provides essential built-in functions that are automatically available without any import or declaration.

#### `printd` - Print Double Value

```aurora
fn main() {
  printd(3.14)      # Prints: 3.14
  printd(42)        # Prints: 42
  printd(2 + 3)     # Prints: 5
}
```

**Signature**: `printd(x: double) -> double`

**Description**: Prints a numeric value to standard output followed by a newline.

**Note**: No `extern` declaration is needed. The function is automatically registered by the compiler.

### 9.2 Package System

AuroraLang supports a hierarchical package system similar to Java/Kotlin, allowing you to organize code in a structured namespace hierarchy.

#### Package Declaration

```ebnf
package_declaration ::= "package" package_name ";"?
package_name ::= identifier ("." identifier)*
```

**Syntax:**
```aurora
package com.example.myapp

// Package members (functions, classes, etc.)
fn myFunction() {
    // ...
}
```

**Rules:**
- Package declaration must appear at the **beginning of the file**, before any imports or code
- Package names follow reverse domain naming convention (e.g., `com.company.project`)
- Package names map to directory structure: `com.example.app` → `com/example/app/`
- Files in a package should be placed in the corresponding directory structure

#### Import System

AuroraLang supports multiple import styles for backward compatibility and flexibility.

##### Import Syntax

```ebnf
import_statement ::= "import" (string_literal | package_path) ";"?
package_path ::= identifier ("." identifier)*
```

##### Package-Style Imports (Recommended)

```aurora
package myapp

import com.example.math.Calculator
import com.example.utils.StringHelper

fn main() {
    // Use imported functions
    let result = square(5)
}
```

##### String-Based Imports (Legacy, Still Supported)

```aurora
import "mymodule"        # Imports mymodule.aur from current directory
import "lib/mathutils"   # Imports lib/mathutils.aur (relative path)
```

#### Package Structure Example

**Directory Structure:**
```
src/
  com/
    example/
      math/
        Calculator.aur
        Functions.aur
      utils/
        StringHelper.aur
  myapp/
    Main.aur
```

**File: `com/example/math/Functions.aur`**
```aurora
package com.example.math

fn square(x: int) -> int {
    return x * x
}

fn cube(x: int) -> int {
    return x * x * x
}

fn factorial(n: int) -> int {
    if n <= 1 {
        return 1
    }
    return n * factorial(n - 1)
}
```

**File: `myapp/Main.aur`**
```aurora
package myapp

import com.example.math.Functions

fn main() -> int {
    printd(square(5))      # Prints: 25
    printd(cube(3))        # Prints: 27
    printd(factorial(5))   # Prints: 120
    return 0
}
```

#### Module Resolution Rules

1. **Package-Style Imports**: `import com.example.MyClass`
   - Converts dots to slashes: `com.example.MyClass` → `com/example/MyClass.aur`
   - Searches in package search paths (default: `.`, `src`, `stdlib/aurora`)
   - Suitable for hierarchical package organization

2. **String Imports**: `import "path/to/module"`
   - Relative or absolute file paths
   - `.aur` extension is added automatically if not present
   - Searches relative to current file first

3. **Identifier Imports**: `import mymodule`
   - Searches for `mymodule.aur` in current directory

4. **Circular Import Protection**: Modules are loaded only once, preventing infinite loops

#### Package Search Paths

The compiler searches for packages in the following directories (in order):
1. Current directory (`.`)
2. Source directory (`src`)
3. Standard library (`stdlib/aurora`)

You can organize your project using standard package conventions:
```
myproject/
  src/
    com/
      mycompany/
        myapp/
          Main.aur
        utils/
          Helper.aur
```

### 9.3 External Declarations (Deprecated)

**Note**: The `extern` keyword is deprecated and kept only for backward compatibility. Built-in functions like `printd` are now automatically available.

For custom C/C++ interop (advanced use):

```aurora
extern my_c_function(x);  # Legacy syntax
```

To add custom external functions:

1. Define the function in C++ with `extern "C"` linkage
2. Link it during compilation
3. The JIT engine will resolve the symbol at runtime

**Modern Approach**: Use the module system to organize AuroraLang code, and rely on built-in functions for standard operations.

---

## 10. Program Structure

### 10.1 Compilation Units

Each `.aur` file is a complete compilation unit. There is currently no module or import system.

### 10.2 Declaration Order

Functions can be called before they are defined, as long as they are defined somewhere in the program:

```aurora
fn main() {
  printd(helper());  # OK: helper defined later
  return 0;
}

fn helper() {
  return 42;
}
```

External declarations should appear before use (best practice).

### 10.3 Name Resolution

- All functions are in a global namespace
- Function names must be unique
- Parameter names are local to their function
- No variable declarations (only parameters exist as names)

---

## 11. Code Generation

### 10.1 LLVM IR Generation

Each AST node generates corresponding LLVM Intermediate Representation:

| AST Node     | LLVM IR                                    |
|--------------|--------------------------------------------|
| `Number`     | `ConstantFP` (constant floating-point)     |
| `Variable`   | Function parameter lookup                   |
| `Binary +`   | `fadd` instruction                         |
| `Binary -`   | `fsub` instruction                         |
| `Binary *`   | `fmul` instruction                         |
| `Binary /`   | `fdiv` instruction                         |
| `Binary <`   | `fcmp ult` + `uitofp` conversion           |
| `Call`       | `call` instruction                         |
| `Function`   | Function definition with entry basic block |
| `Return`     | `ret` instruction                          |

### 10.2 Function Signatures

All functions have the signature:
```llvm
double @function_name(double, double, ...)
```

Example LLVM IR for `fn add(a, b) { return a + b; }`:

```llvm
define double @add(double %a, double %b) {
entry:
  %addtmp = fadd double %a, %b
  ret double %addtmp
}
```

### 10.3 Verification

After code generation, LLVM's `verifyFunction` ensures the generated IR is well-formed.

---

## 12. Runtime Behavior

### 11.1 JIT Compilation

AuroraLang uses LLVM's ORC (On-Request Compilation) JIT engine:

1. **Parse** source code into AST
2. **Generate** LLVM IR from AST
3. **Add** IR to JIT engine
4. **Lookup** `main` function address
5. **Execute** native compiled code

### 11.2 Execution Flow

```
Source (.aur) → Lexer → Parser → AST → CodeGen → LLVM IR → JIT → Native Code → Execute
```

### 11.3 Memory Management

- All values are doubles (stored in registers when possible)
- No heap allocation or garbage collection
- Function call frames are managed by LLVM/system stack

### 11.4 Error Handling

**Compile-time errors:**
- Lexical errors: Invalid tokens
- Syntax errors: Malformed expressions or statements
- Semantic errors: Undefined functions, argument count mismatches

**Runtime errors:**
- Division by zero (produces `inf` or `nan`)
- Stack overflow (from deep recursion)
- No exception handling mechanism currently

### 11.5 Symbol Resolution

The JIT engine resolves external symbols using `DynamicLibrarySearchGenerator`, which searches:
1. The current process (for `printd` and user-defined externals)
2. Dynamically linked libraries

---

## 13. Grammar Summary

Complete EBNF grammar for AuroraLang:

```ebnf
program ::= declaration*

declaration ::= function_definition
              | external_declaration

function_definition ::= "fn" identifier "(" parameter_list? ")" block

external_declaration ::= "extern" identifier "(" parameter_list? ")" ";"?

parameter_list ::= identifier ("," identifier)*

block ::= "{" statement* "}"

statement ::= expression ";"?
            | return_statement

return_statement ::= "return" expression ";"?

expression ::= comparison

comparison ::= additive (("<") additive)*

additive ::= multiplicative (("+" | "-") multiplicative)*

multiplicative ::= primary (("*" | "/") primary)*

primary ::= number
          | identifier
          | function_call
          | "(" expression ")"

function_call ::= identifier "(" argument_list? ")"

argument_list ::= expression ("," expression)*

identifier ::= letter (letter | digit | "_")*

number ::= digit+ ("." digit*)? | "." digit+

letter ::= "a".."z" | "A".."Z" | "_"

digit ::= "0".."9"
```

---

## 14. Examples

### Example 1: Basic Arithmetic

```aurora
fn main() {
  printd(2 + 3 * 4)     # 14
  printd((2 + 3) * 4)   # 20
}
```

### Example 2: Function Composition

```aurora
fn double(x: double) -> double {
  return x + x
}

fn triple(x: double) -> double {
  return x + x + x
}

fn six_times(x: double) -> double {
  return double(triple(x))
}

fn main() {
  printd(six_times(7))  # 42
}
```

### Example 3: Comparison

```aurora
fn min(a, b) {
  return a < b  # Returns 1.0 if a < b, else 0.0
}

fn main() {
  printd(min(5, 10))   # 1
  printd(min(10, 5))   # 0
  return 0
}
```

### Example 4: Multiple Statements

```aurora
fn compute() {
  printd(1)
  printd(2)
  printd(3)
  return 100
}

fn main() {
  printd(compute())  # Prints: 1, 2, 3, 100
  return 0
}
```

### Example 5: Using Modules

```aurora
# mathlib.aur
fn factorial(n: int) -> int {
    if n <= 1 {
        return 1
    }
    return n * factorial(n - 1)
}
```

```aurora
# main.aur
import "mathlib"

fn main() {
    printd(factorial(5))  # Prints: 120
    printd(factorial(10)) # Prints: 3628800
    return 0
}
```

---

## 15. Implementation Notes

### 14.1 Lexer Implementation

- Single-pass character stream processing
- Greedy tokenization (longest match)
- One-token lookahead for parser

### 14.2 Parser Implementation

- Recursive descent parser
- Operator precedence climbing for binary expressions
- Error recovery via token skipping

### 14.3 AST Design

- Expression-based (statements are special expressions)
- Smart pointers (`unique_ptr`) for memory safety
- Visitor pattern via virtual `codegen()` methods

### 14.4 Code Generation

- Direct AST-to-IR translation
- No intermediate representation
- Single-pass code generation

### 14.5 Optimizations

- LLVM optimization passes (optional)
- Constant folding by LLVM
- Register allocation by LLVM
- Inlining by LLVM (if enabled)

---

## 16. Future Extensions

### Planned Language Features

1. **Control Flow**
   ```aurora
   if (condition) { ... } else { ... }
   while (condition) { ... }
   for (i = 0; i < 10; i = i + 1) { ... }
   ```

2. **Variable Declarations**
   ```aurora
   var x = 42;
   x = x + 1;
   ```

3. **More Types**
   ```aurora
   int, bool, string, array
   ```

4. **More Operators**
   ```aurora
   >, >=, <=, ==, !=, &&, ||, !
   ```

5. **Arrays**
   ```aurora
   var arr = [1, 2, 3, 4, 5];
   printd(arr[2]);
   ```

6. **Structs**
   ```aurora
   struct Point { x, y }
   var p = Point { x: 10, y: 20 };
   ```

7. **Modules**
   ```aurora
   import math;
   printd(math.sqrt(16));
   ```

---

## Appendix A: Reserved Words

Current keywords:
- `fn`
- `extern`
- `return`

Reserved for future use:
- `if`, `else`, `while`, `for`, `break`, `continue`
- `var`, `let`, `const`
- `true`, `false`, `null`
- `struct`, `enum`, `type`
- `import`, `export`, `module`
- `pub`, `priv`

---

## Appendix B: Error Messages

Common error messages and their meanings:

| Error Message | Meaning |
|---------------|---------|
| `expected )` | Missing closing parenthesis |
| `expected }` | Missing closing brace |
| `expected , or ) in args` | Invalid function call syntax |
| `expected function name` | Missing identifier after `fn` keyword |
| `unknown variable: X` | Variable `X` not found in scope |
| `unknown function referenced: X` | Function `X` not declared |
| `arg count mismatch in call to X` | Wrong number of arguments |
| `function redefinition` | Function already defined |
| `No main() defined.` | Program missing entry point |

---

## Appendix C: LLVM IR Examples

### Example: Simple Addition

**Aurora:**
```aurora
fn add(a, b) {
  return a + b;
}
```

**LLVM IR:**
```llvm
define double @add(double %a, double %b) {
entry:
  %addtmp = fadd double %a, %b
  ret double %addtmp
}
```

### Example: Comparison

**Aurora:**
```aurora
fn less(a, b) {
  return a < b;
}
```

**LLVM IR:**
```llvm
define double @less(double %a, double %b) {
entry:
  %cmptmp = fcmp ult double %a, %b
  %booltmp = uitofp i1 %cmptmp to double
  ret double %booltmp
}
```

---

**Happy coding!** 🌅
