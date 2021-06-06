cmake_minimum_required(VERSION 3.19)

project(FileSystem_test)

include(FetchContent)
FetchContent_Declare(
                googletest
                GIT_REPOSITORY https://github.com/google/googletest.git
                GIT_TAG	master
                )

FetchContent_MakeAvailable(googletest)

add_executable(FileSystem_test test.cpp)
target_link_libraries(FileSystem_test gtest gtest_main)

include_directories(${myfs_SOURCE_DIR})
target_link_libraries(FileSystem_test gtest myfs)
