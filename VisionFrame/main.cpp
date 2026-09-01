#include "VisonFrame.h"
#include <QtWidgets/QApplication>
#include <windows.h>
#include <string.h>

// 将 DLL 搜索路径设置为 exe 所在目录下的 Bin 子目录
// 使程序运行时能从 Bin 自动加载依赖的第三方 DLL（Qt、OpenCV 等）
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

int main(int argc, char* argv[])
{
    // 在创建 QApplication 之前设置 DLL 搜索路径，
    // 确保 Qt 等依赖 DLL 能从 exe\Bin 加载
    setupDllSearchPath();

    QApplication app(argc, argv);
    VisionFrame window;
    window.show();

    // 示例：按需显式加载业务插件 DLL
    // HMODULE hPlugin = loadPlugin(L"MyPlugin.dll");
    // if (hPlugin) { /* auto fn = (FnType)GetProcAddress(hPlugin, "Run"); fn(...); */ }

    return app.exec();
}
