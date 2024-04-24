cmake_minimum_required(VERSION 3.26)
project(tuple_utility)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CXX_STANDARD C++20)

if(MSVC)
   add_compile_options(/std:c++20)
endif()

add_executable(tuple_utility main.cpp)
