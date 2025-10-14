#pragma once

namespace aurora {

// Setup crash handler to display stack trace on segfault
void setupCrashHandler();

// Verify LLVM module for errors
bool verifyModule(void* module, bool abortOnError = false);

} // namespace aurora

