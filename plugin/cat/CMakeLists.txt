cmake_minimum_required(VERSION 3.14)

project(runningcatplugin LANGUAGES CXX)

# 使用与主程序相同的Qt版本
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Gui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(runningcatplugin SHARED
        runningcatplugin.cpp
        runningcatplugin.h
        runningcatplugin.json
        runningcatplugin.qrc
    )
else()
    add_library(runningcatplugin SHARED
        runningcatplugin.cpp
        runningcatplugin.h
        runningcatplugin.json
        runningcatplugin.qrc
    )
endif()

target_link_libraries(runningcatplugin PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    common
)

if(WIN32)
    target_link_libraries(runningcatplugin PRIVATE ${CMAKE_BINARY_DIR}/common/common.lib)
elseif(APPLE)
    target_link_libraries(runningcatplugin PRIVATE ${CMAKE_BINARY_DIR}/common/libcommon.dylib)
else()
    target_link_libraries(runningcatplugin PRIVATE ${CMAKE_BINARY_DIR}/common/libcommon.so)
endif()

target_include_directories(runningcatplugin PRIVATE
    ${CMAKE_SOURCE_DIR}/common
)

set_target_properties(runningcatplugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)
