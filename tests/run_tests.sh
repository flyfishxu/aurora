#!/bin/bash

# AuroraLang Comprehensive Test Runner
# Runs all tests in the tests/ directory with optional output validation

echo "============================================"
echo "AuroraLang Test Suite"
echo "============================================"
echo ""

# Determine script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
TESTS_DIR="$SCRIPT_DIR"
AURORA_BIN="$PROJECT_ROOT/build/aurora"

PASS=0
FAIL=0
SKIP=0
TOTAL_CHECKS=0
PASSED_CHECKS=0

# Parse command line arguments
VERBOSE=false
VALIDATE_OUTPUT=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        --validate)
            VALIDATE_OUTPUT=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  -v, --verbose    Show detailed output for each test"
            echo "  --validate       Validate specific output values (comprehensive mode)"
            echo "  -h, --help       Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0                  # Run all tests"
            echo "  $0 --verbose        # Run with detailed output"
            echo "  $0 --validate       # Run with output validation"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use -h or --help for usage information"
            exit 1
            ;;
    esac
done

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Check if aurora binary exists
if [ ! -f "$AURORA_BIN" ]; then
    echo -e "${RED}Error: Aurora binary not found at $AURORA_BIN${NC}"
    echo "Please run: cmake --build build"
    exit 1
fi

# Function to check if output contains expected string (for validation mode)
check_output() {
    local output="$1"
    local expected="$2"
    local description="$3"
    
    ((TOTAL_CHECKS++))
    
    if echo "$output" | grep -q "$expected"; then
        if [ "$VERBOSE" = true ]; then
            echo -e "    ${GREEN}✓${NC} $description"
        fi
        ((PASSED_CHECKS++))
        return 0
    else
        echo -e "    ${RED}✗${NC} $description"
        echo -e "      Expected to find: ${CYAN}$expected${NC}"
        return 1
    fi
}

# Function to run a single test
run_test() {
    local file=$1
    local filename=$(basename "$file")
    local category=$(basename "$(dirname "$file")")
    
    # Skip broken test files
    if [[ "$filename" =~ "_broken" ]]; then
        echo -e "  ${YELLOW}⊘${NC} $filename (skipped: marked as broken)"
        ((SKIP++))
        return
    fi
    
    # Skip library files (no main function)
    if [[ "$filename" == "mathlib.aur" ]]; then
        echo -e "  ${YELLOW}⊘${NC} $filename (skipped: library file)"
        ((SKIP++))
        return
    fi
    
    if [ "$VALIDATE_OUTPUT" = true ]; then
        echo -e "  ${BLUE}━━━ Testing: $filename ━━━${NC}"
    else
        echo -n "  Testing: $filename ... "
    fi
    
    # Run the test and capture output
    OUTPUT=$($AURORA_BIN "$file" 2>&1)
    EXIT_CODE=$?
    
    local test_passed=0
    
    # Special case: exit_code.aur should return exit code 1
    if [[ "$filename" == "exit_code.aur" ]]; then
        if [ $EXIT_CODE -eq 1 ]; then
            if [ "$VALIDATE_OUTPUT" = true ]; then
                echo -e "    ${GREEN}✓${NC} Exit code 1 as expected"
            else
                echo -e "${GREEN}✓ PASSED${NC} (exit code 1 as expected)"
            fi
            ((PASS++))
        else
            echo -e "${RED}✗ FAILED${NC} (expected exit code 1, got $EXIT_CODE)"
            ((FAIL++))
        fi
        [ "$VALIDATE_OUTPUT" = true ] && echo ""
        return
    fi
    
    # Special case: error tests are expected to fail compilation
    if [[ "$category" == "errors" ]] && [[ "$filename" =~ "type_mismatch" || "$filename" =~ "syntax_error" ]]; then
        if [ $EXIT_CODE -ne 0 ]; then
            if [ "$VALIDATE_OUTPUT" = true ]; then
                echo -e "    ${GREEN}✓${NC} Error detected as expected (exit code $EXIT_CODE)"
            else
                echo -e "${GREEN}✓ PASSED${NC} (failed as expected)"
            fi
            ((PASS++))
        else
            echo -e "${RED}✗ FAILED${NC} (should have failed but didn't)"
            ((FAIL++))
        fi
        [ "$VALIDATE_OUTPUT" = true ] && echo ""
        return
    fi
    
    # Output validation mode
    if [ "$VALIDATE_OUTPUT" = true ]; then
        case "$filename" in
            "basicSyntax.aur")
                check_output "$OUTPUT" "15" "add(10.0, 5.0) = 15"
                check_output "$OUTPUT" "5" "subtract(10.0, 5.0) = 5"
                check_output "$OUTPUT" "50" "multiply(10.0, 5.0) = 50"
                check_output "$OUTPUT" "2" "divide(10.0, 5.0) = 2"
                test_passed=$?
                ;;
            "operators.aur")
                check_output "$OUTPUT" "1000" "Test start marker"
                check_output "$OUTPUT" "13" "10 + 3 = 13"
                check_output "$OUTPUT" "7" "10 - 3 = 7"
                test_passed=$?
                ;;
            "controlFlow.aur")
                check_output "$OUTPUT" "2000" "Test start marker"
                check_output "$OUTPUT" "1" "if x > y condition"
                check_output "$OUTPUT" "0" "while loop counter"
                test_passed=$?
                ;;
            "typeSystem.aur")
                check_output "$OUTPUT" "3000" "Test start marker"
                check_output "$OUTPUT" "42" "int type annotation"
                check_output "$OUTPUT" "1" "boolean true value"
                test_passed=$?
                ;;
            "classesAndObjects.aur")
                check_output "$OUTPUT" "9999" "Test start marker"
                check_output "$OUTPUT" "3" "Point x coordinate"
                check_output "$OUTPUT" "4" "Point y coordinate"
                test_passed=$?
                ;;
            "designPatterns.aur")
                check_output "$OUTPUT" "9999" "Test start marker"
                check_output "$OUTPUT" "5000" "Singleton pattern test"
                check_output "$OUTPUT" "100" "Config max connections"
                test_passed=$?
                ;;
            "importSystem.aur")
                check_output "$OUTPUT" "9999" "Test start marker"
                check_output "$OUTPUT" "42" "External function test"
                check_output "$OUTPUT" "30" "addNumbers(10, 20) = 30"
                test_passed=$?
                ;;
            "errorHandling.aur")
                check_output "$OUTPUT" "9999" "Test start marker"
                check_output "$OUTPUT" "7000" "Division by zero test"
                check_output "$OUTPUT" "7001" "Boundary values test"
                test_passed=$?
                ;;
            "testImport.aur")
                check_output "$OUTPUT" "25" "square(5.0) = 25"
                check_output "$OUTPUT" "27" "cube(3.0) = 27"
                test_passed=$?
                ;;
            "arrayBasic.aur")
                check_output "$OUTPUT" "10" "Array element access"
                test_passed=$?
                ;;
            "testMathComprehensive.aur")
                check_output "$OUTPUT" "1000" "Start marker"
                check_output "$OUTPUT" "9999" "End marker"
                test_passed=$?
                ;;
            "exitCode.aur")
                check_output "$OUTPUT" "42" "Exit code test"
                test_passed=$?
                ;;
            "oop_demo_broken.aur")
                check_output "$OUTPUT" "9999" "Test suite start marker"
                check_output "$OUTPUT" "1000" "Counter test marker"
                check_output "$OUTPUT" "2000" "Point test marker"
                check_output "$OUTPUT" "3000" "Rectangle test marker"
                check_output "$OUTPUT" "4000" "Bank account test marker"
                test_passed=$?
                ;;
            *)
                # For other tests, just check exit code
                if [ $EXIT_CODE -eq 0 ]; then
                    echo -e "    ${GREEN}✓${NC} Program executed successfully"
                    test_passed=0
                else
                    echo -e "    ${RED}✗${NC} Program failed with exit code $EXIT_CODE"
                    if [ "$VERBOSE" = true ]; then
                        echo -e "${CYAN}Output:${NC}"
                        echo "$OUTPUT" | head -20
                    fi
                    test_passed=1
                fi
                ;;
        esac
        
        if [ $test_passed -eq 0 ]; then
            ((PASS++))
        else
            ((FAIL++))
        fi
        echo ""
    else
        # Simple mode: just check exit code
        if [ $EXIT_CODE -eq 0 ]; then
            echo -e "${GREEN}✓ PASSED${NC}"
            ((PASS++))
        else
            echo -e "${RED}✗ FAILED${NC} (exit code: $EXIT_CODE)"
            if [ "$VERBOSE" = true ]; then
                echo "$OUTPUT" | head -10
            fi
            ((FAIL++))
        fi
    fi
}

