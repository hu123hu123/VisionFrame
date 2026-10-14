#include "VisonFrame.h"
#include "NodeEditor/ToolPluginManager.h"
#include <QtWidgets/QApplication>
#include <QCoreApplication>
#include <QTimer>
#include <atomic>
#include <csignal>
#include <cstring>
#include <memory>
#include <string>

#include "FrameLog.h"
#include "CrashReporter.h"
#include "Log/QtLogBridge.h"
#include "ProjectConfig.h"
#include "TaskController.h"

#if defined(_WIN32)
#include <windows.h>
// 将 DLL 搜索路径设置为 exe 所在目录
// Qt/OpenCV/业务 DLL 均已由 CMake 拷贝到 exe 同级目录，Windows 加载器自动查找
// Linux 由 RPATH($ORIGIN) 解析依赖，无需此步骤
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
#endif

// 无头模式停止标志（SIGINT/SIGTERM 置位，主线程定时轮询）
static std::atomic<bool> g_headlessStop{ false };
static void headlessSignalHandler(int)
{
    g_headlessStop.store(true);
}

static void installHeadlessSignals()
{
    std::signal(SIGINT, headlessSignalHandler);
    std::signal(SIGTERM, headlessSignalHandler);
}

// 无头运行：加载工程 -> 启动全部任务 -> 常驻直到收到停止信号
static int runHeadless(QCoreApplication& app, const QString& projectPath)
{
    const int plugins = ToolPluginManager::loadAll();
    VFL_INFO("app") << "[headless] tool plugins registered: " << plugins;

    if (!ProjectConfig::instance().loadProject(projectPath))
    {
        VFL_ERROR("app") << "[headless] load project failed: " << projectPath.toStdString();
        return 2;
    }
    VFL_INFO("app") << "[headless] project loaded: " << projectPath.toStdString()
                    << ", tasks=" << ProjectConfig::instance().taskItems.size();

    for (TaskItem* t : ProjectConfig::instance().taskItems)
        TaskController::instance().AddTask(t);
    TaskController::instance().StartAllTasks();
    VFL_INFO("app") << "[headless] all tasks started";

    installHeadlessSignals();
    QTimer stopPoller;
    stopPoller.setInterval(200);
    QObject::connect(&stopPoller, &QTimer::timeout, &app, [&app]() {
        if (g_headlessStop.load())
        {
            VFL_INFO("app") << "[headless] stop requested, stopping tasks";
            TaskController::instance().StopAllTasks();
            app.quit();
        }
    });
    stopPoller.start();

    const int rc = app.exec();
    VFL_INFO("app") << "[headless] exiting, code=" << rc;
    vflog::Logger::instance().flush();
    return rc;
}

int main(int argc, char* argv[])
{
    // 在创建应用对象之前设置 DLL 搜索路径（仅 Windows），
    // 确保业务 DLL 能从 exe\Bin 加载
#if defined(_WIN32)
    setupDllSearchPath();
#endif

    // 解析 --headless <工程文件>：无 GUI 运行（边缘部署）
    bool headless = false;
    QString headlessProject;
    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--headless") == 0)
        {
            headless = true;
            if (i + 1 < argc)
                headlessProject = QString::fromLocal8Bit(argv[i + 1]);
        }
    }

    std::unique_ptr<QCoreApplication> app;
    if (headless)
        app = std::make_unique<QCoreApplication>(argc, argv);
    else
        app = std::make_unique<QApplication>(argc, argv);

    // 1) 初始化日志核心：文件 sink（<exe>/logs）+ 调试输出 sink 内建于 logger
    vflog::Config logCfg;
    logCfg.logDir = (QCoreApplication::applicationDirPath() + "/logs").toStdString();
    logCfg.minLevel = vflog::Level::Info;
    vflog::Logger::instance().configure(logCfg);

    // 2) 桥接既有 qDebug/qWarning 到新日志系统（不改动旧代码）
    installQtLogBridge();

    // 3) 崩溃捕获（Windows: MiniDump / POSIX: backtrace）
    vflog::installCrashHandler(logCfg.logDir + "/dumps");

    if (headless)
    {
        if (headlessProject.isEmpty())
        {
            VFL_ERROR("app") << "[headless] missing project path, usage: VisionFrame --headless <project.vfProj>";
            return 1;
        }
        return runHeadless(*app, headlessProject);
    }

    const int plugins = ToolPluginManager::loadAll();
    VFL_INFO("app") << "tool plugins registered: " << plugins;

    VFL_INFO("app") << "VisionFrame starting";

    VisionFrame window;
    window.show();

    const int rc = app->exec();
    VFL_INFO("app") << "VisionFrame exiting, code=" << rc;
    vflog::Logger::instance().flush();
    return rc;
}
