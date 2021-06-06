project(myfs_fuse)

set(HEADER_FILES
        myfs_fuse.h
        )

set(SOURCE_FILES
        myfs_fuse.cpp
        )

set(CMAKE_CXX_FLAGS "-lfuse3 -lpthread")

include_directories(${myfs_SOURCE_DIR})

add_library(myfs_fuse STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(myfs_fuse myfs)
