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
    
    char aurora_string_char_at(const char* str, int64_t index) {
        if (!str) return '\0';
        int64_t len = static_cast<int64_t>(strlen(str));
        if (index < 0 || index >= len) return '\0';
        return str[index];
    }
    
    char* aurora_string_trim(const char* str) {
        if (!str) return nullptr;
        
        const char* start = str;
        while (*start && std::isspace(static_cast<unsigned char>(*start))) {
            start++;
        }
        
        if (*start == '\0') {
            char* result = new char[1];
            result[0] = '\0';
            return result;
        }
        
        const char* end = str + strlen(str) - 1;
        while (end > start && std::isspace(static_cast<unsigned char>(*end))) {
            end--;
        }
        
        size_t len = end - start + 1;
        char* result = new char[len + 1];
        strncpy(result, start, len);
        result[len] = '\0';
        
        return result;
    }
    
    char* aurora_string_trim_start(const char* str) {
        if (!str) return nullptr;
        
        const char* start = str;
        while (*start && std::isspace(static_cast<unsigned char>(*start))) {
            start++;
        }
        
        size_t len = strlen(start);
        char* result = new char[len + 1];
        strcpy(result, start);
        
        return result;
    }
    
    char* aurora_string_trim_end(const char* str) {
        if (!str) return nullptr;
        
        size_t len = strlen(str);
        if (len == 0) {
            char* result = new char[1];
            result[0] = '\0';
            return result;
        }
        
        const char* end = str + len - 1;
        while (end >= str && std::isspace(static_cast<unsigned char>(*end))) {
            end--;
        }
        
        len = end - str + 1;
        char* result = new char[len + 1];
        strncpy(result, str, len);
        result[len] = '\0';
        
        return result;
    }
    
    int aurora_string_starts_with(const char* str, const char* prefix) {
        if (!str || !prefix) return 0;
        size_t str_len = strlen(str);
        size_t prefix_len = strlen(prefix);
        if (prefix_len > str_len) return 0;
        return strncmp(str, prefix, prefix_len) == 0 ? 1 : 0;
    }
    
    int aurora_string_ends_with(const char* str, const char* suffix) {
        if (!str || !suffix) return 0;
        size_t str_len = strlen(str);
        size_t suffix_len = strlen(suffix);
        if (suffix_len > str_len) return 0;
        return strcmp(str + str_len - suffix_len, suffix) == 0 ? 1 : 0;
    }
    
    int aurora_string_contains(const char* str, const char* substr) {
        if (!str || !substr) return 0;
        return strstr(str, substr) != nullptr ? 1 : 0;
    }
    
    int64_t aurora_string_index_of(const char* str, const char* substr) {
        if (!str || !substr) return -1;
        const char* found = strstr(str, substr);
        if (!found) return -1;
        return static_cast<int64_t>(found - str);
    }
    
    int64_t aurora_string_last_index_of(const char* str, const char* substr) {
        if (!str || !substr) return -1;
        
        const char* last_found = nullptr;
        const char* current = str;
        
        while ((current = strstr(current, substr)) != nullptr) {
            last_found = current;
            current++;
        }
        
        if (!last_found) return -1;
        return static_cast<int64_t>(last_found - str);
    }
    
    char* aurora_string_to_upper(const char* str) {
        if (!str) return nullptr;
        
        size_t len = strlen(str);
        char* result = new char[len + 1];
        
        for (size_t i = 0; i < len; i++) {
            result[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(str[i])));
        }
        result[len] = '\0';
        
        return result;
    }
    
    char* aurora_string_to_lower(const char* str) {
        if (!str) return nullptr;
        
        size_t len = strlen(str);
        char* result = new char[len + 1];
        
        for (size_t i = 0; i < len; i++) {
            result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(str[i])));
        }
        result[len] = '\0';
        
        return result;
    }
    
    char* aurora_string_replace(const char* str, const char* from, const char* to) {
        if (!str || !from || !to) return nullptr;
        
        std::string s = str;
        size_t from_len = strlen(from);
        size_t to_len = strlen(to);
        size_t pos = 0;
        
        while ((pos = s.find(from, pos)) != std::string::npos) {
            s.replace(pos, from_len, to);
            pos += to_len;
        }
        
        char* result = new char[s.length() + 1];
        strcpy(result, s.c_str());
        return result;
    }
    
    char* aurora_string_repeat(const char* str, int64_t count) {
        if (!str || count <= 0) {
            char* result = new char[1];
            result[0] = '\0';
            return result;
        }
        
        size_t len = strlen(str);
        size_t total_len = len * count;
        char* result = new char[total_len + 1];
        
        for (int64_t i = 0; i < count; i++) {
            strcpy(result + (i * len), str);
        }
        result[total_len] = '\0';
        
        return result;
    }
    
    int64_t aurora_string_to_int(const char* str) {
        if (!str) return 0;
        try {
            return static_cast<int64_t>(std::stoll(str));
        } catch (...) {
            return 0;
        }
    }
    
    double aurora_string_to_double(const char* str) {
        if (!str) return 0.0;
        try {
            return std::stod(str);
        } catch (...) {
            return 0.0;
        }
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
    
    char* aurora_bool_to_string(int value) {
        const char* str = value ? "true" : "false";
        char* result = new char[strlen(str) + 1];
        strcpy(result, str);
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


