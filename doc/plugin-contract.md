# VisionFrame 工具插件契约文档

> 版本：`VF_PLUGIN_API_VERSION = 1`
> 本文档描述宿主（VisionFrame）与工具插件（各 FrameXxxTools 动态库）之间的全部接口约定。
> 实现新工具或新插件时，必须遵守本文档中的每一项契约。

---

## 1. 总体架构

```
┌─────────────────────────────────────────────────────┐
│  宿主 VisionFrame（GUI / --headless）                │
│  ToolPluginManager: 扫描 <exe>/Bin/plugins/ 并加载    │
└──────────────────────┬──────────────────────────────┘
                       │ C ABI 契约（FrameToolPlugin.h）
                       ▼
┌─────────────────────────────────────────────────────┐
│  ToolFactory（FrameToolRegistry.dll，全局单例）       │
│  宿主与插件链接同一份，保证唯一实例                    │
└──────────────────────┬──────────────────────────────┘
                       │ C++ 虚接口契约（FrameToolBase.h）
                       ▼
┌─────────────────────────────────────────────────────┐
│  工具插件（FrameImageTools / FrameCameraTools / …）   │
│  工具实现 + 参数控件 + PluginExport.cpp 注册           │
└─────────────────────────────────────────────────────┘
```

涉及的关键头文件：

| 头文件 | 契约内容 |
|---|---|
| `FrameToolRegistry/FrameToolPlugin.h` | 插件 C ABI 契约（导出函数、版本号、导出宏） |
| `FrameToolRegistry/ToolFactory.h` | 工具注册项 `ToolEntry` 与注册表接口 |
| `FarmeToolBase/FrameToolBase.h` | 工具基类虚接口、引脚、执行结果 |
| `VisionFrame/NodeEditor/ToolPluginManager.h` | 宿主侧加载规则 |

---

## 2. 插件 C ABI 契约（必须遵守）

每个工具插件动态库**必须**导出以下两个 `extern "C"` 函数（x64 下名字不修饰）：

```cpp
// 返回本插件编译时所用的契约版本号
extern "C" int  vfPluginApiVersion();          // -> VF_PLUGIN_API_VERSION

// 把本插件的全部工具注册进宿主注册表
extern "C" void vfRegisterTools(ToolFactory* factory);
```

规则：

1. **版本校验**：宿主加载插件时先调用 `vfPluginApiVersion()`，返回值不等于宿主的 `VF_PLUGIN_API_VERSION` 时拒绝注册并记日志。
2. **导出宏**：契约函数必须使用 `VF_PLUGIN_EXPORT` 标记（跨平台，见 `FrameToolPlugin.h`）：
   - Windows：`extern "C" __declspec(dllexport)`
   - Linux/macOS：`extern "C" __attribute__((visibility("default")))`
3. **健壮性**：`vfRegisterTools` 实现开头必须判空 `factory`。
4. **失败隔离**：单个插件加载失败（缺失导出、版本不符、崩溃）只记日志，不影响宿主和其它插件。

标准写法见 `FrameImageTools/PluginExport.cpp`：

```cpp
VF_PLUGIN_EXPORT int vfPluginApiVersion()
{
    return VF_PLUGIN_API_VERSION;
}

VF_PLUGIN_EXPORT void vfRegisterTools(ToolFactory* f)
{
    if (!f)
        return;
    // 逐个 registerTool(...)
}
```

---

## 3. 工具注册契约（ToolEntry）

每个工具通过 `ToolFactory::registerTool(const ToolEntry&)` 注册，字段定义（`ToolFactory.h`）：

| 字段 | 类型 | 要求 |
|---|---|---|
| `typeId` | `QString` | **全局唯一**、稳定的类型标识（如 `"ImageSaveTool"`）。已保存工程（.vfproj）按它反序列化节点，**一旦发布不可更改** |
| `displayName` | `QString` | UI 显示名（中文，如 `"图像保存"`），用 `QString::fromUtf8` 构造 |
| `category` | `QString` | 工具箱分类名（如 `"图像处理"` / `"图像源"`） |
| `create` | `std::function<FrameToolBase*()>` | 工具工厂函数，每次调用返回新实例 |
| `createWidget` | `std::function<QWidget*(FrameToolBase*)>` | 参数控件工厂；**可为 `nullptr`**（无 UI 的工具，如 `ImageGrayscaleTool`） |

