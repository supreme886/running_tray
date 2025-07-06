#!/bin/bash

# DMG打包脚本
set -e  # 遇到错误立即退出

# 配置变量
APP_NAME="running-tray"
APP_DISPLAY_NAME="Running Tray"
DMG_NAME="Running-Tray"
VERSION="${GITHUB_REF_NAME:-1.0.0}"
BUILD_DIR="build"
DMG_DIR="dmg_staging"

# 自动检测Qt路径和macdeployqt
echo "Detecting Qt installation..."

# 尝试多种方式找到macdeployqt
MACDEPLOYQT_PATH=""

# 方法1: 使用which命令
if command -v macdeployqt >/dev/null 2>&1; then
    MACDEPLOYQT_PATH=$(which macdeployqt)
    echo "Found macdeployqt via which: $MACDEPLOYQT_PATH"
fi

# 方法2: 检查常见的Qt安装路径
if [ -z "$MACDEPLOYQT_PATH" ]; then
    QT_PATHS=(
        "/usr/local/opt/qt@6/bin/macdeployqt"
        "/usr/local/opt/qt@5/bin/macdeployqt"
        "/usr/local/Qt/*/macos/bin/macdeployqt"
        "/opt/homebrew/opt/qt@6/bin/macdeployqt"
        "/opt/homebrew/opt/qt@5/bin/macdeployqt"
        "$HOME/Qt/*/macos/bin/macdeployqt"
    )
    
    for path in "${QT_PATHS[@]}"; do
        # 处理通配符路径
        for expanded_path in $path; do
            if [ -f "$expanded_path" ]; then
                MACDEPLOYQT_PATH="$expanded_path"
                echo "Found macdeployqt at: $MACDEPLOYQT_PATH"
                break 2
            fi
        done
    done
fi

# 方法3: 使用环境变量
if [ -z "$MACDEPLOYQT_PATH" ] && [ -n "$Qt6_DIR" ]; then
    if [ -f "$Qt6_DIR/bin/macdeployqt" ]; then
        MACDEPLOYQT_PATH="$Qt6_DIR/bin/macdeployqt"
        echo "Found macdeployqt via Qt6_DIR: $MACDEPLOYQT_PATH"
    fi
fi

# 检查是否找到macdeployqt
if [ -z "$MACDEPLOYQT_PATH" ]; then
    echo "Error: macdeployqt not found!"
    echo "Please install Qt or set Qt6_DIR environment variable"
    echo "You can install Qt via Homebrew: brew install qt@6"
    exit 1
fi

# 从macdeployqt路径推导Qt目录
QT_DIR=$(dirname $(dirname "$MACDEPLOYQT_PATH"))
echo "Using Qt directory: $QT_DIR"
echo "Using macdeployqt: $MACDEPLOYQT_PATH"

# 检测当前系统架构和Qt库架构
SYSTEM_ARCH=$(uname -m)
echo "Current system architecture: $SYSTEM_ARCH"

# 检测Qt库支持的架构
QT_LIB_PATH="$QT_DIR/lib"
if [ -d "$QT_LIB_PATH" ]; then
    # 检查QtCore库支持的架构
    QT_CORE_LIB="$QT_LIB_PATH/QtCore.framework/Versions/*/QtCore"
    for qt_lib in $QT_CORE_LIB; do
        if [ -f "$qt_lib" ]; then
            QT_ARCHS=$(lipo -info "$qt_lib" 2>/dev/null | grep "Architectures" | cut -d: -f2 | xargs)
            echo "Qt library architectures: $QT_ARCHS"
            break
        fi
    done
fi

# 根据系统架构和Qt支持情况决定编译策略
BUILD_UNIVERSAL=false
TARGET_ARCHS=()

