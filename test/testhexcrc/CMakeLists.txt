file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(testhexcrc ${SRCS})

TARGET_LINK_LIBRARIES(testhexcrc
    libutil
    libhex
    libgtest
    libgmock
)

add_test(NAME testhexcrc
         COMMAND testhexcrc)
