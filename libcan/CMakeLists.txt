file(GLOB libcan_SRCS src/*.cpp)

include_directories(include)

# Declare the library
add_library(libcan STATIC
    ${libcan_SRCS}
)

IF(WIN32)
target_link_libraries(libcan -static)
endif()

target_link_libraries(libcan
   libhex
   libutil
)

# Specify here the include directories exported
# by this library
target_include_directories(libcan PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
