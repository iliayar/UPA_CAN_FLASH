cmake_minimum_required(VERSION 3.10)

project(can-util C CXX)

if(CMAKE_BUILD_TYPE EQUAL "Debug")
enable_testing()
endif()

add_subdirectory(main)
add_subdirectory(libhex)
add_subdirectory(libcan)
add_subdirectory(libutil)
if(CMAKE_BUILD_TYPE EQUAL "Debug")
add_subdirectory(test)
endif()