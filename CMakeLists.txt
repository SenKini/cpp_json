cmake_minimum_required(VERSION 3.5)
project(cpp_json)

set(CMAKE_CXX_STANDARD 20)
aux_source_directory(./ SRC)

add_executable(cpp_json ${SRC})
# add_executable(cpp_json main.cpp)