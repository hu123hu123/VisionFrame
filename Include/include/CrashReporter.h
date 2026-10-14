#pragma once

#include <string>

#include "FrameLog.h"

namespace vflog
{
    // 安装进程级崩溃捕获：未处理异常 + C++ 未捕获异常(std::terminate)。
    // Windows：未处理 SEH → 记录 Fatal 日志 → 刷盘 → dbghelp 生成 MiniDump → 交给系统终止。
    // Linux/macOS：致命信号(SIGSEGV/SIGABRT/SIGFPE/SIGBUS/SIGILL) → Fatal 日志 + backtrace → 恢复默认处置(core dump)。
    // dumpDir 为转储文件输出目录(如 "<exe>/logs/dumps"，仅 Windows 使用)，为空则不写 dump、仅记日志。
    FRAMELOG_API void installCrashHandler(const std::string& dumpDir);
} // namespace vflog