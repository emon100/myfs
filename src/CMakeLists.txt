cmake_minimum_required(VERSION 3.19)

project(FileSystem_main)


add_subdirectory(myfs)
include_directories(myfs)

add_executable(FileSystem_main FILESYS.h main.cpp)
target_link_libraries(FileSystem_main myfs)
