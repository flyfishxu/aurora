cmake_minimum_required(VERSION 3.20)

# Runtime sources (C)
set(AURORA_RUNTIME_SOURCES
  src/aurora_runtime.c
)

# Option to build shared or static libraries (default: both)
option(BUILD_SHARED_LIBS "Build shared libraries (.so/.dylib/.dll)" ON)
option(BUILD_STATIC_LIBS "Build static libraries (.a/.lib)" ON)

# Create static runtime library (always available for compiler)
add_library(aurora_runtime_static STATIC ${AURORA_RUNTIME_SOURCES})
set_target_properties(aurora_runtime_static PROPERTIES
  OUTPUT_NAME aurora_runtime
  C_STANDARD 11
  C_STANDARD_REQUIRED ON
)
target_include_directories(aurora_runtime_static PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Create shared runtime library (for dynamic linking)
if(BUILD_SHARED_LIBS)
  add_library(aurora_runtime_shared SHARED ${AURORA_RUNTIME_SOURCES})
  set_target_properties(aurora_runtime_shared PROPERTIES
    OUTPUT_NAME aurora_runtime
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    VERSION 0.6.3
    SOVERSION 0
  )
  target_include_directories(aurora_runtime_shared PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  )
endif()

# Alias for backward compatibility
add_library(aurora_runtime ALIAS aurora_runtime_static)

# Install libraries
if(BUILD_STATIC_LIBS)
  install(TARGETS aurora_runtime_static
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
  )
endif()

if(BUILD_SHARED_LIBS)
  install(TARGETS aurora_runtime_shared
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
  )
endif()

# Install headers
install(DIRECTORY include/
  DESTINATION include
  FILES_MATCHING PATTERN "*.h"
)


