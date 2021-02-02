set(libhex_SRCS
    hex.h
    hex.cpp
)

# Declare the library
add_library(libhex STATIC
    ${libhex_SRCS}
)

target_link_libraries(libhex)

# Specify here the include directories exported
# by this library
target_include_directories(libhex PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
