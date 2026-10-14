#!/usr/bin/env bash
# VisionFrame Linux 无头冒烟测试：图像源 -> 图像保存 全链路验证。
# 用法：先构建（cmake --preset linux-release && cmake --build --preset linux-release），再执行本脚本。
set -euo pipefail

cd "$(dirname "$0")/.."
APP=App/VisionFrame
if [ ! -x "$APP" ]; then
    echo "未找到 $APP，请先构建：cmake --preset linux-release && cmake --build --preset linux-release" >&2
    exit 1
fi

IN=/tmp/vf_smoke_in.png
OUT=/tmp/vf_smoke_out.png
rm -f "$OUT"
# 1x1 PNG（base64）
printf 'iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAEhQGAhKmMIQAAAABJRU5ErkJggg==' | base64 -d > "$IN"

# 无头运行（offscreen 兜底；headless 用 QCoreApplication，通常不加载平台插件）
QT_QPA_PLATFORM=offscreen "$APP" --headless deploy/smoke.vfproj &
PID=$!

# 最多等 10 秒让任务链落盘输出
for _ in $(seq 1 50); do
    [ -f "$OUT" ] && break
    sleep 0.2
done

# 优雅停止（SIGINT）
kill -INT "$PID" 2>/dev/null || true
wait "$PID" 2>/dev/null || true

if [ -f "$OUT" ]; then
    echo "SMOKE OK: $OUT generated"
else
    echo "SMOKE FAILED: $OUT not generated，请查看 App/Bin/logs/ 日志" >&2
    exit 1
fi
