set(libutil_SRCS
    util.h
    util.cpp
    map.h
    bytes.h
    bytes.cpp
)

# Declare the library
add_library(libutil STATIC
    ${libutil_SRCS}
)

set_target_properties(libutil PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(libutil)

# Specify here the include directories exported
# by this library
target_include_directories(libutil PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
