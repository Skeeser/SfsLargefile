cmake_minimum_required(VERSION 3.10.2)
project(monitor_work LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(sfs)
add_subdirectory(test)