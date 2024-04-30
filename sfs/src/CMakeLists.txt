set(SOURCES
    mmap_file.cpp
    file_op.cpp
    mmap_file_op.cpp
    index_handle.cpp
)

add_library(sfs ${SOURCES})

target_include_directories(sfs PUBLIC
  ${PROJECT_SOURCE_DIR}/sfs/include
)

