#pragma once
#include <QString>
#include <QVector>
#include <functional>
#include "FrameToolRegistryExport.h"

class QWidget;
class FrameToolBase;

// 工具类型注册项：类型标识 + 显示名 + 分类 + 创建函数 + 参数控件工厂。
// 由各工具插件 DLL 在 vfRegisterTools 回调中填充并注册。
struct ToolEntry
{
    QString typeId;
    QString displayName;
    QString category;
    std::function<FrameToolBase*()> create;
    std::function<class QWidget*(FrameToolBase*)> createWidget; // 可为 null
};

// 全局工具注册表（单例），把工具创建与参数控件从 UI 中解耦。
// 宿主与插件链接同一 FrameToolRegistry.dll，保证唯一实例。
class FRAMETOOLREGISTRY_API ToolFactory
{
public:
    static ToolFactory& instance();

    void registerTool(const ToolEntry& e);
    const QVector<ToolEntry>& tools() const;
    const ToolEntry* find(const QString& typeId) const;
    FrameToolBase* create(const QString& typeId) const;
    QVector<QString> categories() const;
    QString displayName(const QString& typeId) const;

private:
    ToolFactory() = default;
    ToolFactory(const ToolFactory&) = delete;
    ToolFactory& operator=(const ToolFactory&) = delete;

    QVector<ToolEntry> m_tools;
};
