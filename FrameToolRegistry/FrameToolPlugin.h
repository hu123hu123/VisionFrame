#pragma once
#include "ToolFactory.h"

// 工具插件 C ABI 契约。
// 每个工具插件 DLL 必须导出以下两个函数（x64 下 extern "C" 名字不修饰）：
//   extern "C" int  vfPluginApiVersion();          -> 返回 VF_PLUGIN_API_VERSION
//   extern "C" void vfRegisterTools(ToolFactory*); -> 把本插件工具注册进注册表
// 宿主在加载插件时先校验版本号，不匹配则拒绝注册。

#define VF_PLUGIN_API_VERSION 1

// 插件侧导出函数指针类型（宿主经 GetProcAddress 获取）
using VfPluginApiVersionFn = int (*)();
using VfRegisterToolsFn    = void (*)(ToolFactory*);

// 插件侧导出宏：用于 PluginExport.cpp 中标记契约函数（跨平台）
#if defined(_WIN32)
  #define VF_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
  #define VF_PLUGIN_EXPORT extern "C" __attribute__((visibility("default")))
#endif
