#pragma once

// AuroraLang Standard Library C++ Interface
// This header provides the C++ interface for standard library functions
// that are implemented in C++ for performance or system integration

#include <cstdint>
#include <string>

namespace aurorax {

// ============================================================================
// Core I/O Functions
// ============================================================================

// Print functions for different types
extern "C" {
    // Print integer to stdout
    int64_t auroraStdPrintInt(int64_t value);
    
    // Print double to stdout
    double auroraStdPrintDouble(double value);
    
    // Print boolean to stdout
    int auroraStdPrintBool(int value);
    
    // Print string to stdout
    void auroraStdPrintString(const char* str);
    
    // Print with newline variants
    int64_t auroraStdPrintlnInt(int64_t value);
    double auroraStdPrintlnDouble(double value);
    int auroraStdPrintlnBool(int value);
    void auroraStdPrintlnString(const char* str);
}

// ============================================================================
// String Operations
// ============================================================================

extern "C" {
    // String length
    int64_t auroraStdStringLength(const char* str);
    
    // String concatenation (returns new string)
    char* auroraStdStringConcat(const char* a, const char* b);
    
    // String comparison
    int auroraStdStringCompare(const char* a, const char* b);
    
    // String equality
    int auroraStdStringEquals(const char* a, const char* b);
    
    // Substring (from start to end, exclusive)
    char* auroraStdStringSubstring(const char* str, int64_t start, int64_t end);
    
    // String to integer conversion
    int64_t auroraStdStringToInt(const char* str);
    
    // String to double conversion
    double auroraStdStringToDouble(const char* str);
    
    // Integer to string conversion
    char* auroraStdIntToString(int64_t value);
    
    // Double to string conversion
    char* auroraStdDoubleToString(double value);
    
    // Free string memory
    void auroraStdStringFree(char* str);
}

// ============================================================================
// Math Functions (advanced)
// ============================================================================

extern "C" {
    // Trigonometric functions
    double auroraStdSin(double x);
    double auroraStdCos(double x);
    double auroraStdTan(double x);
    
    // Inverse trigonometric functions
    double auroraStdAsin(double x);
    double auroraStdAcos(double x);
    double auroraStdAtan(double x);
    double auroraStdAtan2(double y, double x);
    
    // Exponential and logarithmic functions
    double auroraStdExp(double x);
    double auroraStdLog(double x);
    double auroraStdLog10(double x);
    double auroraStdPow(double base, double exponent);
    double auroraStdSqrt(double x);
    
    // Rounding functions
    double auroraStdFloor(double x);
    double auroraStdCeil(double x);
    double auroraStdRound(double x);
    
    // Random number generation
    int64_t auroraStdRandomInt(int64_t min, int64_t max);
    double auroraStdRandomDouble();
    void auroraStdRandomSeed(int64_t seed);
}

// ============================================================================
// Time Functions
// ============================================================================

extern "C" {
    // Get current timestamp in seconds since epoch
    int64_t auroraStdTimeNow();
    
    // Get current timestamp in milliseconds
    int64_t auroraStdTimeNowMillis();
    
    // Sleep for specified milliseconds
    void auroraStdSleepMillis(int64_t millis);
}

// ============================================================================
// File I/O Functions
// ============================================================================

extern "C" {
    // Read entire file as string
    char* auroraStdFileReadAll(const char* path);
    
    // Write string to file
    int auroraStdFileWrite(const char* path, const char* content);
    
    // Append string to file
    int auroraStdFileAppend(const char* path, const char* content);
    
    // Check if file exists
    int auroraStdFileExists(const char* path);
    
    // Delete file
    int auroraStdFileDelete(const char* path);
}

// ============================================================================
// System Functions
// ============================================================================

extern "C" {
    // Exit program with code
    void auroraStdExit(int code);
    
    // Get environment variable
    char* auroraStdGetEnv(const char* name);
    
    // Get command line arguments count
    int64_t auroraStdGetArgCount();
    
    // Get command line argument at index
    char* auroraStdGetArg(int64_t index);
}

} // namespace aurorax


