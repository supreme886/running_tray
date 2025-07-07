@echo off
setlocal enabledelayedexpansion

:: Set variables
set PROJECT_NAME=running-tray
set BUILD_DIR=build
set OUTPUT_DIR=release_output
set PLUGIN_DIR=%BUILD_DIR%\plugins\Release
set VERSION=1.0.0

echo Starting build process...
echo Current directory: %CD%

:: Create build directory
echo Creating build directory...
mkdir %BUILD_DIR%
cd %BUILD_DIR%

:: Configure project
echo Configuring project with CMake...
cmake .. -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    exit /b 1
)

:: Build project
echo Building project...
cmake --build . --config Release
if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)

:: Create output directory
cd ..
echo Creating output directory...
mkdir %OUTPUT_DIR%

:: Check if main executable exists
if not exist "%BUILD_DIR%\Release\%PROJECT_NAME%.exe" (
    echo ERROR: Main executable not found at %BUILD_DIR%\Release\%PROJECT_NAME%.exe
    echo Listing build directory contents:
    dir "%BUILD_DIR%\Release\"
    exit /b 1
)

:: Copy main executable
echo Copying main executable...
copy "%BUILD_DIR%\Release\%PROJECT_NAME%.exe" "%OUTPUT_DIR%\"

:: 新增：复制common.dll
echo Copying common library...
if exist "%BUILD_DIR%\common\Release\common.dll" (
    copy "%BUILD_DIR%\common\Release\common.dll" "%OUTPUT_DIR%\"
) else (
    echo ERROR: common.dll not found at %BUILD_DIR%\Release\common.dll
    exit /b 1
)

:: 新增：复制应用图标
echo Copying application icon...
if exist "src\resources\app_icon.ico" (
    copy "src\resources\app_icon.ico" "%OUTPUT_DIR%\"
) else (
    echo ERROR: app_icon.ico not found at src\resources\app_icon.ico
    exit /b 1
)

:: Copy dependency files
echo Running windeployqt...
windeployqt --release --no-translations --no-angle --no-opengl-sw "%OUTPUT_DIR%\%PROJECT_NAME%.exe"

:: Create plugin directory
echo Creating plugin directory...
mkdir "%OUTPUT_DIR%\plugins"

:: Copy plugin
if exist "%PLUGIN_DIR%\runningcatplugin.dll" (
    echo Copying plugin...
    copy "%PLUGIN_DIR%\runningcatplugin.dll" "%OUTPUT_DIR%\plugins\"
) else (
    echo Warning: Plugin not found at %PLUGIN_DIR%\runningcatplugin.dll
    echo Listing plugin directory contents:
    if exist "%PLUGIN_DIR%" (
        dir "%PLUGIN_DIR%\" /b
    ) else (
        echo Plugin directory does not exist: %PLUGIN_DIR%
    )
    echo Searching for runningcatplugin.dll in build directory:
    dir "%BUILD_DIR%" /s /b | findstr /i "runningcatplugin.dll"
)

:: Check for NSIS installation
echo Checking for NSIS...
set NSIS_PATH=
if exist "C:\Program Files (x86)\NSIS\makensis.exe" (
    set "NSIS_PATH=C:\Program Files (x86)\NSIS\makensis.exe"
    echo Found NSIS at: !NSIS_PATH!
) else if exist "C:\Program Files\NSIS\makensis.exe" (
    set "NSIS_PATH=C:\Program Files\NSIS\makensis.exe"
    echo Found NSIS at: !NSIS_PATH!
) else if exist "C:\ProgramData\chocolatey\lib\nsis\tools\makensis.exe" (
    set "NSIS_PATH=C:\ProgramData\chocolatey\lib\nsis\tools\makensis.exe"
    echo Found NSIS at: !NSIS_PATH!
) else (
    echo NSIS not found in standard locations
    echo Searching for makensis.exe...
    where makensis.exe
)

:: Create NSIS installer script
echo Creating NSIS installer script...
echo ^^!define APP_NAME "%PROJECT_NAME%" > installer.nsi
echo ^^!define APP_VERSION "%VERSION%" >> installer.nsi
echo ^^!define APP_PUBLISHER "Your Company" >> installer.nsi
echo ^^!define APP_EXE "%PROJECT_NAME%.exe" >> installer.nsi
echo. >> installer.nsi
echo Name "${APP_NAME}" >> installer.nsi
echo OutFile "%PROJECT_NAME%_setup.exe" >> installer.nsi
echo InstallDir "$PROGRAMFILES\${APP_NAME}" >> installer.nsi
echo RequestExecutionLevel admin >> installer.nsi
echo. >> installer.nsi
echo Icon "%OUTPUT_DIR%\app_icon.ico" >> installer.nsi
echo. >> installer.nsi
echo Page directory >> installer.nsi
echo Page instfiles >> installer.nsi
echo. >> installer.nsi
echo Section "MainSection" SEC01 >> installer.nsi
echo   SetOutPath "$INSTDIR" >> installer.nsi
echo   File /r "%OUTPUT_DIR%\*" >> installer.nsi
echo   CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}" "" "$INSTDIR\app_icon.ico" 0 >> installer.nsi
echo   CreateDirectory "$SMPROGRAMS\${APP_NAME}" >> installer.nsi
echo   CreateShortCut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}" "" "$INSTDIR\app_icon.ico" 0 >> installer.nsi
echo   CreateShortCut "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk" "$INSTDIR\uninstall.exe" >> installer.nsi
echo   WriteUninstaller "$INSTDIR\uninstall.exe" >> installer.nsi
echo SectionEnd >> installer.nsi
echo. >> installer.nsi
echo Section "Uninstall" >> installer.nsi
echo   Delete "$DESKTOP\${APP_NAME}.lnk" >> installer.nsi
echo   RMDir /r "$SMPROGRAMS\${APP_NAME}" >> installer.nsi
echo   RMDir /r "$INSTDIR" >> installer.nsi
echo SectionEnd >> installer.nsi

:: Create installer using NSIS
if defined NSIS_PATH (
    echo Creating installer with NSIS...
    "!NSIS_PATH!" installer.nsi
    if !ERRORLEVEL! equ 0 (
        echo Installer created: %PROJECT_NAME%_setup.exe
        if exist "%PROJECT_NAME%_setup.exe" (
            echo Setup file exists and is ready
        ) else (
            echo ERROR: Setup file was not created
        )
    ) else (
        echo NSIS compilation failed, creating ZIP instead
        goto create_zip
    )
) else (
    echo NSIS not found, creating ZIP instead
    goto create_zip
)

goto cleanup

:create_zip
echo Creating ZIP archive...
powershell Compress-Archive -Path "%OUTPUT_DIR%\*" -DestinationPath "%PROJECT_NAME%_release.zip" -Force
if %ERRORLEVEL% equ 0 (
    echo ZIP created: %PROJECT_NAME%_release.zip
) else (
    echo Failed to create ZIP file
)

:cleanup
:: List final output files
echo Listing final output files:
dir "*.exe" "*.zip"

:: Clean temporary files
echo Cleaning up...
rmdir /s /q %BUILD_DIR%
rmdir /s /q %OUTPUT_DIR%
if exist installer.nsi del installer.nsi

echo Build process completed.
endlocal
