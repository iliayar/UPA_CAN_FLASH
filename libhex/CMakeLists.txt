set(libhex_SRCS
    hex.h
    hex.cpp
)

# Declare the library
add_library(libhex STATIC
    ${libhex_SRCS}
)

IF(WIN32)
target_link_libraries(libhex -static)
endif()

target_link_libraries(libhex
	libutil
	libcan
)

# Specify here the include directories exported
# by this library
target_include_directories(libhex PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
