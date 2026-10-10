#pragma once

#include <string>

#include "FrameLog.h"

namespace vflog
{
    // 安装进程级崩溃捕获：未处理 SEH 异常(访问违规等) + C++ 未捕获异常(std::terminate)。
    // 崩溃时：记录一条 Fatal 日志 → 强制刷盘 → 若 dbghelp.dll 可用则生成 MiniDump → 交给系统终止。
    // dumpDir 为转储文件输出目录(如 "<exe>/logs/dumps")，为空则不写 dump、仅记日志。
    FRAMELOG_API void installCrashHandler(const std::string& dumpDir);
} // namespace vflog