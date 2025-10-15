# AuroraLang Object-Oriented Programming Features

This document describes the Object-Oriented Programming (OOP) features in AuroraLang.

---

## Table of Contents

1. [Overview](#overview)
2. [Class Declarations](#class-declarations)
3. [Fields](#fields)
4. [Methods](#methods)
5. [Constructors](#constructors)
6. [Object Creation](#object-creation)
7. [Member Access](#member-access)
8. [The this Keyword](#the-this-keyword)
9. [Access Modifiers](#access-modifiers)
10. [Examples](#examples)

---

## Overview

AuroraLang now supports object-oriented programming, including classes, fields, methods, constructors, and object instantiation. OOP features are implemented using LLVM structs, providing efficient performance.

### Key Features

- **Classes**: Define custom types containing data and behavior
- **Fields**: Member variables that store object state
- **Methods**: Member functions that define object behavior
- **Constructors**: Special methods for initializing objects
- **Member Access**: Use dot notation to access fields and call methods
- **this**: Reference to the current object instance

---

## Class Declarations

### Syntax

```ebnf
class_declaration ::= "class" identifier [primary_constructor_params] "{" class_member* "}"

primary_constructor_params ::= "(" [param ("," param)*] ")"
param ::= ["priv"] ("let" | "var") identifier ":" type

class_member ::= field_declaration
               | method_declaration
               | constructor_declaration
```

### Example: Traditional Syntax

```aurora
class Point {
    let x: double
    let y: double
    
    constructor(x_val: double, y_val: double) {
        this.x = x_val
        this.y = y_val
    }
    
    fn getX() -> double {
        return this.x
    }
}
```

### Example: Primary Constructor (Recommended)

```aurora
class Point(let x: double, let y: double) {
    fn getX() -> double {
        return this.x
    }
}
```

The primary constructor syntax is more concise and eliminates repetitive field initialization code.

---

## Fields

Fields are member variables of a class that store the object's state.

### Syntax

```ebnf
field_declaration ::= ["priv"] ("let" | "var") identifier ":" type ["=" expression] ";"?
```

**Note**: The `pub` modifier is redundant and should be omitted (members are public by default).

### Features

- **Immutable Fields**: Declared with `let`
- **Mutable Fields**: Declared with `var`
- **Type Annotations**: Field type must be specified
- **Default Values**: Optional initialization expression

### Example

```aurora
class Example {
    let immutableField: int      # Immutable field
    var mutableField: double      # Mutable field
    let withDefault: int = 42     # Field with default value
}
```

---

## Methods

Methods are member functions of a class that define the object's behavior.

### Syntax

```ebnf
method_declaration ::= ["priv"] ["static"] "fn" identifier "(" parameter_list? ")" ["->" type] block
```

**Note**: The `pub` modifier is redundant and should be omitted (methods are public by default).

### Features

- **Instance Methods**: Can access `this` and object state
- **Parameters**: Same parameter syntax as regular functions
- **Return Type**: Optional return type annotation
- **Method Calls**: First parameter automatically receives `this` pointer

### Example

```aurora
class Counter {
    let value: int
    
    fn increment() {
        this.value = this.value + 1
    }
    
    fn getValue() -> int {
        return this.value
    }
    
    fn add(amount: int) {
        this.value = this.value + amount
    }
}
```

---

## Constructors

Constructors are special methods used to initialize newly created objects.

### Syntax

```ebnf
constructor_declaration ::= ["priv"] "constructor" "(" parameter_list? ")" block
primary_constructor ::= "class" identifier "(" [["priv"] ("let" | "var") identifier ":" type] ("," ["priv"] ("let" | "var") identifier ":" type)* ")" "{" class_member* "}"
```

**Note**: The `pub` modifier is redundant for constructors (they are public by default).

### Features

- **Name**: Always uses the `constructor` keyword
- **Return Type**: Implicitly `void`, does not return a value
- **Invocation**: Automatically called through object creation
- **Initialization**: Should initialize all fields
- **Overloading**: Multiple constructors with different parameter types are supported

### Example: Basic Constructor

```aurora
class Rectangle {
    let width: double
    let height: double
    
    constructor(w: double, h: double) {
        this.width = w
        this.height = h
    }
}
```

### Example: Multiple Constructors (Overloading)

```aurora
class Point {
    let x: double
    let y: double
    
    // Constructor with two parameters
    constructor(x: double, y: double) {
        this.x = x
        this.y = y
    }
    
    // Constructor with one parameter (creates point on line y=x)
    constructor(value: double) {
        this.x = value
        this.y = value
    }
}

fn main() -> int {
    let p1 = Point(3.0, 4.0)  // Calls first constructor
    let p2 = Point(5.0)        // Calls second constructor
    return 0
}
```

### Example: Primary Constructor (Kotlin/Swift Style)

Primary constructors allow you to define fields directly in the class header, eliminating boilerplate code.

```aurora
// Concise syntax: fields automatically created from constructor parameters
class Point(let x: double, let y: double) {
    fn getDistance() -> double {
        return this.x * this.x + this.y * this.y
    }
}

// With visibility modifiers and mutable fields
// Note: 'pub' is unnecessary as members are public by default
class Person(let name: string, priv var age: int) {
    fn birthday() {
        this.age = this.age + 1  # Can modify private field
    }
}
```

### Primary Constructor Features

1. **Automatic Field Creation**: Parameters become class fields
2. **Type Annotations**: Required for all parameters
3. **Mutability**: Use `let` for immutable, `var` for mutable fields
4. **Visibility**: Use `priv` for private fields (public by default)
5. **Additional Constructors**: Can define additional constructors alongside primary constructor

---

## Object Creation

Create instances of a class by calling the class name as a function, similar to Kotlin and Swift.

### Syntax

```ebnf
object_creation ::= identifier "(" argument_list? ")"
```

### Behavior

1. Allocate memory (using `malloc`)
2. Initialize all fields to default values or specified values
3. Call the constructor (if it exists)
4. Return a pointer to the new object

### Example

```aurora
# Create objects directly by calling the class name
let p = Point(3.0, 4.0)
let rect = Rectangle(10.0, 5.0)
let counter = Counter(0)
```

**Note**: AuroraLang does not require the `new` keyword, following modern language design like Kotlin and Swift.

---

## Member Access

Use dot notation (`.`) to access an object's fields and methods.

### Syntax

```ebnf
member_access ::= expression "." identifier
method_call ::= expression "." identifier "(" argument_list? ")"
```

### Example

```aurora
let p = Point(3.0, 4.0)

# Field access
let x = p.x

# Method call
let xValue = p.getX()
p.setX(5.0)

# Method chaining
p.setX(10.0).setY(20.0)  # If method returns this
```

---

## The this Keyword

`this` is a special keyword that references the current object instance within methods.

### Uses

- Access fields of the current object
- Call other methods of the current object
- Distinguish between parameters and fields (when names are the same)

### Example

```aurora
class Person {
    let name: string
    let age: int
    
    constructor(name: string, age: int) {
        # Use this to distinguish parameters from fields
        this.name = name
        this.age = age
    }
    
    fn getName() -> string {
        return this.name
    }
    
    fn isAdult() -> bool {
        return this.age >= 18
    }
    
    fn birthday() {
        this.age = this.age + 1
    }
}
```

---

## Access Modifiers

AuroraLang supports access modifiers to control member visibility.

### Keywords

- **`pub`**: Public member (unnecessary - members are public by default)
- **`priv`**: Private member

### Default Visibility

**All class members (fields, methods, constructors) are public by default.** You only need to specify `priv` for private members. Using `pub` is unnecessary and redundant.

### Example

```aurora
class BankAccount {
    priv let balance: double      # Private field
    let accountNumber: int        # Public field (default)
    
    constructor(accNum: int) {    # Public constructor (default)
        this.accountNumber = accNum
        this.balance = 0.0
    }
    
    fn deposit(amount: double) {  # Public method (default)
        this.balance = this.balance + amount
    }
    
    priv fn validateAmount(amount: double) -> bool {
        return amount > 0.0       # Private method
    }
}
```

### Best Practices

1. **Omit `pub` modifier**: Since members are public by default, omit the `pub` keyword
2. **Explicitly mark private members**: Always use `priv` for private members to make privacy intent clear
3. **Use IDE hints**: Modern IDEs will warn about unnecessary `pub` modifiers

---

## Examples

### Complete Example: Bank Account System

```aurora
extern printd(x)

# Using primary constructor for cleaner code
class BankAccount(let accountNumber: int, var balance: double) {
    fn deposit(amount: double) {
        this.balance = this.balance + amount
    }
    
    fn withdraw(amount: double) -> bool {
        if this.balance >= amount {
            this.balance = this.balance - amount
            return true
        } else {
            return false
        }
    }
    
    fn getBalance() -> double {
        return this.balance
    }
    
    fn getAccountNumber() -> int {
        return this.accountNumber
    }
    
    fn transfer(other: BankAccount, amount: double) -> bool {
        if this.withdraw(amount) {
            other.deposit(amount)
            return true
        } else {
            return false
        }
    }
}

fn main() -> int {
    # Create accounts using primary constructor
    let account1 = BankAccount(12345, 1000.0)
    let account2 = BankAccount(67890, 500.0)
    
    # Deposit
    account1.deposit(200.0)
    printd(account1.getBalance())  # 1200.0
    
    # Withdraw
    account1.withdraw(300.0)
    printd(account1.getBalance())  # 900.0
    
    # Transfer
    account1.transfer(account2, 100.0)
    printd(account1.getBalance())  # 800.0
    printd(account2.getBalance())  # 600.0
    
    return 0
}
```

### Example: Geometric Shapes

```aurora
extern printd(x)

# Using primary constructor with mutable field
class Circle(var radius: double) {
    fn area() -> double {
        # π * r^2 (using 3.14159 as π)
        return 3.14159 * this.radius * this.radius
    }
    
    fn circumference() -> double {
        # 2 * π * r
        return 2.0 * 3.14159 * this.radius
    }
    
    fn scale(factor: double) {
        this.radius = this.radius * factor
    }
}

fn main() -> int {
    let circle = Circle(5.0)
    
    printd(circle.area())          # ~78.54
    printd(circle.circumference()) # ~31.42
    
    circle.scale(2.0)
    printd(circle.area())          # ~314.16
    
    return 0
}
```

---

## Memory Management

### Object Allocation

- Objects are allocated on the heap using `malloc`
- Returns a pointer to the object structure
- Fields are stored in contiguous memory layout

### Method Calling Convention

- Methods are compiled to regular functions with name mangling: `ClassName_methodName`
- Constructors support overloading via parameter type mangling: `ClassName_constructor_type1_type2...`
- First parameter is an implicit `this` pointer
- Other parameters are passed in declaration order

### Example: LLVM IR Representation

```llvm
# Class structure
%Point = type { double, double }

# Constructor with two parameters
define void @Point_constructor_d_d(%Point* %this, double %x, double %y) {
  %1 = getelementptr %Point, %Point* %this, i32 0, i32 0
  store double %x, double* %1
  %2 = getelementptr %Point, %Point* %this, i32 0, i32 1
  store double %y, double* %2
  ret void
}

# Overloaded constructor with one parameter
define void @Point_constructor_d(%Point* %this, double %value) {
  %1 = getelementptr %Point, %Point* %this, i32 0, i32 0
  store double %value, double* %1
  %2 = getelementptr %Point, %Point* %this, i32 0, i32 1
  store double %value, double* %2
  ret void
}

# Method
define double @Point_getX(%Point* %this) {
  %1 = getelementptr %Point, %Point* %this, i32 0, i32 0
  %2 = load double, double* %1
  ret double %2
}
```

Note: Type mangling scheme:
- `i` = int
- `d` = double
- `b` = bool
- `s` = string

---

## Limitations and Considerations

### Current Limitations

1. **No Inheritance**: Class inheritance is not yet supported
2. **No Polymorphism**: Virtual functions and interfaces are not yet supported
3. **No Garbage Collection**: Memory must be managed manually (or use RAII pattern)
4. **No Static Members**: Class-level static fields or methods are not yet supported

### Best Practices

1. **Initialize All Fields**: Initialize all fields in the constructor
2. **Use Getters/Setters**: Access fields through methods rather than direct access
3. **Minimize Public Interface**: Prefer `priv` modifiers, only expose necessary members
4. **Avoid Circular References**: Be careful with mutual references between objects

---

## Future Extensions

Planned OOP feature enhancements:

- **Inheritance**: Inheritance relationships between classes
- **Polymorphism**: Virtual functions and dynamic dispatch
- **Interfaces/Traits**: Abstract types and contracts
- **Operator Overloading**: Custom operator behavior
- **Properties**: Property syntax sugar with getters/setters
- **Destructors**: Resource cleanup and RAII
- **Static Members**: Class-level fields and methods
- **Nested Classes**: Classes defined inside other classes
- **Friend Functions**: External functions with access to private members

---

**Happy coding!** 🌅