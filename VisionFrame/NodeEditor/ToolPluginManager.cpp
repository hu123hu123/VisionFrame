#include "ToolPluginManager.h"
#include "ToolFactory.h"
#include "FrameToolPlugin.h"
#include "FrameLog.h"

#include <QCoreApplication>
#include <QDir>
#include <QLibrary>
#include <QString>
#include <memory>
#include <vector>

namespace
{
// 已加载插件模块保持到进程结束（工具实例生命周期不确定，不安全卸载）
std::vector<std::unique_ptr<QLibrary>>& loadedModules()
{
    static std::vector<std::unique_ptr<QLibrary>> s;
    return s;
}
} // namespace

int ToolPluginManager::loadAll()
{
    const QString dir = QCoreApplication::applicationDirPath() + "/Bin/plugins";
    // 各平台动态库后缀（避免把 .pdb 等伴生文件当候选）
#if defined(_WIN32)
    const QStringList filters = { "*.dll" };
#elif defined(__APPLE__)
    const QStringList filters = { "*.dylib", "*.so" };
#else
    const QStringList filters = { "*.so" };
#endif
    const QStringList files = QDir(dir).entryList(filters, QDir::Files);
    if (files.isEmpty())
    {
        VFL_WARN("plugin") << "plugin dir missing or empty: " << dir.toStdString();
        return 0;
    }

    int ok = 0;
    for (const QString& name : files)
    {
        const QString full = dir + "/" + name;
        auto lib = std::make_unique<QLibrary>(full);
        if (!lib->load())
        {
            VFL_ERROR("plugin") << "load failed: " << name.toStdString()
                                << ", err=" << lib->errorString().toStdString();
            continue;
        }

        auto verFn = (VfPluginApiVersionFn)lib->resolve("vfPluginApiVersion");
        auto regFn = (VfRegisterToolsFn)lib->resolve("vfRegisterTools");
        if (!verFn || !regFn)
        {
            VFL_INFO("plugin") << "skip non-tool library: " << name.toStdString();
            lib->unload();
            continue;
        }

        const int ver = verFn();
        if (ver != VF_PLUGIN_API_VERSION)
        {
            VFL_WARN("plugin") << "version mismatch: " << name.toStdString()
                               << ", plugin=" << ver
                               << " host=" << VF_PLUGIN_API_VERSION;
            lib->unload();
            continue;
        }

        regFn(&ToolFactory::instance());
        loadedModules().push_back(std::move(lib));
        ++ok;
        VFL_INFO("plugin") << "tool plugin registered: " << name.toStdString();
    }

    VFL_INFO("plugin") << "tool plugins loaded: " << ok;
    return ok;
}
