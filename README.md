# VisionFrame

VisionFrame 是一个基于 **Qt5 + OpenCV** 的可视化视觉任务框架。它以「蓝图」方式组织图像处理流程：从工具面板拖入工具节点（图像源、灰度、模糊、模板匹配、仿射变换等），在画布中连线构成数据流图，每个任务（Task）在独立线程中按拓扑顺序执行，运行中的节点会高亮显示。

## 功能特性

- 工具面板 + 蓝图编辑器 + 图像显示三栏式界面，页面可随意隐藏/合并
- 工具节点通过引脚连线构成数据流图，按拓扑顺序执行
- 多任务（Task）管理，每个任务独立线程运行，支持运行态高亮（执行中/成功/失败）
- 内置工具：图像源、灰度、模糊、模板匹配（支持 ROI 训练模板）、仿射变换（XYZR）
- 项目保存/加载（`.vfProj`，JSON 格式，预留二进制切换）

## 目录结构

```
VisionFrame/
├── CMakeLists.txt              # 顶层构建脚本（含 Qt/OpenCV 依赖路径）
├── CMakePresets.json           # CMake 预设（Release / Debug，VS 生成器）
├── App/                        # 构建产物输出目录
│   ├── VisionFrame.exe         # 主程序
│   ├── FrameImageView.dll      # 图像显示 DLL（含 Q_OBJECT，需随 exe 放置）
│   ├── Qt5Core.dll 等          # Qt 运行期 DLL（构建后自动拷贝）
│   └── Bin/                    # 业务 DLL 输出（延迟加载）
│       ├── FrameToolBase.dll
│       ├── FrameCameraBase.dll
│       └── ...
├── Include/
│   ├── include/                # 对外公共头文件（构建后自动拷贝）
│   ├── lib/                    # 导入库 / 静态库（*.lib）
│   └── opencv4120/             # OpenCV 4.12.0（include/ x64/bin/ x64/lib）
├── FarmeToolBase/              # 工具基类库
├── FrameCameraBase/            # 相机基类库
├── FrameImageView/             # 图像显示与 ROI 绘制 DLL
├── FrameQueueTool/             # 任务间数据队列（enqueue/dequeue）
├── FrameImageTools/            # 图像处理工具集（源/模糊/灰度/模板匹配/仿射变换）
└── VisionFrame/                # 主程序与蓝图编辑器/显示界面
```

## 构建环境需求

| 组件 | 版本 | 说明 |
| --- | --- | --- |
| 操作系统 | Windows 10/11（x64） | 仅支持 64 位 |
| Visual Studio | 2026（18）/ 2022 | 需安装「使用 C++ 的桌面开发」工作负载，MSVC v143 工具集 |
| CMake | ≥ 3.20 | 可用 VS 自带 CMake，或独立安装并加入 PATH |
| Qt | 5.14.2（msvc2017_64） | 需要 Core / Gui / Widgets 模块 |
| OpenCV | 4.12.0 | 使用 `opencv_world` 单一库 |

> 工程使用 C++20 标准；Release 配置也会生成 PDB 调试信息（`/Zi /DEBUG`），便于断点调试。

## 依赖路径说明

以下路径为构建脚本中的**默认值**，均通过 CMake 缓存变量管理，可在配置阶段覆盖（见下一节）。

| 变量 | 默认值 | 用途 |
| --- | --- | --- |
| `QT_DIR` | `D:/Qt/Qt5.14.2/5.14.2/msvc2017_64/lib/cmake/Qt5` | `find_package(Qt5)` 查找位置 |
| `QT_BIN` | `D:/Qt/Qt5.14.2/5.14.2/msvc2017_64/bin` | 拷贝 Qt DLL 的来源目录 |
| `QT_PLUGINS` | `D:/Qt/Qt5.14.2/5.14.2/msvc2017_64/plugins/platforms` | 拷贝 `qwindows.dll` 平台插件 |
| `OPENCV_ROOT` | `${CMAKE_SOURCE_DIR}/Include/opencv4120` | OpenCV 安装根目录 |
| `OPENCV_INCLUDE` | `${OPENCV_ROOT}/include` | OpenCV 头文件目录 |
| `OPENCV_LIB_DIR` | `${OPENCV_ROOT}/x64/lib` | OpenCV 导入库目录 |
| `OPENCV_BIN_DIR` / `OPENCV_BIN` | `${OPENCV_ROOT}/x64/bin` | OpenCV DLL 目录 |

> OpenCV 目录需满足 `include/`、`x64/bin/`、`x64/lib/` 的结构（即安装版 layout）。工程默认使用 `Include/opencv4120` 内嵌目录。

