@echo off
setlocal enabledelayedexpansion

:: Set variables
set PROJECT_NAME=running-tray
set BUILD_DIR=build
set OUTPUT_DIR=release_output
set PLUGIN_DIR=plugin\cat\Release

:: Create build directory
mkdir %BUILD_DIR%
cd %BUILD_DIR%

:: Configure project
cmake .. -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release

:: Build project
cmake --build . --config Release

:: Create output directory
cd ..
mkdir %OUTPUT_DIR%

:: Copy main executable
copy %BUILD_DIR%\Release\%PROJECT_NAME%.exe %OUTPUT_DIR%\

:: Copy dependency files
windeployqt --release --no-translations --no-angle --no-opengl-sw %OUTPUT_DIR%\%PROJECT_NAME%.exe

:: Create plugin directory
mkdir %OUTPUT_DIR%\plugins

:: Copy plugin
copy %PLUGIN_DIR%\runningcatplugin.dll %OUTPUT_DIR%\plugins\

:: Compress output directory
powershell Compress-Archive -Path %OUTPUT_DIR%\* -DestinationPath %PROJECT_NAME%_release.zip

:: Clean temporary files
rmdir /s /q %BUILD_DIR%
rmdir /s /q %OUTPUT_DIR%

echo Packaging completed: %PROJECT_NAME%_release.zip
endlocal