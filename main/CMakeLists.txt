set(Main_SRCS
    main.cpp
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets Core SerialBus REQUIRED)

add_executable(myApp
    ${Main_SRCS}
)

# Specify here the libraries this program depends on
target_link_libraries(myApp
    libhex
    Qt5::Widgets
    Qt5::SerialBus
)

install(TARGETS myApp DESTINATION bin)
