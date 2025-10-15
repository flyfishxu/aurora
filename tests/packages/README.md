# Package System Tests

This directory contains tests for AuroraLang's package system.

## Package Structure

```
com/
  aurora/
    math/
      Calculator.aur      # Calculator class in com.aurora.math
    utils/
      StringHelper.aur    # String utilities in com.aurora.utils
```

## Test Files

### testPackageBasic.aur
Tests basic package declaration and import:
- Declares a package: `package test.basic`
- Imports using package path: `import com.aurora.math.Calculator`
- Uses imported classes

### testPackageImport.aur
Tests backward compatibility:
- No package declaration (root package)
- Uses old-style string import: `import "com/aurora/math/Calculator"`
- Verifies mixed import styles work

## Running Tests

```bash
# Run from project root
cd tests/packages

# Test basic package usage
../../build/aurora testPackageBasic.aur

# Test backward compatibility
../../build/aurora testPackageImport.aur
```

## Expected Output

### testPackageBasic.aur
```
10000.0
30.0
30.0
50.0
9999.0
```

### testPackageImport.aur
```
20000.0
40.0
56.0
29999.0
```

