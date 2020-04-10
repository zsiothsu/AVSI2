cmake_minimum_required(VERSION 3.0.0)
project(InterpreterTest VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(InterpreterTest main.cpp)

add_subdirectory(./src)

target_link_libraries(${PROJECT_NAME} Interpreter)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
