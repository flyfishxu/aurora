# AuroraLang Examples

This document explains all example programs included with AuroraLang.

---

## Running Examples

All examples are located in the `examples/` directory. To run an example:

```bash
cd build
./aurora ../examples/filename.aur
```

---

## Example Programs

### 1. Simple Test (`simple_test.aur`)

**Purpose**: Minimal test to verify compiler works

**Features Demonstrated**:
- Function definition
- Function calls
- Arithmetic operations
- Operator precedence

**Code**:
```aurora
extern printd(x);

fn twice(x) {
  return x + x;
}

fn main() {
  printd(5);
  printd(twice(10));
  printd(3 + 4 * 2);
  return 0;
}
```

**Expected Output**:
```
5
20
11

[Process exited with code 0]
```

**Learning Points**:
- Functions can call other functions
- Operator precedence: `*` before `+`
- `main()` is the program entry point

---

### 2. Basic Arithmetic (`basic_arithmetic.aur`)

**Purpose**: Comprehensive demonstration of arithmetic operations

**Features Demonstrated**:
- All arithmetic operators
- Multiple function definitions
- Comparison operations
- Floating-point math
- Complex expressions
- Function composition

**Code** (excerpt):
```aurora
extern printd(x);

fn add(a, b) {
  return a + b;
}

fn subtract(a, b) {
  return a - b;
}

fn multiply(a, b) {
  return a * b;
}

fn divide(a, b) {
  return a / b;
}

fn is_less(a, b) {
  return a < b;
}

fn main() {
  printd(add(10, 5));           # 15
  printd(subtract(10, 5));      # 5
  printd(multiply(10, 5));      # 50
  printd(divide(10, 5));        # 2
  printd(is_less(5, 10));       # 1 (true)
  printd(is_less(10, 5));       # 0 (false)
  return 0;
}
```

**Expected Output**:
```
15
5
50
2
1
0
0
14
20
19.3333
6
3.14286

[Process exited with code 0]
```

**Learning Points**:
- All basic operators work correctly
- Comparison operators return 0 or 1
- Floating-point division is precise
- Functions can be composed

---

### 3. Type System (`type_system.aur`)

**Purpose**: Demonstrate type annotations and null safety

**Features Demonstrated**:
- Type annotations on parameters
- Type annotations on return values
- Optional types (`?`)
- Null values
- Boolean operations
- String types

**Code**:
```aurora
# Function with explicit type annotations
fn add(x: int, y: int) -> int {
    return x + y;
}

# Function returning optional type (nullable)
fn safe_divide(a: double, b: double) -> double? {
    if b == 0.0 {
        return null;
    }
    return a / b;
}

# Boolean operations
fn is_positive(n: int) -> bool {
    return n > 0;
}

# String operations
fn greet(name: string?) -> string {
    if name == null {
        return "Hello, Guest!";
    }
    return "Hello, " + name + "!";
}

fn main() -> int {
    let x: int = 42;
    let y: double = 3.14;
    let flag: bool = true;
    let msg: string = "Aurora";
    
    let optional_num: int? = null;
    let optional_str: string? = "Hello";
    
    return 0;
}
```

**Note**: This example demonstrates the syntax but some features (like full null checking) are still being implemented.

**Learning Points**:
- Type annotations are optional but recommended
- `?` suffix creates optional (nullable) types
- Non-nullable types cannot hold null
- Types improve code clarity and safety

---

### 4. Exit Code Test (`exit_code_test.aur`)

**Purpose**: Demonstrate non-zero exit codes

**Features Demonstrated**:
- Return values from main
- Exit code propagation

**Code**:
```aurora
extern printd(x);

fn main() {
  printd(42);
  return 1;  # Exit with error code
}
```

**Expected Output**:
```
42

[Process exited with code 1]
```

**Learning Points**:
- `return 0` indicates success
- Non-zero return values indicate errors
- Exit codes are visible in the output

---

## Creating Your Own Examples

### Template Structure

```aurora
# Comments explaining what the program does

# External function declarations
extern printd(x);

# Helper functions (if needed)
fn helper(param) {
  return value;
}

# Main entry point
fn main() {
  # Your code here
  return 0;
}
```

### Best Practices

1. **Start with `extern printd(x)`**: Most programs need to output results
2. **Always have `fn main()`**: Required entry point
3. **Return 0 on success**: Standard convention
4. **Use comments**: Explain non-obvious logic
5. **Keep it simple**: One concept per example

---

## Example Ideas

Here are some ideas for programs you can write:

### Factorial
```aurora
extern printd(x);

fn factorial(n) {
  if n <= 1 {
    return 1;
  }
  return n * factorial(n - 1);
}

fn main() {
  printd(factorial(5));  # 120
  return 0;
}
```

### Fibonacci
```aurora
extern printd(x);

fn fib(n) {
  if n < 2 {
    return n;
  }
  return fib(n - 1) + fib(n - 2);
}

fn main() {
  printd(fib(10));  # 55
  return 0;
}
```

### Sum to N
```aurora
extern printd(x);

fn sum_to(n) {
  let total = 0;
  let i = 1;
  while i <= n {
    total = total + i;
    i = i + 1;
  }
  return total;
}

fn main() {
  printd(sum_to(100));  # 5050
  return 0;
}
```

### GCD (Greatest Common Divisor)
```aurora
extern printd(x);

fn gcd(a, b) {
  while b != 0 {
    let temp = b;
    b = a % b;
    a = temp;
  }
  return a;
}

fn main() {
  printd(gcd(48, 18));  # 6
  return 0;
}
```

### Power Function
```aurora
extern printd(x);

fn power(base, exp) {
  let result = 1;
  let i = 0;
  while i < exp {
    result = result * base;
    i = i + 1;
  }
  return result;
}

fn main() {
  printd(power(2, 10));  # 1024
  return 0;
}
```

---

## Debugging Examples

### View Tokens

```bash
./aurora --lex ../examples/simple_test.aur
```

Shows all tokens the lexer produces.

### View LLVM IR

```bash
./aurora --emit-llvm ../examples/simple_test.aur
cat output.ll
```

Shows the generated LLVM IR code.

### Check Exit Code

```bash
./aurora ../examples/exit_code_test.aur
echo $?  # Print exit code
```

---

## Contributing Examples

Want to add an example? Great!

1. **Create the file**: `examples/your_example.aur`
2. **Test it works**: `./aurora examples/your_example.aur`
3. **Add documentation**: Comment your code
4. **Submit PR**: Include a description of what it demonstrates

Examples should:
- Be under 50 lines
- Demonstrate one concept clearly
- Include comments
- Use descriptive function names
- Work correctly

---

## Common Patterns

### Input Validation
```aurora
fn validate(x) {
  if x < 0 {
    return 0;
  }
  if x > 100 {
    return 100;
  }
  return x;
}
```

### Range Sum
```aurora
fn sum_range(start, end) {
  let total = 0;
  let i = start;
  while i <= end {
    total = total + i;
    i = i + 1;
  }
  return total;
}
```

### Count Iterations
```aurora
fn count_until(limit) {
  let count = 0;
  let value = 1;
  while value < limit {
    count = count + 1;
    value = value * 2;
  }
  return count;
}
```

---

## Next Steps

1. **Run all examples**: Get familiar with the syntax
2. **Modify examples**: Change values and see what happens
3. **Write your own**: Start with simple calculations
4. **Share**: Contribute interesting examples back

---

**Happy coding!** 🌅