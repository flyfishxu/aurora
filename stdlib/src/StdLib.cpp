#include "aurorax/StdLib.h"
#include <iostream>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>

namespace aurorax {

// ============================================================================
// Core I/O Functions
// ============================================================================

extern "C" {
    int64_t aurora_print_int(int64_t value) {
        std::cout << value;
        std::cout.flush();
        return value;
    }
    
    double aurora_print_double(double value) {
        std::cout << value;
        std::cout.flush();
        return value;
    }
    
    int aurora_print_bool(int value) {
        std::cout << (value ? "true" : "false");
        std::cout.flush();
        return value;
    }
    
    void aurora_print_string(const char* str) {
        if (str) {
            std::cout << str;
            std::cout.flush();
        }
    }
    
    int64_t aurora_println_int(int64_t value) {
        std::cout << value << std::endl;
        return value;
    }
    
    double aurora_println_double(double value) {
        std::cout << value << std::endl;
        return value;
    }
    
    int aurora_println_bool(int value) {
        std::cout << (value ? "true" : "false") << std::endl;
        return value;
    }
    
    void aurora_println_string(const char* str) {
        if (str) {
            std::cout << str << std::endl;
        }
    }
}

// ============================================================================
// String Operations
// ============================================================================

extern "C" {
    int64_t aurora_string_length(const char* str) {
        return str ? static_cast<int64_t>(strlen(str)) : 0;
    }
    
    char* aurora_string_concat(const char* a, const char* b) {
        if (!a || !b) return nullptr;
        
        size_t len_a = strlen(a);
        size_t len_b = strlen(b);
        char* result = new char[len_a + len_b + 1];
        
        strcpy(result, a);
        strcat(result, b);
        
        return result;
    }
    
    int aurora_string_compare(const char* a, const char* b) {
        if (!a || !b) return 0;
        return strcmp(a, b);
    }
    
    int aurora_string_equals(const char* a, const char* b) {
        if (!a || !b) return 0;
        return strcmp(a, b) == 0 ? 1 : 0;
    }
    
    char* aurora_string_substring(const char* str, int64_t start, int64_t end) {
        if (!str) return nullptr;
        
        int64_t len = static_cast<int64_t>(strlen(str));
        if (start < 0 || end > len || start >= end) return nullptr;
        
        int64_t sub_len = end - start;
        char* result = new char[sub_len + 1];
        strncpy(result, str + start, sub_len);
        result[sub_len] = '\0';
        
        return result;
    }
    
    int64_t aurora_string_to_int(const char* str) {
        if (!str) return 0;
        return static_cast<int64_t>(std::stoll(str));
    }
    
    double aurora_string_to_double(const char* str) {
        if (!str) return 0.0;
        return std::stod(str);
    }
    
    char* aurora_int_to_string(int64_t value) {
        std::string str = std::to_string(value);
        char* result = new char[str.length() + 1];
        strcpy(result, str.c_str());
        return result;
    }
    
    char* aurora_double_to_string(double value) {
        std::string str = std::to_string(value);
        char* result = new char[str.length() + 1];
        strcpy(result, str.c_str());
        return result;
    }
    
    void aurora_string_free(char* str) {
        delete[] str;
    }
}

// ============================================================================
// Math Functions
// ============================================================================

extern "C" {
    double aurora_sin(double x) { return std::sin(x); }
    double aurora_cos(double x) { return std::cos(x); }
    double aurora_tan(double x) { return std::tan(x); }
    
    double aurora_asin(double x) { return std::asin(x); }
    double aurora_acos(double x) { return std::acos(x); }
    double aurora_atan(double x) { return std::atan(x); }
    double aurora_atan2(double y, double x) { return std::atan2(y, x); }
    
    double aurora_exp(double x) { return std::exp(x); }
    double aurora_log(double x) { return std::log(x); }
    double aurora_log10(double x) { return std::log10(x); }
    double aurora_pow(double base, double exp) { return std::pow(base, exp); }
    double aurora_sqrt(double x) { return std::sqrt(x); }
    
    double aurora_floor(double x) { return std::floor(x); }
    double aurora_ceil(double x) { return std::ceil(x); }
    double aurora_round(double x) { return std::round(x); }
    
    static bool random_seeded = false;
    
    int64_t aurora_random_int(int64_t min, int64_t max) {
        if (!random_seeded) {
            std::srand(static_cast<unsigned int>(std::time(nullptr)));
            random_seeded = true;
        }
        if (max <= min) return min;
        return min + (std::rand() % (max - min));
    }
    
    double aurora_random_double() {
        if (!random_seeded) {
            std::srand(static_cast<unsigned int>(std::time(nullptr)));
            random_seeded = true;
        }
        return static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX);
    }
    
    void aurora_random_seed(int64_t seed) {
        std::srand(static_cast<unsigned int>(seed));
        random_seeded = true;
    }
}

// ============================================================================
// Time Functions
// ============================================================================

extern "C" {
    int64_t aurora_time_now() {
        return static_cast<int64_t>(std::time(nullptr));
    }
    
    int64_t aurora_time_now_millis() {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    }
    
    void aurora_sleep_millis(int64_t millis) {
        std::this_thread::sleep_for(std::chrono::milliseconds(millis));
    }
}

// ============================================================================
// File I/O Functions
// ============================================================================

extern "C" {
    char* aurora_file_read(const char* path) {
        if (!path) return nullptr;
        
        std::ifstream file(path);
        if (!file.is_open()) return nullptr;
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();
        
        char* result = new char[content.length() + 1];
        strcpy(result, content.c_str());
        
        return result;
    }
    
    int aurora_file_write(const char* path, const char* content) {
        if (!path || !content) return -1;
        
        std::ofstream file(path);
        if (!file.is_open()) return -1;
        
        file << content;
        return 0;
    }
    
    int aurora_file_append(const char* path, const char* content) {
        if (!path || !content) return -1;
        
        std::ofstream file(path, std::ios::app);
        if (!file.is_open()) return -1;
        
        file << content;
        return 0;
    }
    
    int aurora_file_exists(const char* path) {
        if (!path) return 0;
        std::ifstream file(path);
        return file.good() ? 1 : 0;
    }
    
    int aurora_file_delete(const char* path) {
        if (!path) return -1;
        return std::remove(path);
    }
}

// ============================================================================
// System Functions
// ============================================================================

extern "C" {
    void aurora_exit(int code) {
        std::exit(code);
    }
    
    char* aurora_get_env(const char* name) {
        if (!name) return nullptr;
        const char* value = std::getenv(name);
        if (!value) return nullptr;
        
        char* result = new char[strlen(value) + 1];
        strcpy(result, value);
        return result;
    }
    
    // These will be set by the main program
    static int64_t g_argc = 0;
    static char** g_argv = nullptr;
    
    int64_t aurora_arg_count() {
        return g_argc;
    }
    
    char* aurora_arg_get(int64_t index) {
        if (index < 0 || index >= g_argc || !g_argv) return nullptr;
        
        const char* arg = g_argv[index];
        char* result = new char[strlen(arg) + 1];
        strcpy(result, arg);
        return result;
    }
}

} // namespace aurorax


