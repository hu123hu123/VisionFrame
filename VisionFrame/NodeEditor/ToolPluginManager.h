#pragma once

// 宿主侧工具插件加载器（跨平台）：
// 扫描 <exe>/Bin/plugins/（Windows: *.dll / Linux: *.so / macOS: *.dylib），
// 逐个加载并校验插件契约（FrameToolPlugin.h），
// 合法插件经 vfRegisterTools 把工具注册进 ToolFactory。单个插件失败仅记日志。
namespace ToolPluginManager
{
    // 加载全部工具插件；返回成功注册的插件数量。需在应用对象（QApplication/QCoreApplication）构造后调用。
    int loadAll();
}
