# VisionFrame Linux（边缘侧）构建 + 无头冒烟验证环境
# 用法：
#   docker build -t visionframe-build -f deploy/Dockerfile .
#   docker run --rm visionframe-build          # 构建并跑冒烟测试
#   docker run --rm -v $(pwd)/dist:/out visionframe-build pack   # 顺便产出部署包
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake ninja-build \
        qtbase5-dev qtbase5-dev-tools \
        libopencv-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . /src

# Linux 构建（Ninja；系统 Qt5/OpenCV 由 find_package 发现）
RUN cmake --preset linux-release && cmake --build --preset linux-release --parallel "$(nproc)"

# 无头冒烟：图像源 -> 图像保存 全链路（deploy/linux-smoke.sh）
RUN chmod +x deploy/*.sh && ./deploy/linux-smoke.sh

# 可选：./deploy/linux-deploy.sh 产出 dist/visionframe-linux-<arch>.tar.gz（挂载 /out 取走）
CMD []