# Function to run tests in a category
run_category() {
    local category=$1
    local category_path="$TESTS_DIR/$category"
    
    if [ ! -d "$category_path" ]; then
        return
    fi
    
    local file_count=$(find "$category_path" -name "*.aur" | wc -l | tr -d ' ')
    if [ "$file_count" -eq 0 ]; then
        return
    fi
    
    echo -e "${BLUE}━━━ Category: $category ($file_count tests) ━━━${NC}"
    echo ""
    
    for file in "$category_path"/*.aur; do
        if [ ! -f "$file" ]; then
            continue
        fi
        
        run_test "$file"
    done
    
    echo ""
}

# Run tests by category (new streamlined structure)
run_category "core"
run_category "oop"
run_category "modules"
run_category "errors"
run_category "arrays"
run_category "stdlib"

# Summary
echo "============================================"
echo -e "${CYAN}Test Summary${NC}"
echo "============================================"
if [ $FAIL -eq 0 ]; then
    echo -e "${GREEN}✓ All tests passed!${NC}"
else
    echo -e "${RED}✗ Some tests failed${NC}"
fi
echo ""
echo -e "  ${GREEN}Tests Passed:${NC}    $PASS"
if [ $FAIL -gt 0 ]; then
    echo -e "  ${RED}Tests Failed:${NC}    $FAIL"
fi
if [ $SKIP -gt 0 ]; then
    echo -e "  ${YELLOW}Tests Skipped:${NC}   $SKIP"
fi
echo -e "  Total Tests:     $((PASS + FAIL + SKIP))"

if [ "$VALIDATE_OUTPUT" = true ] && [ $TOTAL_CHECKS -gt 0 ]; then
    echo ""
    echo -e "  ${GREEN}Checks Passed:${NC}   $PASSED_CHECKS / $TOTAL_CHECKS"
fi
echo "============================================"

# Exit with failure if any tests failed
if [ $FAIL -gt 0 ]; then
    exit 1
fi

exit 0

