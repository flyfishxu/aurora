cmake_minimum_required(VERSION 3.20)

# Standard library C++ implementation sources
set(AURORA_STDLIB_SOURCES
  src/StdLib.cpp
)

# Create stdlib library
add_library(aurora_stdlib STATIC ${AURORA_STDLIB_SOURCES})

# Include directories for stdlib
target_include_directories(aurora_stdlib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/runtime/include
)

# Standard library doesn't directly depend on LLVM
set_target_properties(aurora_stdlib PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)

# Install Aurora stdlib source files
install(DIRECTORY aurora/
  DESTINATION share/aurora/stdlib
  FILES_MATCHING PATTERN "*.aur"
)


