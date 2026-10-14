#pragma once

// FrameToolRegistry 导出宏：注册表由宿主与全部工具插件共用同一 DLL 实例
#if defined(_WIN32)
  #ifdef FRAMETOOLREGISTRY_EXPORTS
  #define FRAMETOOLREGISTRY_API __declspec(dllexport)
  #else
  #define FRAMETOOLREGISTRY_API __declspec(dllimport)
  #endif
#else
  #define FRAMETOOLREGISTRY_API __attribute__((visibility("default")))
#endif
