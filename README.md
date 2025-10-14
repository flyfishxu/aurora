# AuroraLang

<p align="center">
  <img src="images/icon.svg" alt="AuroraLang Logo" width="120" />
</p>

A Modern, High-Performance Programming Language Built on LLVM

AuroraLang is a statically-typed systems programming language that combines the simplicity of modern languages (Kotlin, Swift) with the raw performance of LLVM-compiled native code. Write clean, intuitive code that runs at C++ speeds.

---

## 🎯 Why AuroraLang?

**Our goal is: "Write like Kotlin, Run like C++"**

✨ **Modern, Clean Syntax** - No semicolons, intuitive keywords, readable code  
⚡ **Native Performance** - LLVM-optimized machine code, zero-cost abstractions  
🛡️ **Memory Safety** - Null safety, optional types, bounds checking  
🚀 **Fast Compilation** - LLVM JIT for instant feedback, ahead-of-time for production  
🔧 **Simple Yet Powerful** - Easy to learn, powerful enough for systems programming  
🌐 **True Cross-Platform** - Linux, macOS, Windows - compile once, run anywhere

---

## 📦 Installation

### Prerequisites

- **LLVM 14+** (tested with LLVM 21)
- **CMake 3.20+**
- **C++17 compatible compiler**

### macOS (Homebrew)

```bash
# Install LLVM
brew install llvm cmake

# Clone repository
git clone https://github.com/flyfishxu/AuroraLang.git
cd AuroraLang

# Build
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH="$(brew --prefix llvm)"
make
```

### Linux

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install llvm-dev cmake build-essential

# Clone and build
git clone https://github.com/flyfishxu/AuroraLang.git
cd AuroraLang
mkdir build && cd build
cmake ..
make
```

---

## 🎯 Quick Start

### Hello, World!

```aurora
fn main() -> int {
    printd(42)  # Built-in function, no import needed!
    return 0
}
```

### Variables and Types

```aurora
fn demo() -> void {
    # Type inference
    let x = 42          # int
    let y = 3.14        # double
    let flag = true     # bool
    
    # Explicit types
    let name: string = "Aurora"
    let value: int? = null  # Optional type
    
    # Mutable variables
    var counter = 0
    counter = counter + 1
}
```

### Control Flow

```aurora
fn fibonacci(n: int) -> int {
    if n <= 1 {
        return n
    }
    
    var a = 0
    var b = 1
    
    for i in 2..n {
        let temp = a + b
        a = b
        b = temp
    }
    
    return b
}
```

---

## 🌟 Vision

AuroraLang is in **active development** with a clear mission: **build the language that should exist** - combining modern syntax with native performance.

### Core Principles

- **Simplicity + Power** - Clean syntax with zero-cost abstractions (write like Kotlin, run like C++)
- **Safety Without Overhead** - Compile-time guarantees, no GC pauses
- **Developer Experience** - Fast compilation, clear errors, intuitive tooling

### Designed For

- **High-performance systems** - Backends, microservices, game engines
- **Cross-platform development** - No VM, no runtime dependencies
- **Production workloads** - Real-world use from day one, not a toy language

---

## 📚 Documentation

### Getting Started
- **[Getting Started](docs/GETTING_STARTED.md)** - Quick start guide for new users (5-10 minutes)
- **[Tutorial](docs/TUTORIAL.md)** - Step-by-step learning guide
- **[Examples](docs/EXAMPLES.md)** - Real-world code examples

### Language Reference
- **[Language Specification](docs/LANGUAGE_SPEC.md)** - Complete language reference
- **[OOP Features](docs/OOP_FEATURES.md)** - Object-oriented programming guide

### Development
- **[Architecture](docs/ARCHITECTURE.md)** - Compiler internals and design
- **[Developer Guide](docs/DEVELOPER_GUIDE.md)** - Contributing, debugging, and development workflow
- **[Development Roadmap](docs/ROADMAP.md)** - Current status and future plans

### Project Info
- **[Why AuroraLang?](docs/WHY_AURORALANG.md)** - Understanding our value proposition

---

## 🤝 Contributing

We welcome contributions! AuroraLang is actively seeking:

- Core language features (see roadmap)
- Standard library implementations
- Documentation and examples
- Bug reports and fixes
- Performance optimizations

See [Developer Guide](docs/DEVELOPER_GUIDE.md) for contribution guidelines and [ROADMAP](docs/ROADMAP.md) for priority areas.

---

## 📄 License

MIT License - See [LICENSE](LICENSE) for details.

---

**Built with ❤️ and LLVM** | [GitHub](https://github.com/flyfishxu/AuroraLang) | [Documentation](docs/)
