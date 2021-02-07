set(Main_SRCS
    main.cpp
    mainwindow.h
    mainwindow.cpp
)


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets Core SerialBus REQUIRED)

set(CMAKE_CXX_FLAGS "-g -pthread")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COMPILE_OPTS -Wall -Wextra -pedantic -pedantic-errors)

add_executable(myApp
    ${Main_SRCS}
)

# Specify here the libraries this program depends on
target_link_libraries(myApp
    libcan
    Qt5::Widgets
    Qt5::SerialBus
)

install(TARGETS myApp DESTINATION bin)