## 如何修改依赖路径

有两种方式，推荐先用命令行 `-D` 覆盖（临时），确认无误后再改默认值（永久）。

### 方式一：命令行覆盖（临时）

配置时传入对应变量即可，例如把 Qt 换成 `E:/Qt/5.14.2/msvc2017_64`：

```powershell
cmake --preset Release --fresh `
  -DQT_DIR="E:/Qt/5.14.2/msvc2017_64/lib/cmake/Qt5" `
  -DQT_BIN="E:/Qt/5.14.2/msvc2017_64/bin" `
  -DQT_PLUGINS="E:/Qt/5.14.2/msvc2017_64/plugins/platforms"
```

替换 OpenCV 位置：

```powershell
cmake --preset Release --fresh `
  -DOPENCV_ROOT="D:/opencv/build/x64"        # 根目录下需有 include/ x64/bin/ x64/lib
```

### 方式二：修改构建脚本默认值（永久）

- Qt 路径：编辑根目录 `CMakeLists.txt` 中的 `QT_DIR_DEFAULT`（约第 62 行），以及 `VisionFrame/CMakeLists.txt` 中的 `QT_BIN`（约第 86 行）与 `QT_PLUGINS`（约第 87 行）。
- OpenCV 路径：编辑根目录 `CMakeLists.txt` 中的 `OPENCV_ROOT` / `OPENCV_INCLUDE` / `OPENCV_LIB_DIR` / `OPENCV_BIN_DIR`（约第 80–83 行），以及 `VisionFrame/CMakeLists.txt` 中的 `OPENCV_BIN`（约第 88 行）。

### 更换 OpenCV 版本的额外注意

工程针对 OpenCV 4.12.0 的 `opencv_world` 单库硬编码了文件名（位于根目录 `CMakeLists.txt` 约第 90–93 行）：

```cmake
IMPORTED_LOCATION_DEBUG    ".../opencv_world4120d.dll"
IMPORTED_LOCATION_RELEASE  ".../opencv_world4120.dll"
IMPORTED_IMPLIB_DEBUG      ".../opencv_world4120d.lib"
IMPORTED_IMPLIB_RELEASE    ".../opencv_world4120.lib"
```

换版本时需同步修改这些文件名（例如 4.7.0 对应 `opencv_world470.dll`），以及 `VisionFrame/CMakeLists.txt` 中 POST_BUILD 拷贝逻辑里的 DLL 名字（约第 95 行）。

## 构建步骤

### 命令行（PowerShell）

```powershell
# 1. 配置（首次构建，或修改 CMakeLists.txt / CMakePresets.json 后建议加 --fresh）
cmake --preset Release --fresh

# 2. 编译
cmake --build build/Release --config Release
```

生成产物：
- 主程序：`App/VisionFrame.exe`
- 业务 DLL：`App/Bin/`
- 库文件：`Include/lib/`，对外头文件：`Include/include/`

### Visual Studio

配置阶段结束后，直接双击打开 `build/Release/VisionFrame.slnx`，然后在 VS 中选择 `Release` + `x64` 配置生成。

> 若 `cmake` 不在 PATH 中，可用 VS 自带的 CMake 可执行文件，例如
> `D:\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`。

## 输出与运行

构建完成后，直接运行 `App/VisionFrame.exe` 即可，无需额外手动拷贝 DLL——构建脚本会在 POST_BUILD 阶段自动完成：

- Qt / OpenCV 运行期 DLL 拷贝到 `App/`（与 exe 同级）
- `qwindows.dll` 拷贝到 `App/platforms/`
- `FrameImageView.dll` 拷贝到 `App/`（含 Q_OBJECT，需随 exe 放置）

业务 DLL（`FrameToolBase.dll`、`FrameCameraBase.dll` 等）通过 `/DELAYLOAD` 延迟加载，程序启动后由 `main()` 中的 `SetDllDirectoryW` 从 `App/Bin/` 解析。

## 注意事项

- **仅支持 x64**：工程在配置阶段会校验目标架构，非 64 位将报错退出。
- **修改 `CMakeLists.txt` 后需重新配置**：新增编译选项/输出目录变更时，旧的 `build/Release` 可能残留旧缓存，建议删除该目录或使用 `--fresh` 重新配置。
- **Release 含调试信息**：Release 配置附加了 `/Zi /DEBUG /O2 /OPT:REF /OPT:ICF`，可正常断点调试，无需切换到 Debug。
- **依赖路径一致性**：Qt 的三处路径（`QT_DIR` / `QT_BIN` / `QT_PLUGINS`）应指向同一份 Qt 安装，避免配置、链接与部署三者不一致导致的运行错误。