cmake_minimum_required(VERSION 3.20)

project(converter
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(converter
    src/main.cc
    src/ast/ASTPrinter.cc
    src/ast/LiteralFormat.cc
    src/converter/CodeGenerator.cc
    src/lexer/Lexer.cc
    src/parser/Parser.cc
)

target_include_directories(converter
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(MSVC)
    target_compile_options(converter
        PRIVATE
            /W4
            /permissive-
    )
else()
    target_compile_options(converter
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )
endif()

include(CTest)

if(BUILD_TESTING)
    find_package(Python3 COMPONENTS Interpreter QUIET)

    if(Python3_Interpreter_FOUND)
        add_test(NAME converter_tests
            COMMAND
                ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_tests.py
                $<TARGET_FILE:converter>
        )
    else()
        message(STATUS "Python3 not found; converter tests will not be registered")
    endif()
endif()
