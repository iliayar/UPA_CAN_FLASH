file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(testcan ${SRCS})

TARGET_LINK_LIBRARIES(testcan
    libcan
    libgtest
    libgmock
)

add_test(NAME testcan
         COMMAND testcan)
