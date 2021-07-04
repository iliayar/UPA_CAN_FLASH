find_package(Qt5 COMPONENTS Widgets Core SerialBus Network SerialPort Test REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB SRCS *.cpp)
file(GLOB HEADERS *.h)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

ADD_EXECUTABLE(testqcan ${SRCS})

TARGET_LINK_LIBRARIES(testqcan
  libhex
  libutil
  libqcan
  libcan
  Qt5::Core
  Qt5::Widgets
  Qt5::SerialBus
  Qt5::Test
  libgtest
  libgmock
  )

add_test(NAME testqcan
  COMMAND testqcan)
