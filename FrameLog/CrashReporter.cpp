#include "pch.h"
#include "CrashReporter.h"

#include <atomic>
#include <cstdio>
#include <ctime>
#include <exception>
#include <filesystem>
#include <string>

#include <dbghelp.h>

namespace
{
    std::string g_dumpDir;
    std::atomic<bool> g_inCrash{ false };

    std::string timestampCompact()
    {
        std::time_t t = std::time(nullptr);
        std::tm tmb{};
        localtime_s(&tmb, &t);
        char buf[32];
        snprintf(buf, sizeof(buf), "%04d%02d%02d_%02d%02d%02d",
                 tmb.tm_year + 1900, tmb.tm_mon + 1, tmb.tm_mday,
                 tmb.tm_hour, tmb.tm_min, tmb.tm_sec);
        return buf;
    }

    // 动态加载 dbghelp.dll，避免静态链接依赖。
    void writeDump(EXCEPTION_POINTERS* ep)
    {
        if (g_dumpDir.empty())
            return;

        HMODULE hDbg = LoadLibraryW(L"dbghelp.dll");
        if (!hDbg)
            return;

        using MiniDumpWriteDumpFn = BOOL(WINAPI*)(
            HANDLE, DWORD, HANDLE, MINIDUMP_TYPE,
            PMINIDUMP_EXCEPTION_INFORMATION, PMINIDUMP_USER_STREAM_INFORMATION,
            PMINIDUMP_CALLBACK_INFORMATION);

        auto pfn = (MiniDumpWriteDumpFn)GetProcAddress(hDbg, "MiniDumpWriteDump");
        if (!pfn)
        {
            FreeLibrary(hDbg);
            return;
        }

        std::error_code ec;
        std::filesystem::create_directories(g_dumpDir, ec);

        const std::string path = g_dumpDir + "\\VisionFrame_" + timestampCompact() + ".dmp";
        HANDLE hFile = CreateFileA(path.c_str(), GENERIC_WRITE, 0, nullptr,
                                   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            FreeLibrary(hDbg);
            return;
        }

        MINIDUMP_EXCEPTION_INFORMATION mei{};
        mei.ThreadId = GetCurrentThreadId();
        mei.ExceptionPointers = ep;
        mei.ClientPointers = FALSE;

        const MINIDUMP_TYPE type = (MINIDUMP_TYPE)(MiniDumpNormal | MiniDumpWithDataSegs | MiniDumpWithThreadInfo);
        pfn(GetCurrentProcess(), GetCurrentProcessId(), hFile, type,
            ep ? &mei : nullptr, nullptr, nullptr);

        CloseHandle(hFile);
        FreeLibrary(hDbg);
    }

    LONG WINAPI sehHandler(EXCEPTION_POINTERS* ep)
    {
        if (g_inCrash.exchange(true))
            return EXCEPTION_EXECUTE_HANDLER;

        const DWORD code = (ep && ep->ExceptionRecord) ? ep->ExceptionRecord->ExceptionCode : 0;
        const void* addr = (ep && ep->ExceptionRecord) ? ep->ExceptionRecord->ExceptionAddress : nullptr;

        char buf[96];
        snprintf(buf, sizeof(buf), "code=0x%08lX, address=0x%p", (unsigned long)code, addr);
        VFL_FATAL("crash") << "Unhandled SEH exception, " << buf;

        vflog::Logger::instance().flush();
        writeDump(ep);
        return EXCEPTION_EXECUTE_HANDLER;
    }

    void terminateHandler()
    {
        if (g_inCrash.exchange(true))
            std::abort();

        std::string what = "unknown";
        try
        {
            if (auto e = std::current_exception())
                std::rethrow_exception(e);
        }
        catch (const std::exception& ex)
        {
            what = ex.what();
        }
        catch (...)
        {
            what = "non-std exception";
        }

        VFL_FATAL("crash") << "Unhandled C++ exception: " << what;

        vflog::Logger::instance().flush();
        writeDump(nullptr);
        std::abort();
    }
} // namespace

namespace vflog
{
    void installCrashHandler(const std::string& dumpDir)
    {
        g_dumpDir = dumpDir;
        SetUnhandledExceptionFilter(&sehHandler);
        std::set_terminate(&terminateHandler);
    }
} // namespace vflog