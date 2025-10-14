cmake_minimum_required(VERSION 3.20)

# Runtime sources (C)
set(AURORA_RUNTIME_SOURCES
  src/aurora_runtime.c
)

# Create runtime library
add_library(aurora_runtime STATIC ${AURORA_RUNTIME_SOURCES})

# Include directories for runtime
target_include_directories(aurora_runtime PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# C library doesn't need C++ standard
set_target_properties(aurora_runtime PROPERTIES
  C_STANDARD 11
  C_STANDARD_REQUIRED ON
)


