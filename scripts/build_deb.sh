#!/bin/bash

# DEB打包脚本
set -e  # 遇到错误立即退出

# 配置变量
APP_NAME="running-tray"
APP_DISPLAY_NAME="Running Tray"
VERSION="${GITHUB_REF_NAME:-1.0.0}"
VERSION_NUMBER="${VERSION#v}"  # 移除v前缀
BUILD_DIR="build"
DEB_DIR="deb_staging"
PACKAGE_DIR="$DEB_DIR/DEBIAN"
USR_DIR="$DEB_DIR/usr"
BIN_DIR="$USR_DIR/bin"
SHARE_DIR="$USR_DIR/share"
APP_DIR="$SHARE_DIR/applications"
ICON_DIR="$SHARE_DIR/pixmaps"

echo "Building DEB for $APP_DISPLAY_NAME version $VERSION_NUMBER"

# 清理之前的构建
rm -rf "$BUILD_DIR" "$DEB_DIR" "*.deb"

# 构建应用
echo "Building application..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_INSTALL_PREFIX=/usr

make -j$(nproc)

# 检查可执行文件是否存在
if [ ! -f "$APP_NAME" ]; then
    echo "Error: Executable $APP_NAME not found!"
    exit 1
fi

echo "Application built successfully"

# 创建DEB包目录结构
echo "Creating DEB package structure..."
cd ..
mkdir -p "$PACKAGE_DIR" "$BIN_DIR" "$APP_DIR" "$ICON_DIR"

# 复制可执行文件
cp "$BUILD_DIR/$APP_NAME" "$BIN_DIR/"
chmod +x "$BIN_DIR/$APP_NAME"

# 复制图标
if [ -f "src/resources/app_icon.ico" ]; then
    # 如果有PNG版本更好，如果没有就转换ico
    if command -v convert >/dev/null 2>&1; then
        convert "src/resources/app_icon.ico" "$ICON_DIR/$APP_NAME.png" 2>/dev/null || \
        cp "src/resources/app_icon.ico" "$ICON_DIR/$APP_NAME.ico"
    else
        cp "src/resources/app_icon.ico" "$ICON_DIR/$APP_NAME.ico"
    fi
fi

# 创建desktop文件
cat > "$APP_DIR/$APP_NAME.desktop" << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=$APP_DISPLAY_NAME
Comment=System tray application with plugin support
Exec=$APP_NAME
Icon=$APP_NAME
Terminal=false
Categories=Utility;System;
StartupNotify=true
EOF

# 创建control文件
INSTALLED_SIZE=$(du -sk "$USR_DIR" | cut -f1)

cat > "$PACKAGE_DIR/control" << EOF
Package: $APP_NAME
Version: $VERSION_NUMBER
Section: utils
Priority: optional
Architecture: amd64
Depends: libc6, libqt6core6, libqt6gui6, libqt6widgets6
Maintainer: Running Tray Team <noreply@example.com>
Description: $APP_DISPLAY_NAME
 A system tray application with plugin support for monitoring
 and managing system resources.
Installed-Size: $INSTALLED_SIZE
EOF

# 创建postinst脚本（安装后脚本）
cat > "$PACKAGE_DIR/postinst" << 'EOF'
#!/bin/bash
set -e

# 更新desktop数据库
if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database /usr/share/applications
fi

# 更新图标缓存
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -f -t /usr/share/pixmaps 2>/dev/null || true
fi

exit 0
EOF

# 创建prerm脚本（卸载前脚本）
cat > "$PACKAGE_DIR/prerm" << 'EOF'
#!/bin/bash
set -e

# 停止可能正在运行的应用
pkill -f running-tray || true

exit 0
EOF

# 设置脚本权限
chmod 755 "$PACKAGE_DIR/postinst" "$PACKAGE_DIR/prerm"

# 构建DEB包
echo "Building DEB package..."
FINAL_DEB="${APP_NAME}_${VERSION_NUMBER}_amd64.deb"

# 使用dpkg-deb构建包
dpkg-deb --build "$DEB_DIR" "$FINAL_DEB"

# 清理临时文件
rm -rf "$DEB_DIR"

echo "DEB package created successfully: $FINAL_DEB"
echo "Package size: $(du -h "$FINAL_DEB" | cut -f1)"

# 验证DEB包
echo "Verifying DEB package..."
dpkg-deb --info "$FINAL_DEB"
dpkg-deb --contents "$FINAL_DEB"

echo "Build complete!"
EOF