add_compile_definitions(APP_VERSION="${APP_VERSION}")

find_package(Qt5 COMPONENTS Widgets Core SerialBus Network SerialPort Test REQUIRED)

include_directories(include)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

file(GLOB Main_SRCS src/*.cpp)
file(GLOB MainConfigure_SRCS src/configure_task/*.cpp)
file(GLOB Main_HEADERS include/*.h)
file(GLOB MainConfigure_HEADERS include/configure_task/*.h)

if(WIN32)
  add_executable(UPA_CAN_FLASH WIN32
    ${Main_SRCS}
    ${MainConfigure_SRCS}
    ${Main_HEADERS}
    ${MainConfigure_HEADERS}
    )
else()
  add_executable(UPA_CAN_FLASH
    ${Main_SRCS}
    ${MainConfigure_SRCS}
    ${Main_HEADERS}
    ${MainConfigure_HEADERS}
    )
endif()

if(WIN32)
  get_filename_component(MINGW_DLL_DIR ${CMAKE_CXX_COMPILER} PATH)
  set(MINGW_DLLS 
    ${MINGW_DLL_DIR}/libstdc++-6.dll
    ${MINGW_DLL_DIR}/libwinpthread-1.dll
    ${MINGW_DLL_DIR}/libharfbuzz-0.dll
    ${MINGW_DLL_DIR}/libpng16-16.dll
    ${MINGW_DLL_DIR}/libdouble-conversion.dll
    ${MINGW_DLL_DIR}/libpcre2-16-0.dll
    ${MINGW_DLL_DIR}/zlib1.dll
    ${MINGW_DLL_DIR}/libpcre2-16-0.dll
    ${MINGW_DLL_DIR}/libzstd.dll
    ${MINGW_DLL_DIR}/libfreetype-6.dll
    ${MINGW_DLL_DIR}/libglib-2.0-0.dll
    ${MINGW_DLL_DIR}/libgraphite2.dll
    ${MINGW_DLL_DIR}/libbrotlidec.dll
    ${MINGW_DLL_DIR}/libbz2-1.dll
    ${MINGW_DLL_DIR}/libintl-8.dll
    ${MINGW_DLL_DIR}/libpcre-1.dll
    ${MINGW_DLL_DIR}/libbrotlicommon.dll
    ${MINGW_DLL_DIR}/libiconv-2.dll
    ${MINGW_DLL_DIR}/libpcre2-16-0.dll
    ${MINGW_DLL_DIR}/libicuuc69.dll
    ${MINGW_DLL_DIR}/libicuin69.dll 
    ${MINGW_DLL_DIR}/libicudt69.dll
    ${MINGW_DLL_DIR}/libmd4c.dll)

  if ($ENV{MSYSTEM} STREQUAL "MINGW32")
    set(MINGW_DLLS ${MINGW_DLLS} ${MINGW_DLL_DIR}/libgcc_s_dw2-1.dll)
  else ($ENV{MSYSTEM} STREQUAL "MINGW32")
    set(MINGW_DLLS ${MINGW_DLLS} ${MINGW_DLL_DIR}/libgcc_s_seh-1.dll)
  endif ($ENV{MSYSTEM} STREQUAL "MINGW32")
  add_custom_command(
    TARGET UPA_CAN_FLASH POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${MINGW_DLLS}
    $<TARGET_FILE_DIR:UPA_CAN_FLASH>
    )

  target_link_libraries(UPA_CAN_FLASH -static -static-libstdc++ -static-libgcc)

  get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
  get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
  find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

  add_custom_command(TARGET UPA_CAN_FLASH POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E
    env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXECUTABLE}"
    "$<TARGET_FILE:UPA_CAN_FLASH>"
    COMMENT "Running windeployqt..."
    )
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread -lstdc++fs")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COMPILE_OPTS -Wall -Wextra -pedantic -pedantic-errors)

# Specify here the libraries this program depends on
target_link_libraries(UPA_CAN_FLASH
  libhex
  libutil
  libcan
  libqcan
  stdc++fs
  Qt5::Core
  Qt5::Widgets
  Qt5::SerialBus
  Qt5::Test
  )


IF(WIN32)
  file(GLOB ROOT_DLLS ${CMAKE_BINARY_DIR}/main/*.dll)
  file(GLOB ROOT_CONFIGS ${CMAKE_BINARY_DIR}/main/*.json)
  install(
    FILES ${CMAKE_BINARY_DIR}/main/UPA_CAN_FLASH.exe
    DESTINATION .
    )
  install(
    FILES ${ROOT_DLLS}
    DESTINATION .
    )
  install(
    FILES ${ROOT_CONFIGS}
    DESTINATION .
    )
  install(
    DIRECTORY ${CMAKE_BINARY_DIR}/main/translations
    DESTINATION .
    )
  install(
    DIRECTORY ${CMAKE_BINARY_DIR}/main/styles
    DESTINATION .
    )
  install(
    DIRECTORY ${CMAKE_BINARY_DIR}/main/platforms
    DESTINATION .
    )
  install(
    DIRECTORY ${CMAKE_BINARY_DIR}/main/imageformats
    DESTINATION .
    )
  install(
    DIRECTORY ${CMAKE_BINARY_DIR}/main/iconengines
    DESTINATION .
    )
  install(
    DIRECTORY ${CMAKE_BINARY_DIR}/main/canbus
    DESTINATION .
    )
  install(
    DIRECTORY ${CMAKE_BINARY_DIR}/main/bearer
    DESTINATION .
    )


  # CPACK macros below here
  #
  set (CPACK_PACKAGE_NAME "UPA_CAN_FLASH")
  set (CPACK_PACKAGE_VERSION ${APP_VERSION})
  set (CPACK_PACKAGE_INSTALL_DIRECTORY "UPA_CAN_FLASH")

  set(CPACK_NSIS_CREATE_ICONS_EXTRA
    "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\UPA_CAN_FLASH.lnk' '$INSTDIR\\\\UPA_CAN_FLASH.exe'"
    )

  # SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS 
  #   "CreateDirectory \\\"\$ENV{APPDATA}\\\\UPA_CAN_FLASH\\\"")

  # Must be after the last CPACK macros
  include(CPack)

endif()
