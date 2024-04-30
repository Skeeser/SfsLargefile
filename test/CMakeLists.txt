

set(SOURCES
    alltest_main.cpp
    mmap_file_test.cpp
    file_op_test.cpp
    mmap_file_op_test.cpp
    block_init_test.cpp
)

add_executable(test ${SOURCES})

target_link_libraries(test
    PUBLIC
    sfs
)


find_package(GTest CONFIG REQUIRED)
target_link_libraries(test PUBLIC GTest::gtest GTest::gtest_main ${LIB_FUC} )