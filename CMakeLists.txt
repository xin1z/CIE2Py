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
    src/lexer/Lexer.cc
)

target_include_directories(converter
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(converter
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)
