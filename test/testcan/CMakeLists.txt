# file(GLOB SRCS *.cpp)
set(SRCS
  main.cpp
  testcrypto.cpp
  testframe.cpp
  testservice.cpp
  testcommunicator.cpp
  )

ADD_EXECUTABLE(testcan ${SRCS})

TARGET_LINK_LIBRARIES(testcan
    libcan
    libgtest
    libgmock
)

add_test(NAME testcan
         COMMAND testcan)
