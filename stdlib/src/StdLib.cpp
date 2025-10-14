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
    int64_t auroraStdPrintInt(int64_t value) {
        std::cout << value;
        std::cout.flush();
        return value;
    }
    
    double auroraStdPrintDouble(double value) {
        std::cout << value;
        std::cout.flush();
        return value;
    }
    
    int auroraStdPrintBool(int value) {
        std::cout << (value ? "true" : "false");
        std::cout.flush();
        return value;
    }
    
    void auroraStdPrintString(const char* str) {
        if (str) {
            std::cout << str;
            std::cout.flush();
        }
    }
    
    int64_t auroraStdPrintlnInt(int64_t value) {
        std::cout << value << std::endl;
        return value;
    }
    
    double auroraStdPrintlnDouble(double value) {
        std::cout << value << std::endl;
        return value;
    }
    
    int auroraStdPrintlnBool(int value) {
        std::cout << (value ? "true" : "false") << std::endl;
        return value;
    }
    
    void auroraStdPrintlnString(const char* str) {
        if (str) {
            std::cout << str << std::endl;
        }
    }
}

// ============================================================================
// String Operations
// ============================================================================

extern "C" {
    int64_t auroraStdStringLength(const char* str) {
        return str ? static_cast<int64_t>(strlen(str)) : 0;
    }
    
    char* auroraStdStringConcat(const char* a, const char* b) {
        if (!a || !b) return nullptr;
        
        size_t len_a = strlen(a);
        size_t len_b = strlen(b);
        char* result = new char[len_a + len_b + 1];
        
        strcpy(result, a);
        strcat(result, b);
        
        return result;
    }
    
    int auroraStdStringCompare(const char* a, const char* b) {
        if (!a || !b) return 0;
        return strcmp(a, b);
    }
    
    int auroraStdStringEquals(const char* a, const char* b) {
        if (!a || !b) return 0;
        return strcmp(a, b) == 0 ? 1 : 0;
    }
    
    char* auroraStdStringSubstring(const char* str, int64_t start, int64_t end) {
        if (!str) return nullptr;
        
        int64_t len = static_cast<int64_t>(strlen(str));
        if (start < 0 || end > len || start >= end) return nullptr;
        
        int64_t sub_len = end - start;
        char* result = new char[sub_len + 1];
        strncpy(result, str + start, sub_len);
        result[sub_len] = '\0';
        
        return result;
    }
    
    int64_t auroraStdStringToInt(const char* str) {
        if (!str) return 0;
        return static_cast<int64_t>(std::stoll(str));
    }
    
    double auroraStdStringToDouble(const char* str) {
        if (!str) return 0.0;
        return std::stod(str);
    }
    
    char* auroraStdIntToString(int64_t value) {
        std::string str = std::to_string(value);
        char* result = new char[str.length() + 1];
        strcpy(result, str.c_str());
        return result;
    }
    
    char* auroraStdDoubleToString(double value) {
        std::string str = std::to_string(value);
        char* result = new char[str.length() + 1];
        strcpy(result, str.c_str());
        return result;
    }
    
    void auroraStdStringFree(char* str) {
        delete[] str;
    }
}

// ============================================================================
// Math Functions
// ============================================================================

extern "C" {
    double auroraStdSin(double x) { return std::sin(x); }
    double auroraStdCos(double x) { return std::cos(x); }
    double auroraStdTan(double x) { return std::tan(x); }
    
    double auroraStdAsin(double x) { return std::asin(x); }
    double auroraStdAcos(double x) { return std::acos(x); }
    double auroraStdAtan(double x) { return std::atan(x); }
    double auroraStdAtan2(double y, double x) { return std::atan2(y, x); }
    
    double auroraStdExp(double x) { return std::exp(x); }
    double auroraStdLog(double x) { return std::log(x); }
    double auroraStdLog10(double x) { return std::log10(x); }
    double auroraStdPow(double base, double exp) { return std::pow(base, exp); }
    double auroraStdSqrt(double x) { return std::sqrt(x); }
    
    double auroraStdFloor(double x) { return std::floor(x); }
    double auroraStdCeil(double x) { return std::ceil(x); }
    double auroraStdRound(double x) { return std::round(x); }
    
    static bool random_seeded = false;
    
    int64_t auroraStdRandomInt(int64_t min, int64_t max) {
        if (!random_seeded) {
            std::srand(static_cast<unsigned int>(std::time(nullptr)));
            random_seeded = true;
        }
        if (max <= min) return min;
        return min + (std::rand() % (max - min));
    }
    
    double auroraStdRandomDouble() {
        if (!random_seeded) {
            std::srand(static_cast<unsigned int>(std::time(nullptr)));
            random_seeded = true;
        }
        return static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX);
    }
    
    void auroraStdRandomSeed(int64_t seed) {
        std::srand(static_cast<unsigned int>(seed));
        random_seeded = true;
    }
}

// ============================================================================
// Time Functions
// ============================================================================

extern "C" {
    int64_t auroraStdTimeNow() {
        return static_cast<int64_t>(std::time(nullptr));
    }
    
    int64_t auroraStdTimeNowMillis() {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    }
    
    void auroraStdSleepMillis(int64_t millis) {
        std::this_thread::sleep_for(std::chrono::milliseconds(millis));
    }
}

// ============================================================================
// File I/O Functions
// ============================================================================

extern "C" {
    char* auroraStdFileReadAll(const char* path) {
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
    
    int auroraStdFileWrite(const char* path, const char* content) {
        if (!path || !content) return -1;
        
        std::ofstream file(path);
        if (!file.is_open()) return -1;
        
        file << content;
        return 0;
    }
    
    int auroraStdFileAppend(const char* path, const char* content) {
        if (!path || !content) return -1;
        
        std::ofstream file(path, std::ios::app);
        if (!file.is_open()) return -1;
        
        file << content;
        return 0;
    }
    
    int auroraStdFileExists(const char* path) {
        if (!path) return 0;
        std::ifstream file(path);
        return file.good() ? 1 : 0;
    }
    
    int auroraStdFileDelete(const char* path) {
        if (!path) return -1;
        return std::remove(path);
    }
}

// ============================================================================
// System Functions
// ============================================================================

extern "C" {
    void auroraStdExit(int code) {
        std::exit(code);
    }
    
    char* auroraStdGetEnv(const char* name) {
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
    
    int64_t auroraStdGetArgCount() {
        return g_argc;
    }
    
    char* auroraStdGetArg(int64_t index) {
        if (index < 0 || index >= g_argc || !g_argv) return nullptr;
        
        const char* arg = g_argv[index];
        char* result = new char[strlen(arg) + 1];
        strcpy(result, arg);
        return result;
    }
}

} // namespace aurorax


