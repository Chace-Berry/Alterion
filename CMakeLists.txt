cmake_minimum_required(VERSION 3.15)
project(Alterion VERSION 2.6.0 LANGUAGES CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configuration options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_TOOLS "Build development tools" ON)
option(BUILD_EXAMPLES "Build examples" ON)

# Include directories
include_directories(include)
include_directories(${CMAKE_SOURCE_DIR}/core/include)

# Runtime system
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/runtime/CMakeLists.txt")
    add_subdirectory(runtime)
endif()

# Standard library
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/stdlib/CMakeLists.txt")
    add_subdirectory(stdlib)
endif()

# Development tools
if(BUILD_TOOLS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tools/CMakeLists.txt")
    add_subdirectory(tools)
endif()

# Tests
if(BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    enable_testing()
    add_subdirectory(tests)
endif()

# Examples
if(BUILD_EXAMPLES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt")
    add_subdirectory(examples)
endif()

# Main Alterion compiler executable
set(ALTERION_SOURCES
    core/lexer/lexer.cpp
    core/lexer/token.cpp
    core/parser/parser.cpp
)

# Check if additional source files exist
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/core/semantic/semantic_analysis.cpp")
    list(APPEND ALTERION_SOURCES core/semantic/semantic_analysis.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/core/codegen/codegen.cpp")
    list(APPEND ALTERION_SOURCES core/codegen/codegen.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tools/cli/main.cpp")
    list(APPEND ALTERION_SOURCES tools/cli/main.cpp)
endif()

add_executable(alterion ${ALTERION_SOURCES})

# Lexer unit test executable
add_executable(lexertest
    tests/unit/lexertest.cpp
    core/lexer/lexer.cpp
    core/lexer/token.cpp
)
target_include_directories(lexertest PRIVATE ${CMAKE_SOURCE_DIR}/core/include)

# Optionally add to test suite
if(BUILD_TESTS)
    add_test(NAME LexerTest COMMAND lexertest)
endif()

# Installation
install(TARGETS alterion DESTINATION bin)
install(DIRECTORY include/ DESTINATION include/alterion FILES_MATCHING PATTERN "*.h")
install(DIRECTORY documentation/ DESTINATION share/doc/alterion FILES_MATCHING PATTERN "*.md")
