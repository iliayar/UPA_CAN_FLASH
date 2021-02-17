cmake_minimum_required(VERSION 3.10)

project(can-util C CXX)

# enable_testing()

set(APP_VERSION "0.6.6.5")

add_subdirectory(main)
add_subdirectory(libhex)
add_subdirectory(libcan)
add_subdirectory(libutil)
# add_subdirectory(test)
