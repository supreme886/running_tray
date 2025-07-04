cmake_minimum_required(VERSION 3.14)

project(common LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_add_library(common SHARED
    interface/itrayloadplugin.h
    interface/itrayloadplugin.cpp
    sharedmenumanager.h
    sharedmenumanager.cpp
)

target_compile_definitions(common PRIVATE COMMON_LIBRARY)

target_link_libraries(common PRIVATE
    Qt6::Core
    Qt6::Widgets
)

target_include_directories(common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 设置插件输出名为 .so/.dll
# set_target_properties(common PROPERTIES
#     LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
# )



