project(myfs)

set(HEADER_FILES
        my.h
        )

set(SOURCE_FILES
        my.cpp
        )

add_library(myfs STATIC ${SOURCE_FILES} ${HEADER_FILES})
