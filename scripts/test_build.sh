#!/bin/bash

set -e

echo "Detecting platform..."

case "$(uname -s)" in
    Linux*)
        echo "Linux system detected"
        ./scripts/build_deb.sh
        ls -la *.deb 2>/dev/null || echo "No DEB packages found"
        ;;
    Darwin*)
        echo "macOS system detected"
        ./scripts/build_dmg.sh
        ls -la *.dmg 2>/dev/null || echo "No DMG files found"
        ;;
    CYGWIN*|MINGW32*|MSYS*)
        echo "Windows system detected"
        cmd.exe /c scripts\package.bat
        ls -la dist/*.exe 2>/dev/null || echo "No EXE files found"
        ;;
    *)
        echo "Unsupported platform: $(uname -s)"
        exit 1
        ;;
esac

echo "Build artifacts:"
find . -type f \( -name "*.deb" -o -name "*.dmg" -o -name "*.exe" \) 2>/dev/null || echo "No build artifacts found"