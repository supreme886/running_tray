@echo off
setlocal enabledelayedexpansion

:: Set variables
set PROJECT_NAME=running-tray
set BUILD_DIR=build
set OUTPUT_DIR=release_output
set PLUGIN_DIR=plugin\cat\Release
set VERSION=1.0.0

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

:: Create NSIS installer script
echo ; NSIS Installer Script > installer.nsi
echo !define APP_NAME "%PROJECT_NAME%" >> installer.nsi
echo !define APP_VERSION "%VERSION%" >> installer.nsi
echo !define APP_PUBLISHER "Your Company" >> installer.nsi
echo !define APP_EXE "%PROJECT_NAME%.exe" >> installer.nsi
echo. >> installer.nsi
echo OutFile "%PROJECT_NAME%_setup.exe" >> installer.nsi
echo InstallDir "$PROGRAMFILES\%PROJECT_NAME%" >> installer.nsi
echo. >> installer.nsi
echo Section "MainSection" SEC01 >> installer.nsi
echo   SetOutPath "$INSTDIR" >> installer.nsi
echo   File /r "%OUTPUT_DIR%\*" >> installer.nsi
echo   CreateShortCut "$DESKTOP\%PROJECT_NAME%.lnk" "$INSTDIR\%PROJECT_NAME%.exe" >> installer.nsi
echo   CreateDirectory "$SMPROGRAMS\%PROJECT_NAME%" >> installer.nsi
echo   CreateShortCut "$SMPROGRAMS\%PROJECT_NAME%\%PROJECT_NAME%.lnk" "$INSTDIR\%PROJECT_NAME%.exe" >> installer.nsi
echo   CreateShortCut "$SMPROGRAMS\%PROJECT_NAME%\Uninstall.lnk" "$INSTDIR\uninstall.exe" >> installer.nsi
echo   WriteUninstaller "$INSTDIR\uninstall.exe" >> installer.nsi
echo SectionEnd >> installer.nsi
echo. >> installer.nsi
echo Section "Uninstall" >> installer.nsi
echo   Delete "$DESKTOP\%PROJECT_NAME%.lnk" >> installer.nsi
echo   RMDir /r "$SMPROGRAMS\%PROJECT_NAME%" >> installer.nsi
echo   RMDir /r "$INSTDIR" >> installer.nsi
echo SectionEnd >> installer.nsi

:: Create installer using NSIS
if exist "C:\Program Files (x86)\NSIS\makensis.exe" (
    "C:\Program Files (x86)\NSIS\makensis.exe" installer.nsi
    echo Installer created: %PROJECT_NAME%_setup.exe
) else (
    echo NSIS not found, creating ZIP instead
    powershell Compress-Archive -Path %OUTPUT_DIR%\* -DestinationPath %PROJECT_NAME%_release.zip
    echo ZIP created: %PROJECT_NAME%_release.zip
)

:: Clean temporary files
rmdir /s /q %BUILD_DIR%
rmdir /s /q %OUTPUT_DIR%
if exist installer.nsi del installer.nsi

endlocal