# 检查环境变量中指定的架构
if [ -n "$CMAKE_OSX_ARCHITECTURES" ]; then
    echo "Using architectures from environment variable: $CMAKE_OSX_ARCHITECTURES"
    
    # 解析环境变量中的架构
    IFS=';' read -ra ENV_ARCHS <<< "$CMAKE_OSX_ARCHITECTURES"
    
    if [ ${#ENV_ARCHS[@]} -eq 1 ]; then
        # 单一架构
        TARGET_ARCHS=("${ENV_ARCHS[0]}")
        BUILD_UNIVERSAL=false
        echo "Building single architecture from env: ${ENV_ARCHS[0]}"
    elif [ ${#ENV_ARCHS[@]} -eq 2 ]; then
        # 通用二进制
        TARGET_ARCHS=("${ENV_ARCHS[@]}")
        BUILD_UNIVERSAL=true
        echo "Building universal binary from env: ${ENV_ARCHS[*]}"
    fi
else
    # 原有的自动检测逻辑
    if [[ "$QT_ARCHS" == *"x86_64"* ]] && [[ "$QT_ARCHS" == *"arm64"* ]]; then
        echo "Qt supports both architectures, building universal binary"
        BUILD_UNIVERSAL=true
        TARGET_ARCHS=("x86_64" "arm64")
    elif [[ "$QT_ARCHS" == *"$SYSTEM_ARCH"* ]]; then
        echo "Building for current system architecture: $SYSTEM_ARCH"
        TARGET_ARCHS=("$SYSTEM_ARCH")
    else
        echo "Warning: Qt doesn't support current architecture, trying to build anyway"
        TARGET_ARCHS=("$SYSTEM_ARCH")
    fi
fi

echo "Building DMG for $APP_DISPLAY_NAME version $VERSION"
echo "Target architectures: ${TARGET_ARCHS[*]}"

# 清理之前的构建
rm -rf "$BUILD_DIR" "${BUILD_DIR}_x86_64" "${BUILD_DIR}_arm64" "$DMG_DIR" "*.dmg"

if [ "$BUILD_UNIVERSAL" = true ] && [ ${#TARGET_ARCHS[@]} -eq 2 ]; then
    # 构建通用二进制
    echo "Building universal binary..."
    
    # 构建 x86_64 版本
    echo "Building x86_64 version..."
    mkdir -p "${BUILD_DIR}_x86_64"
    cd "${BUILD_DIR}_x86_64"
    cmake .. -DCMAKE_BUILD_TYPE=Release \
             -DCMAKE_PREFIX_PATH="$QT_DIR" \
             -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 \
             -DCMAKE_OSX_ARCHITECTURES="x86_64"
    make -j$(sysctl -n hw.ncpu)
    cd ..
    
    # 检查 x86_64 应用包是否存在
    if [ ! -d "${BUILD_DIR}_x86_64/$APP_NAME.app" ]; then
        echo "Error: x86_64 application bundle not found!"
        exit 1
    fi
    
    # 构建 arm64 版本
    echo "Building arm64 version..."
    mkdir -p "${BUILD_DIR}_arm64"
    cd "${BUILD_DIR}_arm64"
    cmake .. -DCMAKE_BUILD_TYPE=Release \
             -DCMAKE_PREFIX_PATH="$QT_DIR" \
             -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 \
             -DCMAKE_OSX_ARCHITECTURES="arm64"
    make -j$(sysctl -n hw.ncpu)
    cd ..
    
    # 检查 arm64 应用包是否存在
    if [ ! -d "${BUILD_DIR}_arm64/$APP_NAME.app" ]; then
        echo "Error: arm64 application bundle not found!"
        exit 1
    fi
    
    # 创建通用二进制目录
    echo "Creating universal binary..."
    mkdir -p "$BUILD_DIR"
    cp -R "${BUILD_DIR}_arm64/$APP_NAME.app" "$BUILD_DIR/"
    
    # 合并可执行文件
    echo "Merging executables with lipo..."
    lipo -create \
        "${BUILD_DIR}_x86_64/$APP_NAME.app/Contents/MacOS/$APP_NAME" \
        "${BUILD_DIR}_arm64/$APP_NAME.app/Contents/MacOS/$APP_NAME" \
        -output "$BUILD_DIR/$APP_NAME.app/Contents/MacOS/$APP_NAME"
    
    # 验证通用二进制
    echo "Verifying universal binary:"
    lipo -info "$BUILD_DIR/$APP_NAME.app/Contents/MacOS/$APP_NAME"
    
    # 合并插件（如果存在）
    if [ -d "${BUILD_DIR}_x86_64/plugins" ] && [ -d "${BUILD_DIR}_arm64/plugins" ]; then
        echo "Merging plugins..."
        mkdir -p "$BUILD_DIR/plugins"
        
        # 遍历 arm64 插件目录
        for arm64_plugin in "${BUILD_DIR}_arm64/plugins"/*.dylib; do
            if [ -f "$arm64_plugin" ]; then
                plugin_name=$(basename "$arm64_plugin")
                x86_64_plugin="${BUILD_DIR}_x86_64/plugins/$plugin_name"
                
                if [ -f "$x86_64_plugin" ]; then
                    echo "Merging plugin: $plugin_name"
                    lipo -create "$x86_64_plugin" "$arm64_plugin" -output "$BUILD_DIR/plugins/$plugin_name"
                else
                    echo "Warning: $plugin_name not found in x86_64 build, copying arm64 version"
                    cp "$arm64_plugin" "$BUILD_DIR/plugins/"
                fi
            fi
        done
    fi
    
    # 清理临时构建目录
    echo "Cleaning up temporary build directories..."
    rm -rf "${BUILD_DIR}_x86_64" "${BUILD_DIR}_arm64"
    
    DMG_SUFFIX="Universal"
else
    # 构建单一架构
    TARGET_ARCH="${TARGET_ARCHS[0]}"
    echo "Building for single architecture: $TARGET_ARCH"
    
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    cmake .. -DCMAKE_BUILD_TYPE=Release \
             -DCMAKE_PREFIX_PATH="$QT_DIR" \
             -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 \
             -DCMAKE_OSX_ARCHITECTURES="$TARGET_ARCH"
    
    make -j$(sysctl -n hw.ncpu)
    
    # 检查应用包是否存在
    if [ ! -d "$APP_NAME.app" ]; then
        echo "Error: Application bundle $APP_NAME.app not found!"
        exit 1
    fi
    
    DMG_SUFFIX="$TARGET_ARCH"
fi

# 确保我们在包含应用包的正确目录中
echo "Current directory: $(pwd)"
echo "Looking for application bundle: $APP_NAME.app"

if [ ! -d "$APP_NAME.app" ]; then
    echo "Application bundle not found in current directory, checking build directory..."
    if [ -d "$BUILD_DIR/$APP_NAME.app" ]; then
        echo "Found application bundle in $BUILD_DIR, switching to that directory"
        cd "$BUILD_DIR"
    else
        echo "Error: Cannot find application bundle!"
        echo "Current directory contents:"
        ls -la
        echo "Build directory contents:"
        ls -la "$BUILD_DIR" 2>/dev/null || echo "Build directory does not exist"
        exit 1
    fi
fi

echo "Confirmed: Application bundle found at $(pwd)/$APP_NAME.app"

# 手动复制插件到应用包
echo "Copying plugins to app bundle..."
mkdir -p "$APP_NAME.app/Contents/PlugIns"

# 查找并复制所有插件
if [ -d "plugins" ]; then
    echo "Found plugins directory, copying all .dylib files..."
    for plugin in plugins/*.dylib; do
        if [ -f "$plugin" ]; then
            cp "$plugin" "$APP_NAME.app/Contents/PlugIns/"
            echo "Copied $(basename "$plugin") to app bundle"
        fi
    done
else
    echo "Warning: plugins directory not found"
    echo "Current directory contents:"
    ls -la
fi

# 确保图标文件被正确复制到应用包
echo "Ensuring app icon is properly deployed..."
mkdir -p "$APP_NAME.app/Contents/Resources"

# 复制图标文件
if [ -f "../src/resources/app_icon.icns" ]; then
    cp "../src/resources/app_icon.icns" "$APP_NAME.app/Contents/Resources/"
    echo "Copied app_icon.icns to app bundle"
else
    echo "Warning: app_icon.icns not found in src/resources/"
fi

# 验证 Info.plist 中的图标设置
if [ -f "$APP_NAME.app/Contents/Info.plist" ]; then
    echo "Verifying Info.plist icon configuration..."
    /usr/libexec/PlistBuddy -c "Print :CFBundleIconFile" "$APP_NAME.app/Contents/Info.plist" 2>/dev/null || echo "CFBundleIconFile not set in Info.plist"
fi

# 验证插件是否复制成功
echo "Verifying plugins in app bundle:"
ls -la "$APP_NAME.app/Contents/PlugIns/" || echo "PlugIns directory is empty or doesn't exist"

# 验证图标文件是否存在
echo "Verifying icon file in app bundle:"
ls -la "$APP_NAME.app/Contents/Resources/app_icon.icns" || echo "Icon file not found in app bundle"

# 使用macdeployqt部署依赖
echo "Deploying Qt dependencies..."
"$MACDEPLOYQT_PATH" "$APP_NAME.app" -verbose=2

# 创建DMG暂存目录
echo "Preparing DMG staging area..."
cd ..
mkdir -p "$DMG_DIR"
cp -R "$BUILD_DIR/$APP_NAME.app" "$DMG_DIR/"

# 创建Applications文件夹的符号链接
ln -s /Applications "$DMG_DIR/Applications"

# 直接从文件夹创建DMG
echo "Creating DMG directly from folder..."
FINAL_DMG="$DMG_NAME-$VERSION-$DMG_SUFFIX.dmg"
hdiutil create -volname "$APP_DISPLAY_NAME" -srcfolder "$DMG_DIR" -ov -format UDZO "$FINAL_DMG"

# 清理临时文件
rm -rf "$DMG_DIR"

echo "DMG created successfully: $FINAL_DMG"
echo "DMG size: $(du -h "$FINAL_DMG" | cut -f1)"

# 验证DMG
echo "Verifying DMG..."
hdiutil verify "$FINAL_DMG"

echo "Build complete!"