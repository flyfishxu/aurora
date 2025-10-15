cmake_minimum_required(VERSION 3.20)

# Aurora LSP Server executable
project(aurora-lsp)

# Find nlohmann/json library
find_package(nlohmann_json QUIET)

if(NOT nlohmann_json_FOUND)
    # If not found, use FetchContent to download it
    include(FetchContent)
    FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
    )
    FetchContent_MakeAvailable(json)
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/compiler/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Source files
set(LSP_SOURCES
    src/main.cpp
    src/LSPServer.cpp
    src/LSPHandlers.cpp
    src/LSPProtocol.cpp
)

# Create executable
add_executable(aurora-lsp ${LSP_SOURCES})

# Link libraries
target_link_libraries(aurora-lsp
    aurora_compiler
    aurora_runtime
    nlohmann_json::nlohmann_json
    ${LLVM_LIBS}
)

# Install target
install(TARGETS aurora-lsp DESTINATION bin)

message(STATUS "Aurora LSP Server configured")

