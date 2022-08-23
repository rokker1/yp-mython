cmake_minimum_required(VERSION 3.10)

project(Lexer CXX)
set(CMAKE_CXX_STANDARD 20)

set(LEXER_FILES lexer.h main.cpp lexer.cpp lexer_test_open.cpp)

add_executable(lexer ${LEXER_FILES})