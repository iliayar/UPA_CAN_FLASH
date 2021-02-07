file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(testutil ${SRCS})

TARGET_LINK_LIBRARIES(testutil
    libutil
    libgtest
    libgmock
)

add_test(NAME testutil
         COMMAND testutil)
