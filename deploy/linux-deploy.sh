#!/usr/bin/env bash
# VisionFrame Linux 部署打包：把 App/（exe + Bin/*.so + Bin/plugins/）打成 tar.gz。
# 用法：./deploy/linux-deploy.sh [输出目录]   （默认 dist/）
set -euo pipefail

cd "$(dirname "$0")/.."
OUT_DIR="${1:-dist}"
STAGE="$OUT_DIR/VisionFrame"

if [ ! -x App/VisionFrame ]; then
    echo "未找到 App/VisionFrame，请先构建：cmake --preset linux-release && cmake --build --preset linux-release" >&2
    exit 1
fi

rm -rf "$STAGE"
mkdir -p "$STAGE/Bin"

# 可执行 + 共享库 + 插件（保持 App/ 布局，RPATH $ORIGIN 已内嵌，无需 install_name 修补）
cp -v App/VisionFrame "$STAGE/"
find App/Bin -maxdepth 1 -name '*.so*' -exec cp -v {} "$STAGE/Bin/" \;
if [ -d App/Bin/plugins ]; then
    mkdir -p "$STAGE/Bin/plugins"
    find App/Bin/plugins -maxdepth 1 -name '*.so*' -exec cp -v {} "$STAGE/Bin/plugins/" \;
fi
mkdir -p "$STAGE/logs"
cp -v deploy/visionframe-headless.service "$STAGE/" 2>/dev/null || true

TARBALL="$OUT_DIR/visionframe-linux-$(uname -m).tar.gz"
tar -czf "$TARBALL" -C "$OUT_DIR" VisionFrame
echo "打包完成: $TARBALL"
echo "目标机运行: tar -xzf visionframe-linux-$(uname -m).tar.gz && ./VisionFrame/VisionFrame --headless VisionFrame/<工程文件>"
