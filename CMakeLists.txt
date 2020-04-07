#the file is main makelists for the whole project

#specify minimum version of cmake
cmake_minimum_required(VERSION 3.5)

#project name
project(Interpreter)

SET(CMAKE_BUILD_TYPE "Debug")

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")

#specify executable file name
add_executable(${PROJECT_NAME} ./main.cpp ./Token.cpp ./Exception.cpp ./Interpreter.cpp)


