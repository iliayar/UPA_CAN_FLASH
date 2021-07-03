find_package(Qt5 COMPONENTS Widgets Core SerialBus Network SerialPort Test REQUIRED)

include_directories(include)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

file(GLOB libqcan_SRCS src/*.cpp)
file(GLOB libqcan_HEADERS include/*.h)
file(GLOB libqcan_configuration_SRCS src/configure_task/*.cpp)
file(GLOB libqcan_configuration_HEADERS include/configure_task/*.h)

# Declare the library
add_library(libqcan STATIC
    ${libqcan_SRCS}
    ${libqcan_HEADERS}
    ${libqcan_configuration_SRCS}
    ${libqcan_configuration_HEADERS}
)

# IF(WIN32)
# target_link_libraries(libqcan -static)
# endif()

target_link_libraries(libqcan
  libhex
  libutil
  libcan
  Qt5::Core
  Qt5::Widgets
  # Qt5::SerialBus
  Qt5::Test
  )

# Specify here the include directories exported
# by this library
target_include_directories(libqcan PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
