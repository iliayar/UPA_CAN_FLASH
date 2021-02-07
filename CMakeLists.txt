cmake_minimum_required(VERSION 3.10)

project(can-util)

enable_testing()

add_subdirectory(main)
add_subdirectory(test)
add_subdirectory(libhex)
add_subdirectory(libcan)
