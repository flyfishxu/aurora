#pragma once

#include <string>

namespace aurora {

/// Get the directory containing the current executable
std::string getExecutablePath();

/// Get the directory containing the executable (without the filename)
std::string getExecutableDirectory();

/// Get the sysroot directory
/// Priority: --sysroot arg > AURORA_HOME env > exe/../ > compile-time AURORA_SYSROOT
std::string getSysroot();

/// Set sysroot explicitly (from command line --sysroot)
void setSysroot(const std::string& path);

/// Initialize sysroot system
void initializeSysroot();

} // namespace aurora