注册表接口（宿主与插件共用 `FrameToolRegistry` 动态库，保证单例唯一）：

```cpp
void registerTool(const ToolEntry& e);          // 注册
FrameToolBase* create(const QString& typeId) const;  // 按类型创建实例
const ToolEntry* find(const QString& typeId) const;
const QVector<ToolEntry>& tools() const;         // 全部已注册工具
QVector<QString> categories() const;             // 去重后的分类列表
```

---

## 4. 工具实现契约（FrameToolBase）

所有工具必须继承 `FrameToolBase`（`FarmeToolBase/FrameToolBase.h`）并实现以下纯虚函数：

| 虚函数 | 契约 |
|---|---|
| `std::string ToolName()` | 返回工具名（诊断/日志用） |
| `ToolResult execute()` | 无参执行（历史队列工具使用） |
| `void LoadParam(std::string strFilePath)` | 从文件加载参数（旧接口，可为存根） |
| `void SaveParam(std::string strFilePath)` | 保存参数到文件（旧接口，可为存根） |

按需覆写的虚函数：

| 虚函数 | 默认行为 | 何时覆写 |
|---|---|---|
| `void GetPorts(std::vector<FPort>&) const` | 空（无引脚） | 声明蓝图式引脚的工具 |
| `std::string SaveParamsToJson() const` | 空串 | 有可持久化参数的工具 |
| `void LoadParamsFromJson(const std::string&)` | 忽略 | 与上一条成对实现 |
| `ToolResult execute(const std::map<std::string,NodeData>& in, std::map<std::string,NodeData>& out)` | 回退到无参 `execute()` | 参与数据流的引脚工具 |
| `void setOwnerQueueName(const std::string&)` | 空 | 需感知所属任务队列的工具（如出队工具） |

### 4.1 执行结果 `ToolResult`

```cpp
enum ToolResult {
    ToolOk = 0,           // 正常完成
    ToolInterrupted = 1,  // 被 stop 唤醒（队列停止等），任务应退出
    ToolError = 2         // 执行异常
};
```

`execute` 中任何失败路径（输入缺失、处理失败、写盘失败）必须返回 `ToolError`，不得抛异常穿越宿主。

### 4.2 数据流执行契约

- 输入 `in`：键为**输入引脚名**（`FPort::name`）。取值前必须用 `find` 判断键存在，缺失且引脚非 `optional` 时返回 `ToolError`。
- 输出 `out`：键为**输出引脚名**。成功路径必须为每个非 optional 输出引脚写入数据。
- 声明为 `PinType::Image` 的引脚，`std::any` 内承载 `cv::Mat`。

示例（`ImageSaveTool::execute`）：

```cpp
ToolResult ImageSaveTool::execute(const std::map<std::string, NodeData>& in,
                                  std::map<std::string, NodeData>& out)
{
    auto it = in.find("InputImage");
    if (it == in.end())
        return ToolResult::ToolError;
    const cv::Mat img = std::any_cast<cv::Mat>(it->second);
    if (img.empty() || m_savePath.empty())
        return ToolResult::ToolError;
    if (!cv::imwrite(m_savePath, img))
        return ToolResult::ToolError;
    return ToolResult::ToolOk;
}
```

---

## 5. 引脚契约（FPort / PinType）

```cpp
enum class PinType { Image, Bool, Int, Double, String, Any };

struct FPort {
    std::string name;
    PinType     type = PinType::Any;
    bool        isInput = true;   // true=输入引脚, false=输出引脚
    bool        optional = false; // 输入引脚是否可缺省
};
```

连接规则（宿主 `TaskGraph::canConnect` 强制执行）：

