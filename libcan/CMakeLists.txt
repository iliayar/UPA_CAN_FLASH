set(libcan_SRCS
    frame.h
    frame_virt.h
    frame.cpp
    communicator.h
    communicator.cpp
    service.h
    service.cpp
    task.h
    task.cpp
)

# Declare the library
add_library(libcan STATIC
    ${libcan_SRCS}
)

target_link_libraries(libcan
   libutil
)

# Specify here the include directories exported
# by this library
target_include_directories(libcan PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
