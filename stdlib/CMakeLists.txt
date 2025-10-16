cmake_minimum_required(VERSION 3.20)

# Standard library C++ implementation sources
set(AURORA_STDLIB_SOURCES
  src/StdLib.cpp
)

# Create static stdlib library
add_library(aurora_stdlib_static STATIC ${AURORA_STDLIB_SOURCES})
set_target_properties(aurora_stdlib_static PROPERTIES
  OUTPUT_NAME aurora_stdlib
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)
target_include_directories(aurora_stdlib_static PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/runtime/include
)

# Create shared stdlib library
if(BUILD_SHARED_LIBS)
  add_library(aurora_stdlib_shared SHARED ${AURORA_STDLIB_SOURCES})
  set_target_properties(aurora_stdlib_shared PROPERTIES
    OUTPUT_NAME aurora_stdlib
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    VERSION 0.6.3
    SOVERSION 0
  )
  target_include_directories(aurora_stdlib_shared PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/runtime/include
  )
  # Link shared stdlib with shared runtime
  target_link_libraries(aurora_stdlib_shared PUBLIC aurora_runtime_shared)
endif()

# Alias for backward compatibility
add_library(aurora_stdlib ALIAS aurora_stdlib_static)

# Install libraries
if(BUILD_STATIC_LIBS)
  install(TARGETS aurora_stdlib_static
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
  )
endif()

if(BUILD_SHARED_LIBS)
  install(TARGETS aurora_stdlib_shared
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
  )
endif()

# Install native extension headers
install(DIRECTORY include/
  DESTINATION include
  FILES_MATCHING PATTERN "*.h"
)

# Install Aurora stdlib source files (.aur) to sysroot structure
# This follows: sysroot/stdlib/<module>/<file>.aur
install(DIRECTORY aurora/
  DESTINATION lib/aurora/stdlib
  FILES_MATCHING PATTERN "*.aur"
)


