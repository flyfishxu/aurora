#pragma once

namespace aurora {

// ANSI color codes for terminal output
namespace Color {
    inline constexpr const char* Reset   = "\033[0m";
    inline constexpr const char* Bold    = "\033[1m";
    inline constexpr const char* Gray    = "\033[90m";
    inline constexpr const char* Red     = "\033[31m";
    inline constexpr const char* Yellow  = "\033[33m";
    inline constexpr const char* Blue    = "\033[34m";
    inline constexpr const char* Cyan    = "\033[36m";
    inline constexpr const char* Green   = "\033[32m";
    inline constexpr const char* Magenta = "\033[35m";
} // namespace Color

} // namespace aurora

