cmake_minimum_required(VERSION 3.20)
project(CompilerLab VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

add_test(NAME compiler_symbols_scopes
    COMMAND ${CMAKE_COMMAND}
        -DCOMPILER=$<TARGET_FILE:compiler>
        -DSOURCE_FILE=${CMAKE_SOURCE_DIR}/tests/symbols_scopes.edu
        -P ${CMAKE_SOURCE_DIR}/tests/verify_symbols_output.cmake)

add_test(NAME compiler_benchmark_valid
    COMMAND compiler bench ${CMAKE_SOURCE_DIR}/examples/hello.edu)

add_test(NAME compiler_benchmark_runtime_error
    COMMAND ${CMAKE_COMMAND}
        -DCOMPILER=$<TARGET_FILE:compiler>
        -DSOURCE_FILE=${CMAKE_SOURCE_DIR}/examples/errors/runtime_error.edu
        "-DEXPECTED_DIAGNOSTIC=division by zero"
        -P ${CMAKE_SOURCE_DIR}/tests/verify_benchmark_failure.cmake)

add_test(NAME compiler_benchmark_syntax_error
    COMMAND ${CMAKE_COMMAND}
        -DCOMPILER=$<TARGET_FILE:compiler>
        -DSOURCE_FILE=${CMAKE_SOURCE_DIR}/examples/errors/syntax_error.edu
        "-DEXPECTED_DIAGNOSTIC=expected a number"
        -P ${CMAKE_SOURCE_DIR}/tests/verify_benchmark_failure.cmake)

add_test(NAME compiler_benchmark_semantic_error
    COMMAND ${CMAKE_COMMAND}
        -DCOMPILER=$<TARGET_FILE:compiler>
        -DSOURCE_FILE=${CMAKE_SOURCE_DIR}/examples/errors/semantic_error.edu
        "-DEXPECTED_DIAGNOSTIC=undeclared variable"
        -P ${CMAKE_SOURCE_DIR}/tests/verify_benchmark_failure.cmake)
