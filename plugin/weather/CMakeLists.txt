cmake_minimum_required(VERSION 3.14)

project(weatherplugin LANGUAGES CXX)

# 使用与主程序相同的Qt版本
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Gui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(weatherplugin SHARED
	weatherplugin.h
	weatherplugin.cpp
	weatherplugin.json
	weatherplugin.qrc
    )
else()
    add_library(weatherplugin SHARED
   	weatherplugin.h
	weatherplugin.cpp
	weatherplugin.json
	weatherplugin.qrc
    )
endif()

target_link_libraries(weatherplugin PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    common
    rlottie
)

# 移除这些手动指定的库路径，让CMake自动处理
# if(WIN32)
#     target_link_libraries(weatherplugin PRIVATE ${CMAKE_BINARY_DIR}/common/common.lib)
# elseif(APPLE)
#     target_link_libraries(weatherplugin PRIVATE ${CMAKE_BINARY_DIR}/common/libcommon.dylib)
# else()
#     target_link_libraries(weatherplugin PRIVATE ${CMAKE_BINARY_DIR}/common/libcommon.so)
# endif()

target_include_directories(weatherplugin PRIVATE
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/extern/rlottie/inc
)

add_dependencies(weatherplugin common rlottie)

set_target_properties(weatherplugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)
