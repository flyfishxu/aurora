# AuroraLang Test Suite

This directory contains comprehensive tests for the AuroraLang programming language, organized in a streamlined structure that covers all core language features.

## Test Structure

### Core Tests (`core/`)
- **`basicSyntax.aur`** - Basic language features: functions, operators, expressions
- **`operators.aur`** - All supported operators: arithmetic, comparison, logical, bitwise, ternary
- **`controlFlow.aur`** - Control structures: if/else, while/for loops, break/continue
- **`typeSystem.aur`** - Type system: annotations, conversions, literals, null handling

### Object-Oriented Programming (`oop/`)
- **`classesAndObjects.aur`** - Classes, objects, constructors, methods, member access
- **`designPatterns.aur`** - Common design patterns: Singleton, Factory, Builder

### Module System (`modules/`)
- **`importSystem.aur`** - Import statements and external function declarations

### Error Handling (`errors/`)
- **`errorHandling.aur`** - Error conditions, edge cases, boundary values

### Standard Library (`stdlib/`)
- **`testMathComprehensive.aur`** - Math library functions
- **`testArrayUtils.aur`** - Array utility functions

### Arrays (`arrays/`)
- **`arrayBasic.aur`** - Basic array operations
- **`arrayComprehensive.aur`** - Advanced array features

### Basic Tests (`basic/`)
- **`exitCode.aur`** - Exit code testing

### Broken Tests (`broken/`)
- **`oop_demo_broken.aur`** - OOP demonstration with intentional issues for testing

## Running Tests

### Basic Test Run
```bash
./run_tests.sh
```

### Verbose Output
```bash
./run_tests.sh --verbose
```

### With Output Validation
```bash
./run_tests.sh --validate
```

## Test Design Principles

1. **Comprehensive Coverage** - Each test file covers a specific language feature completely
2. **Clear Structure** - Tests are organized by functionality, not by implementation details
3. **Minimal Redundancy** - No duplicate tests across different files
4. **Clear Markers** - Each test section has clear start/end markers for validation
5. **Error Testing** - Edge cases and error conditions are properly tested

## Test Categories

### Core Language Features
- Function definitions and calls
- Variable declarations and assignments
- Type annotations and conversions
- Operator precedence and associativity
- Control flow structures

### Object-Oriented Programming
- Class definitions and instantiation
- Constructor overloading
- Method definitions and calls
- Member access and visibility
- Design pattern implementations

### Advanced Features
- Module imports and exports
- External function declarations
- Error handling and edge cases
- Memory management
- Type safety

## Validation

Each test file includes:
- Clear test markers for output validation
- Comprehensive test coverage
- Error condition testing
- Edge case handling
- Performance considerations

## Maintenance

When adding new language features:
1. Add tests to the appropriate category
2. Update the test runner validation logic
3. Ensure tests are comprehensive but not redundant
4. Update this README if new categories are added

## Test Results

The test runner provides:
- Pass/fail status for each test
- Detailed output validation (with `--validate`)
- Summary statistics
- Error reporting with verbose output