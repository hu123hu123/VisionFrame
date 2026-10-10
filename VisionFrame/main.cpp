#include "VisonFrame.h"
#include "NodeEditor/ToolFactory.h"
#include <QtWidgets/QApplication>
#include <windows.h>
#include <string.h>
#include <string>

#include "FrameLog.h"
#include "CrashReporter.h"
#include "Log/QtLogBridge.h"

// 将 DLL 搜索路径设置为 exe 所在目录
// Qt/OpenCV/业务 DLL 均已由 CMake 拷贝到 exe 同级目录，Windows 加载器自动查找
// 此函数保留用于将来按需扩展 DLL 搜索路径（如 Bin 子目录中的可选插件）
static void setupDllSearchPath()
{
    wchar_t exePath[MAX_PATH] = { 0 };
    if (GetModuleFileNameW(nullptr, exePath, MAX_PATH) == 0)
        return;

    wchar_t* slash = wcsrchr(exePath, L'\\');
    if (!slash)
        return;
    *slash = L'\0';

    wchar_t binPath[MAX_PATH];
    if (wcscpy_s(binPath, MAX_PATH, exePath) != 0)
        return;
    if (wcscat_s(binPath, MAX_PATH, L"\\Bin") != 0)
        return;

    SetDllDirectoryW(binPath);
}

// 显式加载业务插件 DLL（路径为 exe\Bin\<name>）
// 成功返回模块句柄，失败返回 nullptr
static HMODULE loadPlugin(const wchar_t* name)
{
    wchar_t exePath[MAX_PATH] = { 0 };
    if (GetModuleFileNameW(nullptr, exePath, MAX_PATH) == 0)
        return nullptr;

    wchar_t* slash = wcsrchr(exePath, L'\\');
    if (slash)
        *slash = L'\0';

    wchar_t pluginPath[MAX_PATH];
    if (wcscpy_s(pluginPath, MAX_PATH, exePath) != 0)
        return nullptr;
    if (wcscat_s(pluginPath, MAX_PATH, L"\\Bin\\") != 0)
        return nullptr;
    if (wcscat_s(pluginPath, MAX_PATH, name) != 0)
        return nullptr;

    return LoadLibraryW(pluginPath);
}

// 取 exe 所在目录（UTF-8），用于定位 logs 输出目录。
static std::string exeDirUtf8()
{
    wchar_t exePath[MAX_PATH] = { 0 };
    if (GetModuleFileNameW(nullptr, exePath, MAX_PATH) == 0)
        return std::string(".");
    wchar_t* slash = wcsrchr(exePath, L'\\');
    if (slash)
        *slash = L'\0';

    const int n = WideCharToMultiByte(CP_UTF8, 0, exePath, -1, nullptr, 0, nullptr, nullptr);
    if (n <= 0)
        return std::string(".");
    std::string out((size_t)n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, exePath, -1, &out[0], n, nullptr, nullptr);
    while (!out.empty() && out.back() == '\0')
        out.pop_back();
    return out;
}

int main(int argc, char* argv[])
{
    // 在创建 QApplication 之前设置 DLL 搜索路径，
    // 确保 Qt 等依赖 DLL 能从 exe\Bin 加载
    setupDllSearchPath();

    // 1) 初始化日志核心：文件 sink（<exe>/logs）+ 调试输出 sink 内建于 logger
    vflog::Config logCfg;
    logCfg.logDir = exeDirUtf8() + "\\logs";
    logCfg.minLevel = vflog::Level::Info;
    vflog::Logger::instance().configure(logCfg);

    // 2) 桥接既有 qDebug/qWarning 到新日志系统（不改动旧代码）
    installQtLogBridge();

    // 3) 崩溃捕获 + MiniDump 转储
    vflog::installCrashHandler(logCfg.logDir + "\\dumps");

    QApplication app(argc, argv);
    registerBuiltinTools();

    VFL_INFO("app") << "VisionFrame starting";

    VisionFrame window;
    window.show();

    // 示例：按需显式加载业务插件 DLL
    // HMODULE hPlugin = loadPlugin(L"MyPlugin.dll");
    // if (hPlugin) { /* auto fn = (FnType)GetProcAddress(hPlugin, "Run"); fn(...); */ }

    const int rc = app.exec();
    VFL_INFO("app") << "VisionFrame exiting, code=" << rc;
    vflog::Logger::instance().flush();
    return rc;
}
