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

echo "Building DMG for $APP_DISPLAY_NAME version $VERSION"

# 清理之前的构建
rm -rf "$BUILD_DIR" "$DMG_DIR" "*.dmg"

# 创建构建目录
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# 配置和构建项目
echo "Configuring project..."
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_PREFIX_PATH="$QT_DIR" \
         -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15

echo "Building project..."
make -j$(sysctl -n hw.ncpu)

# 检查应用包是否存在
if [ ! -d "$APP_NAME.app" ]; then
    echo "Error: Application bundle $APP_NAME.app not found!"
    exit 1
fi

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

# 创建临时DMG
echo "Creating temporary DMG..."
TEMP_DMG="temp_$DMG_NAME.dmg"
hdiutil create -size 200m -fs HFS+ -volname "$APP_DISPLAY_NAME" "$TEMP_DMG"

# 挂载临时DMG
echo "Mounting temporary DMG..."
DEVICE=$(hdiutil attach -readwrite -noverify "$TEMP_DMG" | egrep '^/dev/' | sed 1q | awk '{print $1}')
VOLUME_PATH="/Volumes/$APP_DISPLAY_NAME"

# 等待挂载完成
sleep 2

# 复制文件到DMG
echo "Copying files to DMG..."
cp -R "$DMG_DIR/$APP_NAME.app" "$VOLUME_PATH/"
cp -R "$DMG_DIR/Applications" "$VOLUME_PATH/"

# 设置DMG外观（可选）
echo "Configuring DMG appearance..."
osascript << EOF
tell application "Finder"
    tell disk "$APP_DISPLAY_NAME"
        open
        set current view of container window to icon view
        set toolbar visible of container window to false
        set statusbar visible of container window to false
        set the bounds of container window to {400, 100, 900, 400}
        set viewOptions to the icon view options of container window
        set arrangement of viewOptions to not arranged
        set icon size of viewOptions to 72
        set position of item "$APP_NAME.app" of container window to {150, 200}
        set position of item "Applications" of container window to {350, 200}
        close
        open
        update without registering applications
        delay 2
    end tell
end tell
EOF

# 卸载DMG
echo "Unmounting DMG..."
hdiutil detach "$DEVICE"

# 转换为最终的只读DMG
echo "Creating final DMG..."
FINAL_DMG="$DMG_NAME-$VERSION.dmg"
hdiutil convert "$TEMP_DMG" -format UDZO -imagekey zlib-level=9 -o "$FINAL_DMG"

# 清理临时文件
rm "$TEMP_DMG"
rm -rf "$DMG_DIR"

echo "DMG created successfully: $FINAL_DMG"
echo "DMG size: $(du -h "$FINAL_DMG" | cut -f1)"

# 验证DMG
echo "Verifying DMG..."
hdiutil verify "$FINAL_DMG"

echo "Build complete!"