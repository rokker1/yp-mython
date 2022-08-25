cmake_minimum_required(VERSION 3.10)

project(Lexer CXX)
set(CMAKE_CXX_STANDARD 20)

set(RUNTIME_DIR ../../4.runtime/runtime)
set(LEXER_FILES lexer.h main.cpp lexer.cpp lexer_test_open.cpp)
set(RUNTIME_FILES 
    ${RUNTIME_DIR}/runtime.h 
    ${RUNTIME_DIR}/runtime.cpp
    ${RUNTIME_DIR}/main.cpp 
    ${RUNTIME_DIR}/test_runner_p.h
    ${RUNTIME_DIR}/runtime_test.cpp)

add_executable(lexer ${LEXER_FILES})
add_executable(runtime ${RUNTIME_FILES})