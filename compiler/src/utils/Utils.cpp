#include "aurora/Utils.h"
#include "aurora/Logger.h"
#include <filesystem>
#include <cstdlib>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#elif defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace aurora {

static std::string sysrootPath;
static bool sysrootInitialized = false;

std::string getExecutablePath() {
#ifdef __APPLE__
    char path[1024];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        return std::string(path);
    }
    return "";
#elif defined(_WIN32)
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return std::string(path);
#else
    char path[1024];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0';
        return std::string(path);
    }
    return "";
#endif
}

std::string getExecutableDirectory() {
    std::string exePath = getExecutablePath();
    if (exePath.empty()) {
        return "";
    }
    
    std::filesystem::path p(exePath);
    return p.parent_path().string();
}

void setSysroot(const std::string& path) {
    if (std::filesystem::exists(path)) {
        sysrootPath = std::filesystem::canonical(path).string();
        sysrootInitialized = true;
        Logger::instance().debug("Sysroot set explicitly: " + sysrootPath, "Sysroot");
    } else {
        Logger::instance().warning("Sysroot path does not exist: " + path);
    }
}

void initializeSysroot() {
    if (sysrootInitialized && !sysrootPath.empty()) {
        // Already set via setSysroot() (e.g., --sysroot flag)
        return;
    }
    
    auto& logger = Logger::instance();
    logger.debug("Initializing sysroot...", "Sysroot");
    
    // Priority 1: Already set via setSysroot() from --sysroot argument
    // (checked above)
    
    // Priority 2: AURORA_HOME environment variable
    const char* auroraHome = std::getenv("AURORA_HOME");
    if (auroraHome && auroraHome[0] != '\0') {
        std::string homePath(auroraHome);
        logger.debug("Found AURORA_HOME env var: " + homePath, "Sysroot");
        if (std::filesystem::exists(homePath)) {
            sysrootPath = std::filesystem::canonical(homePath).string();
            sysrootInitialized = true;
            logger.info("Sysroot from AURORA_HOME: " + sysrootPath);
            return;
        } else {
            logger.warning("AURORA_HOME path does not exist: " + homePath);
        }
    }
    
    // Priority 3: Relative to executable (bin/../)
    std::string exeDir = getExecutableDirectory();
    if (!exeDir.empty()) {
        std::filesystem::path sysrootRelPath = std::filesystem::path(exeDir) / "..";
        if (std::filesystem::exists(sysrootRelPath)) {
            sysrootPath = std::filesystem::canonical(sysrootRelPath).string();
            logger.debug("Sysroot from executable path: " + sysrootPath, "Sysroot");
            
            // Verify this looks like a valid sysroot (has stdlib dir)
            std::filesystem::path stdlibCheck = std::filesystem::path(sysrootPath) / "stdlib" / "aurora";
            if (std::filesystem::exists(stdlibCheck)) {
                sysrootInitialized = true;
                logger.info("Sysroot inferred from executable: " + sysrootPath);
                return;
            }
        }
    }
    
    // Priority 4: Compile-time defined AURORA_SYSROOT
#ifdef AURORA_SYSROOT
    std::string compileSysroot = AURORA_SYSROOT;
    logger.debug("Using compile-time sysroot: " + compileSysroot, "Sysroot");
    if (std::filesystem::exists(compileSysroot)) {
        sysrootPath = std::filesystem::canonical(compileSysroot).string();
        sysrootInitialized = true;
        logger.info("Sysroot from compile-time: " + sysrootPath);
        return;
    }
#endif
    
    // Fallback: current directory (for development)
    logger.warning("No sysroot found, using current directory");
    sysrootPath = std::filesystem::current_path().string();
    sysrootInitialized = true;
}

std::string getSysroot() {
    if (!sysrootInitialized) {
        initializeSysroot();
    }
    return sysrootPath;
}

} // namespace aurora

