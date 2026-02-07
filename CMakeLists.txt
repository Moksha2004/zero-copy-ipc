cmake_minimum_required(VERSION 3.15)
project(zpc_ipc VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-O3 -march=native -Wall -pthread)
include_directories(include)

add_executable(writer src/writer.cpp)
target_link_libraries(writer rt pthread)

add_executable(reader src/reader.cpp)
target_link_libraries(reader rt pthread)