1. 同名引脚必须存在且方向匹配（输出→输入）。
2. 一个输入引脚**最多一条入边**。
3. 类型兼容：两侧类型相等，或任一侧为 `PinType::Any`。
4. 不允许成环（拓扑排序校验）。
5. `GetPorts` 返回的引脚名即工程文件中边（edges）的 `fromPin`/`toPin` 值，**发布后不可随意改名**。

---

## 6. 参数持久化契约

两级序列化机制：

1. **JSON 序列化（新，工程文件使用）**：`SaveParamsToJson()` 返回 JSON 文本（空串表示无参数）；`LoadParamsFromJson()` 从 JSON 文本恢复。基类不依赖 Qt，故用 `std::string` 承载；具体工具可用 `QJsonDocument` 实现。
2. **文件存根（旧）**：`LoadParam/SaveParam(strFilePath)` 为历史接口，新工具实现为空存根即可。

规则：

- 两个函数必须**成对实现**、字段一致，保证保存-加载往返无损。
- JSON 中的键名是工程文件 `params` 对象的键，发布后不可随意改名（旧工程兼容）。

---

## 7. 参数控件契约（可选）

`createWidget` 非空时，控件必须遵守：

1. 控件以工具实例指针入参，构造时**从工具读初始值**填充 UI。
2. UI 修改即通过 `changed()` 信号（或多个具名信号）通知，PluginExport 中连接信号**回写工具**（双向绑定）。
3. 控件内部回填初始值时用抑制标志（如 `m_updating`）避免误发 `changed()`。
4. 控件不直接持有工具生命周期，**不得 delete 工具**；宿主负责工具与控件的生命周期。
5. 控件使用 `QWidget + Ui 前置声明` 模式，`.ui` 文件的 `<class>` 必须与控件类名一致。

标准写法见 `FrameImageTools/PluginExport.cpp` 中 `ImageSaveTool` 的注册：

```cpp
save.createWidget = [](FrameToolBase* t) -> QWidget* {
    auto* tool = static_cast<ImageSaveTool*>(t);
    auto* w = new ImageSaveToolWidget();
    w->setSavePath(tool->savePath());                      // 工具 -> 控件
    QObject::connect(w, &ImageSaveToolWidget::changed, w,  // 控件 -> 工具
        [tool, w]() { tool->setSavePath(w->savePath()); });
    return w;
};
```

---

## 8. 插件加载契约（宿主侧）

宿主 `ToolPluginManager::loadAll()` 的行为（`VisionFrame/NodeEditor/ToolPluginManager.h/.cpp`）：

1. **调用时机**：必须在应用对象（`QApplication` / `QCoreApplication`）构造之后调用。
2. **扫描目录**：`<exe所在目录>/Bin/plugins/`（exe 路径经 `QCoreApplication::applicationDirPath()` 解析）。
3. **文件过滤**（按平台）：

   | 平台 | 候选文件 |
   |---|---|
   | Windows | `*.dll` |
   | Linux | `*.so` |
   | macOS | `*.dylib`、`*.so` |

4. **加载方式**：`QLibrary` 依次 resolve `vfPluginApiVersion` 与 `vfRegisterTools`，任一缺失即判定非法并记日志跳过。
5. **版本校验**：`vfPluginApiVersion() != VF_PLUGIN_API_VERSION` 时拒绝注册。
6. **结果**：返回成功注册的插件数量；单个插件失败不影响其它插件。

---

## 9. 工程文件格式契约（.vfproj）

工程文件为**UTF-8 JSON**（`ProjectConfig::saveProject/loadProject`），缺省扩展名自动补 `.vfproj`。

顶层结构：

```json
{
  "version": 1,
  "projectName": "示例工程",
  "tasks": [
    {
      "taskName": "Task1",
      "graph": {
        "nodes": [
          {
            "id": "节点唯一id（缺省时自动生成UUID）",
            "type": "ImageSourceTool",
            "title": "图像源",
            "x": 100.0,
            "y": 200.0,
            "params": { "path": "/tmp/in.png" }
          }
        ],
        "edges": [
          {
            "id": "边唯一id",
            "fromNode": "源节点id",
            "fromPin": "OutputImage",
            "toNode": "目标节点id",
            "toPin": "InputImage"
          }
        ]
      }
    }
  ]
}
```

