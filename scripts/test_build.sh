#!/bin/bash

# 本地测试构建脚本
set -e

echo "Testing local DMG build..."

# 设置测试环境变量
export GITHUB_REF_NAME="test-1.0.0"

# 运行构建脚本
./scripts/build_dmg.sh

echo "Local build test completed!"
echo "Generated files:"
ls -la *.dmg 2>/dev/null || echo "No DMG files found"