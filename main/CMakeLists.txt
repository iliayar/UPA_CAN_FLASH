set(Main_SRCS
    main.cpp
    mainwindow.h
    mainwindow.cpp
    can.h
    can.cpp
    flash.h
    flash.cpp
)


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets Core SerialBus Network SerialPort Test REQUIRED)

if(WIN32)
add_executable(canFlash WIN32
    ${Main_SRCS}
)
else()
add_executable(canFlash
    ${Main_SRCS}
)
endif()

if(WIN32)
#    set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -lwsock32 -lws2_32 ${CMAKE_CXX_STANDARD_LIBRARIES}")
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive")
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic")
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
#    target_link_libraries(canFlash -static gcc stdc++ winpthread -dynamic)
#    set_target_properties(canFlash PROPERTIES COMPILE_DEFINITIONS BUILDER_STATIC_DEFINE)

    get_filename_component(MINGW_DLL_DIR ${CMAKE_CXX_COMPILER} PATH)
#    get_target_property(QtCore_LOCATION Qt5::Core LOCATION)
#    get_filename_component(QT_DLL_DIR ${QtCore_LOCATION} PATH)
    add_custom_command(
        TARGET canFlash POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${MINGW_DLL_DIR}/libstdc++-6.dll
            ${MINGW_DLL_DIR}/libwinpthread-1.dll
            ${MINGW_DLL_DIR}/libharfbuzz-0.dll
            ${MINGW_DLL_DIR}/libpng16-16.dll
            ${MINGW_DLL_DIR}/libdouble-conversion.dll
            ${MINGW_DLL_DIR}/libpcre2-16-0.dll
            ${MINGW_DLL_DIR}/zlib1.dll
            ${MINGW_DLL_DIR}/libgcc_s_seh-1.dll
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
            ${MINGW_DLL_DIR}/libpcre2-16-0.dll
#            ${QT_DLL_DIR}/Qt5Widgets.dll
#            ${QT_DLL_DIR}/Qt5Gui.dll
#            ${QT_DLL_DIR}/Qt5Network.dll
#            ${QT_DLL_DIR}/Qt5SerialPort.dll
#            ${QT_DLL_DIR}/Qt5SerialBus.dll
#            ${QT_DLL_DIR}/Qt5Core.dll
            $<TARGET_FILE_DIR:canFlash>
    )

 target_link_libraries(canFlash -static -static-libstdc++ -static-libgcc)
# set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
# target_compile_options(canFlash PRIVATE /MT)
# target_link_options(canFlash PRIVATE /INCREMENTAL:NO /NODEFAULTLIB:MSVCRT)



get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

add_custom_command(TARGET canFlash POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E
        env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXECUTABLE}"
            "$<TARGET_FILE:canFlash>"
    COMMENT "Running windeployqt..."
)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COMPILE_OPTS -Wall -Wextra -pedantic -pedantic-errors)

# Specify here the libraries this program depends on
target_link_libraries(canFlash
    libhex
    libutil
    libcan
    Qt5::Widgets
    Qt5::SerialBus
    Qt5::Test
)


install(TARGETS canFlash DESTINATION bin)
