# Timer Plugin CMakeLists.txt

# 设置插件名称
set(PLUGIN_NAME "timerplugin")

# 设置源文件
set(SOURCES
    timerplugin.cpp
)

set(HEADERS
    timerplugin.h
)

set(RESOURCES
    timerplugin.qrc
)

# 创建插件
add_library(${PLUGIN_NAME} SHARED
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
    ${PLUGIN_NAME}.json
)

# 链接必要的库
target_link_libraries(${PLUGIN_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    common
)

target_include_directories(runningcatplugin PRIVATE
    ${CMAKE_SOURCE_DIR}/common
)

# 确保插件依赖于 common 库的构建
add_dependencies(runningcatplugin common)

# 设置插件属性
set_target_properties(${PLUGIN_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

# 安装规则
install(TARGETS ${PLUGIN_NAME}
    DESTINATION plugins
)

install(FILES ${PLUGIN_NAME}.json
    DESTINATION plugins
)
