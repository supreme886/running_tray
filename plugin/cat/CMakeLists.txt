cmake_minimum_required(VERSION 3.14)

project(runningcatplugin LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

qt_add_library(runningcatplugin MODULE
    runningcatplugin.cpp
    runningcatplugin.h
    res/runningcatplugin.qrc

)

target_link_libraries(runningcatplugin PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    ${CMAKE_BINARY_DIR}/common/common.lib  # 链接common共享库
)

# 添加common目录到头文件搜索路径
target_include_directories(runningcatplugin PRIVATE
    ${CMAKE_SOURCE_DIR}/common
)

# 设置插件输出名为 .so/.dll
set_target_properties(runningcatplugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)
