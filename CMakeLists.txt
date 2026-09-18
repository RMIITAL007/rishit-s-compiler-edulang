cmake_minimum_required(VERSION 3.20)

project(CompilerLab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(compiler_frontend
    src/ast/printer.cpp
    src/frontend/diagnostic.cpp
    src/frontend/frontend.cpp
    src/frontend/lexer.cpp
    src/frontend/parser.cpp
)

target_include_directories(compiler_frontend
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(compiler_frontend PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(compiler_frontend PRIVATE /W4 /permissive-)
else()
    target_compile_options(compiler_frontend PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(compiler src/main.cpp)
target_link_libraries(compiler PRIVATE compiler_frontend)

include(CTest)
if(BUILD_TESTING)
    add_executable(compiler_tests
        tests/test_main.cpp
    )
    target_link_libraries(compiler_tests PRIVATE compiler_frontend)
    add_test(NAME compiler_tests COMMAND compiler_tests)
endif()
