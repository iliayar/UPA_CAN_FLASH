cmake_minimum_required(VERSION 3.10)

project(can-util C CXX)


add_subdirectory(main)
add_subdirectory(libhex)
add_subdirectory(libcan)
add_subdirectory(libutil)

if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
enable_testing()
add_subdirectory(test)
endif()