反序列化规则（`TaskGraph::fromJson`）：

- `type` 经 `ToolFactory::create` 创建工具实例；`params` 原样传给 `LoadParamsFromJson`。**未注册的 typeId 将产生空工具节点**，因此插件必须先于工程加载注册。
- `edges` 的 `fromPin`/`toPin` 与工具 `GetPorts` 声明的引脚名对应。
- `version` 当前固定为 `1`。

---

## 10. 跨平台与部署约定

### 10.1 目录布局（Windows 与 Linux 一致）

```
<App>/
├── VisionFrame(.exe)          # 宿主（GUI / --headless 同一二进制）
├── Qt5*.dll / libQt5*.so.5    # Qt 运行时（Linux 由 RPATH + 系统包解析）
├── FrameToolRegistry.dll      # 注册表锚点（宿主与插件共用同一实例）
├── FrameToolBase.dll / FrameCameraBase.dll / FrameLog.dll / FrameImageView.dll
├── Bin/
│   └── plugins/               # ★ 工具插件目录（扫描入口）
│       ├── FrameImageTools.dll(.so)
│       └── ...
└── logs/                      # 运行日志与崩溃转储
```

### 10.2 跨平台符号导出

公共接口宏统一采用条件定义（以 `FARMETOOLBASE_API` 为例，各库同构）：

```cpp
#if defined(_WIN32)
  #ifdef FARMETOOLBASE_EXPORTS
  #define FARMETOOLBASE_API __declspec(dllexport)
  #else
  #define FARMETOOLBASE_API __declspec(dllimport)
  #endif
#else
  #define FARMETOOLBASE_API __attribute__((visibility("default")))
#endif
```

### 10.3 Linux 运行时解析

- 可执行文件与动态库设置 RPATH：`$ORIGIN;$ORIGIN/Bin;$ORIGIN/..`，保证 `App/`、`App/Bin/`、`App/Bin/plugins/` 三层都能相互找到依赖库。
- 插件在 Linux 下为 LIBRARY 产物，CMake 已同时设置 `RUNTIME` 与 `LIBRARY_OUTPUT_DIRECTORY` 指向 `App/Bin/plugins/`。

### 10.4 无头运行（边缘部署）

```bash
VisionFrame --headless /path/to/project.vfproj
```

- 无 GUI，使用 `QCoreApplication`；加载流程与 GUI 完全一致（loadAll → loadProject → StartAllTasks）。
- `SIGINT`/`SIGTERM` 触发优雅停止（StopAllTasks 后退出）。
- 退出码：`0` 正常；`1` 缺少工程路径参数；`2` 工程加载失败。
- 常驻部署参考 `deploy/visionframe-headless.service`（systemd 模板）。

---

## 11. 新工具接入清单（Checklist）

新增一个工具（以 `MyTool` 为例）需要完成：

- [ ] `MyTool.h/.cpp`：继承 `FrameToolBase`，实现 4 个纯虚函数；引脚工具另实现 `GetPorts` + `execute(in,out)`；有参数则成对实现 `SaveParamsToJson/LoadParamsFromJson`
- [ ] （可选）`MyToolWidget.h/.cpp/.ui`：参数控件，`changed()` 信号 + `m_updating` 抑制
- [ ] `PluginExport.cpp`：构造 `ToolEntry`（typeId 全局唯一且稳定）并 `f->registerTool(...)`
- [ ] 模块 `CMakeLists.txt`：加入源文件与 .ui；新插件库确保输出到 `App/Bin/plugins/`
- [ ] 构建后确认插件被扫描加载（日志 `tool plugins registered`），GUI 工具箱/参数面板正常
- [ ] 若工具出现在示例工程中，同步核对 `.vfproj` 的 `type` 与引脚名
