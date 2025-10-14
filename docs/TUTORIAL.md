# AuroraLang Tutorial

Welcome! This tutorial will teach you AuroraLang from the ground up.

---

## Chapter 1: Your First Program

### Hello, World!

```aurora
extern printd(x);

fn main() {
  printd(42);
  return 0;
}
```

Let's break this down:

- `extern printd(x);` - Declares an external function that prints numbers
- `fn main()` - Every Aurora program must have a `main` function
- `printd(42);` - Calls the print function with the number 42
- `return 0;` - Returns 0 to indicate success

**Run it:**
```bash
./aurora hello.aur
# Output: 42
```

---

## Chapter 2: Functions

Functions are the building blocks of Aurora programs.

### Defining Functions

```aurora
fn add(a, b) {
  return a + b;
}
```

- `fn` - Function keyword
- `add` - Function name
- `(a, b)` - Parameters (inputs)
- `return a + b;` - Return statement (output)

### Calling Functions

```aurora
extern printd(x);

fn add(a, b) {
  return a + b;
}

fn main() {
  printd(add(10, 5));  # Prints 15
  return 0;
}
```

### Multiple Parameters

```aurora
fn calculate(a, b, c) {
  return a + b * c;
}
```

### No Parameters

```aurora
fn get_answer() {
  return 42;
}
```

---

## Chapter 3: Arithmetic

Aurora supports standard arithmetic operations.

### Basic Operations

```aurora
extern printd(x);

fn main() {
  printd(2 + 3);      # Addition: 5
  printd(10 - 4);     # Subtraction: 6
  printd(3 * 7);      # Multiplication: 21
  printd(20 / 5);     # Division: 4
  return 0;
}
```

### Operator Precedence

```aurora
printd(2 + 3 * 4);     # 14 (not 20) - multiplication first
printd((2 + 3) * 4);   # 20 - parentheses override
```

**Precedence order (highest to lowest):**
1. Parentheses `()`
2. Multiplication `*` and Division `/`
3. Addition `+` and Subtraction `-`

### Floating Point

All numbers in Aurora are double-precision floats:

```aurora
printd(10 / 3);        # 3.33333...
printd(0.5 + 0.5);     # 1
printd(3.14159);       # 3.14159
```

---

## Chapter 4: Expressions

Everything that produces a value is an expression.

### Nested Expressions

```aurora
fn complex(x) {
  return (x + 5) * (x - 3) / 2;
}
```

### Function Calls as Expressions

```aurora
fn double(x) {
  return x + x;
}

fn quad(x) {
  return double(double(x));
}
```

### Expression Statements

```aurora
fn main() {
  printd(100);           # Expression statement
  42;                    # Valid but result is discarded
  1 + 1;                 # Valid but useless
  return 0;
}
```

---

## Chapter 5: Comparison

Aurora supports comparison operations.

### Less Than Operator

```aurora
extern printd(x);

fn main() {
  printd(1 < 2);         # 1.0 (true)
  printd(5 < 3);         # 0.0 (false)
  printd(10 < 10);       # 0.0 (false - not less than)
  return 0;
}
```

**Note:** Comparisons return:
- `1.0` for true
- `0.0` for false

### Using Comparisons

```aurora
fn is_positive(n) {
  return 0 < n;          # Returns 1.0 if n > 0
}

fn is_negative(n) {
  return n < 0;          # Returns 1.0 if n < 0
}
```

---

## Chapter 6: Comments

Use `#` for single-line comments:

```aurora
# This is a comment

fn add(a, b) {
  # This function adds two numbers
  return a + b;  # Return the sum
}

# Multi-line comments not yet supported
# But you can use multiple single-line comments
# Like this!
```

---

## Chapter 7: External Functions

Integrate with C/C++ libraries using `extern`.

### Declaring Externals

```aurora
extern printd(x);           # Print a number
extern sin(angle);          # Sine function (if available)
extern sqrt(x);             # Square root (if available)
```

### Using External Functions

```aurora
extern printd(x);
extern sin(angle);

fn main() {
  printd(sin(3.14159));     # ~0
  return 0;
}
```

---

## Chapter 8: Program Structure

### Complete Program Template

```aurora
# External declarations at top
extern printd(x);

# Helper functions
fn helper1(x) {
  return x * 2;
}

fn helper2(x) {
  return helper1(x) + 1;
}

# Main function (required)
fn main() {
  printd(helper2(10));
  return 0;
}
```

### Declaration Order

Functions can be called before they're defined:

```aurora
fn main() {
  printd(later());       # OK - forward reference
  return 0;
}

fn later() {
  return 42;
}
```

---

## Chapter 9: Common Patterns

### Calculator Functions

```aurora
extern printd(x);

fn add(a, b) { return a + b; }
fn sub(a, b) { return a - b; }
fn mul(a, b) { return a * b; }
fn div(a, b) { return a / b; }

fn main() {
  printd(add(10, 5));    # 15
  printd(sub(10, 5));    # 5
  printd(mul(10, 5));    # 50
  printd(div(10, 5));    # 2
  return 0;
}
```

### Mathematical Functions

```aurora
fn square(x) {
  return x * x;
}

fn cube(x) {
  return x * x * x;
}

fn average(a, b) {
  return (a + b) / 2;
}
```

### Composed Functions

```aurora
fn f(x) {
  return x + 1;
}

fn g(x) {
  return x * 2;
}

fn compose(x) {
  return f(g(x));        # g then f
}
```

---

## Chapter 10: Best Practices

### Naming Conventions

```aurora
# Good: descriptive names
fn calculate_area(width, height) {
  return width * height;
}

# Avoid: single letters (except for math)
fn f(x, y) {
  return x * y;
}
```

### Function Size

Keep functions small and focused:

```aurora
# Good: one responsibility
fn double(x) {
  return x * 2;
}

fn triple(x) {
  return x * 3;
}

fn six_times(x) {
  return double(triple(x));
}
```

### Comments

Comment why, not what:

```aurora
# Good: explains intent
fn discount(price) {
  return price * 0.9;    # 10% discount for members
}

# Unnecessary: obvious from code
fn add(a, b) {
  return a + b;          # Add a and b
}
```

---

## Chapter 11: Debugging

### Print Debugging

```aurora
extern printd(x);

fn debug_function(x) {
  printd(x);             # Print input
  return x * 2;
}

fn main() {
  printd(debug_function(5));
  return 0;
}
```

### Intermediate Results

```aurora
fn complex_calc(x) {
  printd(x);             # Step 1
  printd(x + 5);         # Step 2
  printd((x + 5) * 2);   # Step 3
  return (x + 5) * 2;
}
```

---

## Chapter 12: Common Errors

### Missing Main

```aurora
fn helper() {
  return 42;
}
# Error: No main() defined
```

**Fix:** Always include `main()`.

### Wrong Argument Count

```aurora
fn add(a, b) {
  return a + b;
}

fn main() {
  printd(add(1));        # Error: expected 2 args, got 1
  return 0;
}
```

**Fix:** Match parameter count in call.

### Undefined Function

```aurora
fn main() {
  printd(undefined());   # Error: unknown function
  return 0;
}
```

**Fix:** Define the function before calling.

---

## Next Steps

You've learned the basics of AuroraLang! Here's what to explore next:

1. **[Examples](../examples/)** - See real programs
2. **[Specification](SPECIFICATION.md)** - Deep dive into language details
3. **[Architecture](ARCHITECTURE.md)** - Understand how it works
4. **[Contributing](CONTRIBUTING.md)** - Help build Aurora

---

**Happy coding with Aurora! 🌅**
