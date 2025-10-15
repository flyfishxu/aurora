cmake_minimum_required(VERSION 3.20)

# Compiler sources (C++)
set(AURORA_COMPILER_SOURCES
  src/types/Type.cpp
  src/lexer/Lexer.cpp
  src/parser/Parser.cpp
    src/ast/AST.cpp
    src/ast/ModuleLoader.cpp
    src/ast/ImplicitConstructor.cpp
    src/codegen/CodeGen.cpp
  src/codegen/ExprCodeGen.cpp
  src/codegen/StmtCodeGen.cpp
  src/codegen/ClassCodeGen.cpp
  src/codegen/ArrayCodeGen.cpp
  src/diagnostic/Diagnostic.cpp
  src/logger/Logger.cpp
  src/utils/CrashHandler.cpp
  src/core/LanguageCore.cpp
)

# Create compiler library
add_library(aurora_compiler STATIC ${AURORA_COMPILER_SOURCES})

# Include directories for compiler
target_include_directories(aurora_compiler PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/runtime/include
)

# Link LLVM libraries to compiler
target_link_libraries(aurora_compiler PUBLIC ${LLVM_LIBS})
