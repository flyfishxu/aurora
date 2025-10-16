cmake_minimum_required(VERSION 3.20)
project(AuroraLang VERSION 0.6.3 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find LLVM package
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM include dirs: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM library dirs: ${LLVM_LIBRARY_DIRS}")

# Include LLVM directories and definitions
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Determine native target components for LLVM
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
  set(NATIVE_TARGET "AArch64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
  set(NATIVE_TARGET "X86")
else()
  message(FATAL_ERROR "Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# Map LLVM components to libraries
llvm_map_components_to_libnames(LLVM_LIBS
  OrcJIT
  ${NATIVE_TARGET}CodeGen
  ${NATIVE_TARGET}AsmParser
)

# Remove any duplicates
list(REMOVE_DUPLICATES LLVM_LIBS)

message(STATUS "Building AuroraLang with modular architecture:")
message(STATUS "  - Compiler Layer (C++): ${CMAKE_SOURCE_DIR}/compiler")
message(STATUS "  - Runtime Layer (C):    ${CMAKE_SOURCE_DIR}/runtime")
message(STATUS "  - Stdlib Layer (C++):   ${CMAKE_SOURCE_DIR}/stdlib")
message(STATUS "  - Tools:                ${CMAKE_SOURCE_DIR}/tools")

# Add subdirectories for each layer
add_subdirectory(runtime)      # Runtime layer (C)
add_subdirectory(compiler)     # Compiler layer (C++)
add_subdirectory(stdlib)       # Standard library layer (C++)
add_subdirectory(tools/aurora-cli)  # Aurora CLI tool
add_subdirectory(tools/aurora-lsp)  # Aurora LSP Server

# Create portable distribution directory
set(DIST_DIR "${CMAKE_BINARY_DIR}/../dist")
add_custom_target(dist
    COMMAND ${CMAKE_COMMAND} -E echo "Creating portable distribution in ${DIST_DIR}"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${DIST_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}/bin"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}/lib"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}/include"
    
    # Copy executable
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:aurora>" "${DIST_DIR}/bin/"
    
    # Copy libraries (static and shared)
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:aurora_runtime_static>" "${DIST_DIR}/lib/"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:aurora_stdlib_static>" "${DIST_DIR}/lib/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:aurora_runtime_shared>" "${DIST_DIR}/lib/" || true
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:aurora_stdlib_shared>" "${DIST_DIR}/lib/" || true
    
    # Copy stdlib sources
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/stdlib/aurora" "${DIST_DIR}/stdlib/aurora"
    
    # Copy headers
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/runtime/include" "${DIST_DIR}/include"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/stdlib/include" "${DIST_DIR}/include"
    
    COMMAND ${CMAKE_COMMAND} -E echo "✓ Distribution created at: ${DIST_DIR}"
    
    DEPENDS aurora aurora_runtime_static aurora_stdlib_static
    COMMENT "Creating portable AuroraLang distribution"
)

# Summary message
message(STATUS "")
message(STATUS "=== AuroraLang Build Configuration ===")
message(STATUS "Architecture:      ${CMAKE_SYSTEM_PROCESSOR} (${NATIVE_TARGET})")
message(STATUS "LLVM Version:      ${LLVM_PACKAGE_VERSION}")
message(STATUS "Build Type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard:      C++${CMAKE_CXX_STANDARD}")
message(STATUS "")
message(STATUS "Components:")
message(STATUS "  ✓ Runtime Layer")
message(STATUS "  ✓ Compiler Layer")
message(STATUS "  ✓ Standard Library Layer")
message(STATUS "  ✓ Aurora CLI")
message(STATUS "  ✓ Aurora LSP Server")
message(STATUS "")
message(STATUS "Output:")
message(STATUS "  Executable: ${CMAKE_BINARY_DIR}/aurora")
message(STATUS "  Distribution: Run 'make dist' to create portable package")
message(STATUS "")

