cmake_minimum_required(VERSION 3.10)

project(can-util C CXX)

enable_testing()

add_subdirectory(main)
add_subdirectory(libhex)
add_subdirectory(libcan)
add_subdirectory(libutil)
add_subdirectory(test)