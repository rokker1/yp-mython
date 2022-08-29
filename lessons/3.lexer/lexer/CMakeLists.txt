cmake_minimum_required(VERSION 3.10)

project(Lexer CXX)
set(CMAKE_CXX_STANDARD 20)

set(RUNTIME_DIR ../../4.runtime/runtime)
set(INTERPRETER_DIR ../../5.interpreter)
set(LEXER_FILES lexer.h main.cpp lexer.cpp lexer_test_open.cpp)
set(RUNTIME_FILES 
    ${RUNTIME_DIR}/runtime.h 
    ${RUNTIME_DIR}/runtime.cpp
    ${RUNTIME_DIR}/main.cpp 
    ${RUNTIME_DIR}/test_runner_p.h
    ${RUNTIME_DIR}/runtime_test.cpp)
set(INTERPRETER_FILES 
    ${INTERPRETER_DIR}/lexer.h 
    ${INTERPRETER_DIR}/main.cpp 
    ${INTERPRETER_DIR}/lexer.cpp 
    ${INTERPRETER_DIR}/lexer_test_open.cpp
    ${INTERPRETER_DIR}/runtime.h 
    ${INTERPRETER_DIR}/runtime.cpp
    ${INTERPRETER_DIR}/test_runner_p.h
    ${INTERPRETER_DIR}/runtime_test.cpp
    ${INTERPRETER_DIR}/parse.h
    ${INTERPRETER_DIR}/parse.cpp
    ${INTERPRETER_DIR}/parse_test.cpp
    ${INTERPRETER_DIR}/statement.h
    ${INTERPRETER_DIR}/statement.cpp
    ${INTERPRETER_DIR}/statement_test.cpp)

add_executable(lexer ${LEXER_FILES})
add_executable(runtime ${RUNTIME_FILES})
add_executable(interpreter ${INTERPRETER_FILES})
