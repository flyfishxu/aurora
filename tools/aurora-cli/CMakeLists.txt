cmake_minimum_required(VERSION 3.20)

# Main aurora executable
add_executable(aurora src/main.cpp)

# Link all libraries
target_link_libraries(aurora PRIVATE
  aurora_compiler
  aurora_runtime
  aurora_stdlib
  ${LLVM_LIBS}
)

# Include directories
target_include_directories(aurora PRIVATE
  ${CMAKE_SOURCE_DIR}/compiler/include
  ${CMAKE_SOURCE_DIR}/runtime/include
  ${CMAKE_SOURCE_DIR}/stdlib/include
)

# macOS rpath setup
if(APPLE)
  set_target_properties(aurora PROPERTIES
    BUILD_RPATH "${LLVM_LIBRARY_DIRS}"
    INSTALL_RPATH "${LLVM_LIBRARY_DIRS}"
  )
endif()

# Installation
install(TARGETS aurora DESTINATION bin)


