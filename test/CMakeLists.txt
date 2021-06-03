cmake_minimum_required(VERSION 3.19)

project(FileSystem_test)

include(FetchContent)
FetchContent_Declare(
                googletest
                GIT_REPOSITORY https://github.com/google/googletest.git
                GIT_TAG	release-1.10.0
                )

FetchContent_MakeAvailable(googletest)

add_executable(FileSystem_test test.cpp)
target_link_libraries(FileSystem_test gtest gtest_main)

include_directories(${FileSystem_SOURCE_DIR}/src/myfs)
target_link_libraries(FileSystem_test gtest myfs)
