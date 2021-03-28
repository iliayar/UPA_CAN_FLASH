set(Main_SRCS
    main.cpp
    mainwindow.h
    mainwindow.cpp
    qtask.h
    qtask.cpp
    qlogger.h
    qlogger.cpp
    flash.h
    flash.cpp
    qcommunicator.h
    qcommunicator.cpp
    configure_task/task.h
    configure_task/fields.h
    configure_task/task.cpp
)

add_compile_definitions(APP_VERSION="${APP_VERSION}")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets Core SerialBus Network SerialPort Test REQUIRED)

if(WIN32)
add_executable(UPA_CAN_FLASH WIN32
    ${Main_SRCS}
)
else()
add_executable(UPA_CAN_FLASH
    ${Main_SRCS}
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
            ${MINGW_DLL_DIR}/libicuin68.dll
            ${MINGW_DLL_DIR}/libicuuc68.dll
            ${MINGW_DLL_DIR}/libpcre2-16-0.dll
            ${MINGW_DLL_DIR}/libzstd.dll
            ${MINGW_DLL_DIR}/libfreetype-6.dll
            ${MINGW_DLL_DIR}/libglib-2.0-0.dll
            ${MINGW_DLL_DIR}/libgraphite2.dll
            ${MINGW_DLL_DIR}/libicudt68.dll
            ${MINGW_DLL_DIR}/libbrotlidec.dll
            ${MINGW_DLL_DIR}/libbz2-1.dll
            ${MINGW_DLL_DIR}/libintl-8.dll
            ${MINGW_DLL_DIR}/libpcre-1.dll
            ${MINGW_DLL_DIR}/libbrotlicommon.dll
            ${MINGW_DLL_DIR}/libiconv-2.dll
            ${MINGW_DLL_DIR}/libpcre2-16-0.dll)
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
    stdc++fs
    Qt5::Widgets
    Qt5::SerialBus
    Qt5::Test
)


IF(WIN32)
include(InstallRequiredSystemLibraries)
install(
    DIRECTORY ${CMAKE_BINARY_DIR}/main/
    DESTINATION bin
    COMPONENT applications
)

# CPACK macros below here
#
set (CPACK_PACKAGE_NAME "UPA_CAN_FLASH")
set (CPACK_PACKAGE_VENDOR "Some cool vendor")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "UPA_CAN_FLASH - flash something")
set (CPACK_PACKAGE_VERSION ${APP_VERSION})
set (CPACK_PACKAGE_VERSION_MAJOR "0")
set (CPACK_PACKAGE_VERSION_MINOR "6")
set (CPACK_PACKAGE_VERSION_PATCH "6")
set (CPACK_PACKAGE_INSTALL_DIRECTORY "UPA_CAN_FLASH")

# Define components and their display names
set (CPACK_COMPONENTS_ALL applications libraries headers)
set (CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "Main application")
set (CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "Libraries")
set (CPACK_COMPONENT_HEADERS_DISPLAY_NAME "C++ Headers")

# Human readable component descriptions
set (CPACK_COMPONENT_APPLICATIONS_DESCRIPTION
  "Replace this text with something describing flash")
set (CPACK_COMPONENT_LIBRARIES_DESCRIPTION
  "Have no shared Libraries")
set (CPACK_COMPONENT_HEADERS_DESCRIPTION
  "Have no header files")

# Define dependencies between components
set (CPACK_COMPONENT_HEADERS_DEPENDS libraries)

# Define groups
set(CPACK_COMPONENT_APPLICATIONS_GROUP "Runtime")
set(CPACK_COMPONENT_LIBRARIES_GROUP "Development")
set(CPACK_COMPONENT_HEADERS_GROUP "Development")

set(CPACK_COMPONENT_GROUP_DEVELOPMENT_DESCRIPTION
  "All of the tools you'll ever need to develop software")

# Define NSIS installation types
set(CPACK_ALL_INSTALL_TYPES Full Developer)
set(CPACK_COMPONENT_LIBRARIES_INSTALL_TYPES Developer Full)
set(CPACK_COMPONENT_HEADERS_INSTALL_TYPES Developer Full)
set(CPACK_COMPONENT_APPLICATIONS_INSTALL_TYPES Full)

set(CPACK_NSIS_CREATE_ICONS_EXTRA
  "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\UPA_CAN_FLASH.lnk' '$INSTDIR\\\\bin\\\\UPA_CAN_FLASH.exe'"
  )

# SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS 
#   "CreateDirectory \\\"\$ENV{APPDATA}\\\\UPA_CAN_FLASH\\\"")

# Must be after the last CPACK macros
include(CPack)

endif()
