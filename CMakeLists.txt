cmake_minimum_required(VERSION 3.19)

project(FileSystem)

set(CMAKE_CXX_STANDARD 17)
#remove_definitions(-Werror)

add_subdirectory(src)
add_subdirectory(test)
