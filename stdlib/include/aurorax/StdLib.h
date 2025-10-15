#pragma once

// AuroraLang Standard Library - Native Interface
// Clean, minimal C API for performance-critical operations
// Design inspired by Kotlin and Swift standard libraries

#include <cstdint>

namespace aurorax {

// ============================================================================
// Core I/O Functions
// ============================================================================

extern "C" {
    // Print functions (return value for chaining)
    int64_t aurora_print_int(int64_t value);
    double aurora_print_double(double value);
    int aurora_print_bool(int value);
    void aurora_print_string(const char* str);
    
    // Print with newline
    int64_t aurora_println_int(int64_t value);
    double aurora_println_double(double value);
    int aurora_println_bool(int value);
    void aurora_println_string(const char* str);
}

// ============================================================================
// String Operations
// ============================================================================

extern "C" {
    int64_t aurora_string_length(const char* str);
    char* aurora_string_concat(const char* a, const char* b);
    int aurora_string_compare(const char* a, const char* b);
    int aurora_string_equals(const char* a, const char* b);
    char* aurora_string_substring(const char* str, int64_t start, int64_t end);
    
    // Conversions
    int64_t aurora_string_to_int(const char* str);
    double aurora_string_to_double(const char* str);
    char* aurora_int_to_string(int64_t value);
    char* aurora_double_to_string(double value);
    
    void aurora_string_free(char* str);
}

// ============================================================================
// Math Functions
// ============================================================================

extern "C" {
    // Trigonometric
    double aurora_sin(double x);
    double aurora_cos(double x);
    double aurora_tan(double x);
    double aurora_asin(double x);
    double aurora_acos(double x);
    double aurora_atan(double x);
    double aurora_atan2(double y, double x);
    
    // Exponential and logarithmic
    double aurora_exp(double x);
    double aurora_log(double x);
    double aurora_log10(double x);
    double aurora_pow(double base, double exponent);
    double aurora_sqrt(double x);
    
    // Rounding
    double aurora_floor(double x);
    double aurora_ceil(double x);
    double aurora_round(double x);
    
    // Random
    int64_t aurora_random_int(int64_t min, int64_t max);
    double aurora_random_double();
    void aurora_random_seed(int64_t seed);
}

// ============================================================================
// Time Functions
// ============================================================================

extern "C" {
    int64_t aurora_time_now();
    int64_t aurora_time_now_millis();
    void aurora_sleep_millis(int64_t millis);
}

// ============================================================================
// File I/O
// ============================================================================

extern "C" {
    char* aurora_file_read(const char* path);
    int aurora_file_write(const char* path, const char* content);
    int aurora_file_append(const char* path, const char* content);
    int aurora_file_exists(const char* path);
    int aurora_file_delete(const char* path);
}

// ============================================================================
// System Functions
// ============================================================================

extern "C" {
    void aurora_exit(int code);
    char* aurora_get_env(const char* name);
    int64_t aurora_arg_count();
    char* aurora_arg_get(int64_t index);
}

} // namespace aurorax


