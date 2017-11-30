cmake_minimum_required(VERSION 3.6)
project(my_json_paser)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES JsonPaser.cpp JsonPaser.h test.cpp)
add_executable(my_json_paser ${SOURCE_FILES})
