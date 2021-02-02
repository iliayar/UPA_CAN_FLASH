file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(testhex ${SRCS})

TARGET_LINK_LIBRARIES(testhex
    libgtest
    libgmock
)

add_test(NAME testhex
         COMMAND testhex)
