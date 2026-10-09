cmake_minimum_required(VERSION 3.20)
project(CompilerLab VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

include_directories(${CMAKE_SOURCE_DIR}/include)

set(COMMON_SOURCES
    src/frontend/frontend.cpp
    src/frontend/lexer.cpp
    src/frontend/parser.cpp
    src/frontend/diagnostic.cpp
    src/frontend/semantic_analyzer.cpp
    src/frontend/symbol_table.cpp
    src/ast/printer.cpp
    src/ir/ir.cpp
    src/ir/generator.cpp
    src/ir/optimizer.cpp
    src/runtime/interpreter.cpp
    src/target/code_generator.cpp
)

add_executable(compiler
    src/main.cpp
    ${COMMON_SOURCES}
)

add_executable(compiler_tests
    tests/test_main.cpp
    ${COMMON_SOURCES}
)

add_test(NAME compiler_tests COMMAND compiler_tests)
