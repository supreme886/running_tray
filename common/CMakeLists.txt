cmake_minimum_required(VERSION 3.14)

project(common LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Network)

SET(COMMON_SOURCES
    interface/itrayloadplugin.h
    interface/itrayloadplugin.cpp
    sharedmenumanager.h
    sharedmenumanager.cpp
    autostartmanager.h
    autostartmanager.cpp
    flowlayout.h
    flowlayout.cpp
    networkmanager.h
    networkmanager.cpp
    thememanager.cpp
    thememanager.h
    configmanager.h
    configmanager.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(common SHARED
        ${COMMON_SOURCES}
    )
else()
    add_library(common SHARED
        ${COMMON_SOURCES}
    )
endif()

target_compile_definitions(common PRIVATE COMMON_LIBRARY)

target_link_libraries(common PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)

target_include_directories(common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 设置插件输出名为 .so/.dll
# set_target_properties(common PROPERTIES
#     LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
# )



