cmake_minimum_required(VERSION 3.16)

project(running-tray VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

include_directories(${CMAKE_SOURCE_DIR}/common)

set(TS_FILES ts/running-tray_zh_CN.ts)

# # 添加common共享库
# add_library(common SHARED
#     common/sharedmenumanager.cpp
#     common/sharedmenumanager.h
# )

# # 定义编译宏，用于导出符号
# target_compile_definitions(common PRIVATE COMMON_LIBRARY)

# # 链接Qt库
# target_link_libraries(common PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# 找到现有的 PROJECT_SOURCES 定义，添加资源文件
set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/mainwindow.ui
        src/plugincardwidget.cpp
        src/plugincardwidget.h
        src/pluginmanager.cpp
        src/pluginmanager.h
        src/resources.qrc  # 添加这一行
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Core)

    qt_add_executable(running-tray
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        src/pluginmanager.h
        src/pluginmanager.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET running-tray APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(running-tray SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        # 在现有的可执行文件配置中添加MACOSX_BUNDLE
        if(APPLE)
            add_executable(running-tray MACOSX_BUNDLE
                ${PROJECT_SOURCES}
            )
            
            # 设置Bundle信息
            set_target_properties(running-tray PROPERTIES
                MACOSX_BUNDLE_GUI_IDENTIFIER com.yourcompany.running-tray
                MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
                MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
                MACOSX_BUNDLE_BUNDLE_NAME "Running Tray"
                MACOSX_BUNDLE_DISPLAY_NAME "Running Tray"
                MACOSX_BUNDLE_ICON_FILE app_icon.icns
            )
        else()
            add_executable(running-tray
                ${PROJECT_SOURCES}
            )
        endif()
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

# 链接common库到主程序
target_link_libraries(running-tray PRIVATE Qt${QT_VERSION_MAJOR}::Widgets common)
target_link_libraries(running-tray PRIVATE Qt${QT_VERSION_MAJOR}::Core)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.running-tray)
endif()
set_target_properties(running-tray PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS running-tray
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(running-tray)
endif()

# 添加平台特定编译选项
if(WIN32)
    target_compile_definitions(running-tray PRIVATE PLATFORM_WINDOWS)
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(running-tray PRIVATE PLATFORM_LINUX)
elseif(APPLE)
    target_compile_definitions(running-tray PRIVATE PLATFORM_MACOS)
endif()

add_subdirectory(common)
add_subdirectory(plugin)

# 在文件末尾添加插件安装规则
if(APPLE)
    # 在macOS上，将插件安装到.app/Contents/PlugIns/
    install(TARGETS runningcatplugin
        LIBRARY DESTINATION running-tray.app/Contents/PlugIns
    )
else()
    # 其他平台安装到plugins目录
    install(TARGETS runningcatplugin
        LIBRARY DESTINATION plugins
    )
endif()
