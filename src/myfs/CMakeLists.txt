project(myfs)

set(HEADER_FILES
        my.h
        syscall.h
        )

set(SOURCE_FILES
        my.cpp
        syscall.cpp
        )

add_library(myfs STATIC ${SOURCE_FILES} ${HEADER_FILES})
