# Getting Started with AuroraLang

Welcome to AuroraLang! This guide will get you up and running in 10 minutes.

---

## What is AuroraLang?

AuroraLang is a modern, statically-typed systems programming language built on LLVM. It combines:
- **Clean Syntax** - Write like Kotlin/Swift
- **Native Performance** - Run like C++
- **Memory Safety** - Null safety, bounds checking
- **Fast Compilation** - Sub-second iteration cycles

---

## Installation

### macOS

```bash
# Install LLVM via Homebrew
brew install llvm cmake

# Clone repository
git clone https://github.com/flyfishxu/AuroraLang.git
cd AuroraLang

# Build
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH="$(brew --prefix llvm)"
make

# Verify
./aurora --help
```

### Linux (Ubuntu/Debian)

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install llvm-dev cmake build-essential

# Clone and build
git clone https://github.com/flyfishxu/AuroraLang.git
cd AuroraLang
mkdir build && cd build
cmake ..
make

# Verify
./aurora --help
```

---

## Your First Program

### Hello, World!

Create a file called `hello.aur`:

```aurora
extern printd(x)

fn main() {
    printd(42)
    return 0
}
```

Run it:

```bash
./aurora hello.aur
```

Output:
```
42
```

**Explanation:**
- `extern printd(x)` - Declares an external C function for printing
- `fn main()` - Every program must have a main function
- `printd(42)` - Prints the number 42
- `return 0` - Returns exit code 0 (success)

---

## Language Basics

### Functions

Functions are the building blocks of AuroraLang programs.

```aurora
extern printd(x)

# Define a function
fn add(a, b) {
    return a + b
}

# Call it
fn main() {
    printd(add(10, 5))  # Prints: 15
    return 0
}
```

### Variables

```aurora
fn demo() {
    # Immutable variable (preferred)
    let x = 10
    
    # Mutable variable
    var y = 20
    y = 30  # OK
    
    # x = 15  # ERROR: x is immutable
}
```

### Types and Type Annotations

```aurora
# Type inference (compiler figures out the type)
let x = 42              # int
let y = 3.14            # double
let flag = true         # bool

# Explicit type annotations
let name: string = "Aurora"
let count: int = 100

# Optional types (can be null)
let value: int? = null
```

### Control Flow

#### If-Else

```aurora
fn max(a, b) {
    if a > b {
        return a
    } else {
        return b
    }
}
```

#### While Loops

```aurora
fn countdown(n) {
    var i = n
    while i > 0 {
        printd(i)
        i = i - 1
    }
}
```

#### For Loops

```aurora
# Range-based for loop
fn sum_to_n(n) {
    var total = 0
    for i in 0..n {  # [0, n) - excludes n
        total = total + i
    }
    return total
}
```

#### Loop Control

```aurora
fn find_first_multiple(target) {
    for i in 1..1000 {
        if i % target == 0 {
            break  # Exit loop
        }
    }
}

fn skip_evens() {
    for i in 0..10 {
        if i % 2 == 0 {
            continue  # Skip to next iteration
        }
        printd(i)  # Only prints odd numbers
    }
}
```

### Operators

```aurora
# Arithmetic
2 + 3        # 5
10 - 4       # 6
3 * 7        # 21
20 / 5       # 4
10 % 3       # 1 (modulo)

# Comparison
1 < 2        # true
5 > 3        # true
x <= y       # less than or equal
a >= b       # greater than or equal
x == y       # equal
x != y       # not equal

# Logical
a && b       # logical AND
a || b       # logical OR
!flag        # logical NOT

# Bitwise
a & b        # bitwise AND
a | b        # bitwise OR
a ^ b        # bitwise XOR
~a           # bitwise NOT
a << 2       # left shift
a >> 2       # right shift

# Ternary
x > 0 ? 1 : -1    # conditional expression
```

### Arrays

```aurora
fn array_demo() {
    # Create array
    let numbers = [1, 2, 3, 4, 5]
    
    # Access elements
    let first = numbers[0]
    let second = numbers[1]
    
    # Loop through array
    for i in 0..5 {
        printd(numbers[i])
    }
}
```

### Comments

```aurora
# This is a single-line comment

fn add(a, b) {
    # Comments can be anywhere
    return a + b  # Even at end of lines
}
```

---

## Object-Oriented Programming

### Classes

```aurora
class Point {
    let x: double
    let y: double
    
    constructor(x_val: double, y_val: double) {
        this.x = x_val
        this.y = y_val
    }
    
    fn distance() -> double {
        return (this.x * this.x + this.y * this.y) ** 0.5
    }
}

fn main() {
    let p = Point(3.0, 4.0)
    printd(p.distance())  # Prints: 5.0
    return 0
}
```

---

## Common Patterns

### Factorial

```aurora
extern printd(x)

fn factorial(n) {
    if n <= 1 {
        return 1
    }
    return n * factorial(n - 1)
}

fn main() {
    printd(factorial(5))  # 120
    return 0
}
```

### Fibonacci

```aurora
extern printd(x)

fn fibonacci(n) {
    if n < 2 {
        return n
    }
    
    var a = 0
    var b = 1
    for i in 2..n+1 {
        let temp = a + b
        a = b
        b = temp
    }
    return b
}

fn main() {
    printd(fibonacci(10))  # 55
    return 0
}
```

### Prime Numbers

```aurora
extern printd(x)

fn is_prime(n) {
    if n < 2 {
        return false
    }
    
    var i = 2
    while i * i <= n {
        if n % i == 0 {
            return false
        }
        i = i + 1
    }
    return true
}

fn main() {
    # Print first 10 primes
    var count = 0
    var num = 2
    while count < 10 {
        if is_prime(num) {
            printd(num)
            count = count + 1
        }
        num = num + 1
    }
    return 0
}
```

---

## Development Tools

### Running Programs

```bash
# Run program (quiet mode)
aurora program.aur

# Show compilation info
aurora --log-level info program.aur

# Debug mode (detailed output + timing)
aurora --debug program.aur

# Maximum verbosity
aurora --trace program.aur
```

### Log Levels

| Level   | Output                                    | Use Case           |
|---------|-------------------------------------------|--------------------|
| `off`   | No output (default)                       | Normal execution   |
| `error` | Only errors                               | CI/CD              |
| `warn`  | Warnings + errors                         | Development        |
| `info`  | Compilation phases                        | Understanding flow |
| `debug` | Detailed info + timing                    | Debugging          |
| `trace` | Everything (AST, IR, tokens)              | Deep debugging     |

### Running Test Suite

```bash
# Run all tests
cd AuroraLang
./run_tests.sh

# Run specific test
./build/aurora tests/basic/arithmetic_operators.aur
```

---

## Next Steps

Now that you know the basics, explore:

1. **[Tutorial](TUTORIAL.md)** - In-depth language tutorial
2. **[Language Specification](LANGUAGE_SPEC.md)** - Complete language reference
3. **[OOP Features](OOP_FEATURES.md)** - Object-oriented programming guide
4. **[Examples](EXAMPLES.md)** - More example programs
5. **[Developer Guide](DEVELOPER_GUIDE.md)** - Contributing and debugging

---

## Getting Help

- **Examples**: Check `tests/` directory for working code
- **Issues**: [GitHub Issues](https://github.com/flyfishxu/AuroraLang/issues)
- **Discussions**: [GitHub Discussions](https://github.com/flyfishxu/AuroraLang/discussions)

---

**Happy coding! 🌅**

