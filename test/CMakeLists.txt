

set(SOURCES
    mmap_file_test.cpp
    alltest_main.cpp
)

add_executable(test ${SOURCES})

target_link_libraries(test
    PUBLIC
    sfs
)


find_package(GTest CONFIG REQUIRED)
target_link_libraries(test PUBLIC GTest::gtest GTest::gtest_main ${LIB_FUC} )