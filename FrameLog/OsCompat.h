#pragma once
// FrameLog 内部跨平台辅助（无 Qt 依赖）：进程 exe 路径、localtime、大小写不敏感比较。
#if defined(_WIN32)
    #include <windows.h>
#else
    #include <strings.h>
    #include <unistd.h>
#endif
#include <ctime>
#include <string>

namespace vflog::os
{
    // 当前进程可执行文件所在目录（UTF-8；失败返回 "."）
    inline std::string exeDirUtf8()
    {
#if defined(_WIN32)
        wchar_t exePath[32768] = { 0 };
        if (GetModuleFileNameW(nullptr, exePath, 32768) == 0)
            return ".";
        wchar_t* slash = wcsrchr(exePath, L'\\');
        if (!slash)
            slash = wcsrchr(exePath, L'/');
        if (slash)
            *slash = L'\0';
        char utf8[32768] = { 0 };
        WideCharToMultiByte(CP_UTF8, 0, exePath, -1, utf8, sizeof(utf8), nullptr, nullptr);
        return std::string(utf8);
#elif defined(__APPLE__)
        char buf[4096] = { 0 };
        uint32_t size = sizeof(buf);
        if (_NSGetExecutablePath(buf, &size) != 0)
            return ".";
        std::string exe(buf);
        std::string::size_type slash = exe.rfind('/');
        return (slash == std::string::npos) ? "." : exe.substr(0, slash);
#else
        char buf[4096];
        ssize_t n = ::readlink("/proc/self/exe", buf, sizeof(buf) - 1);
        if (n <= 0)
            return ".";
        buf[n] = '\0';
        std::string exe(buf);
        std::string::size_type slash = exe.rfind('/');
        return (slash == std::string::npos) ? "." : exe.substr(0, slash);
#endif
    }

    // localtime 的跨平台包装
    inline void localtimeSafe(std::time_t t, std::tm* out)
    {
#if defined(_MSC_VER)
        localtime_s(out, &t);
#else
        localtime_r(&t, out);
#endif
    }

    // 大小写不敏感字符串比较
    inline int strcasecmpSafe(const char* a, const char* b)
    {
#if defined(_MSC_VER)
        return _stricmp(a, b);
#else
        return ::strcasecmp(a, b);
#endif
    }
} // namespace vflog::os
