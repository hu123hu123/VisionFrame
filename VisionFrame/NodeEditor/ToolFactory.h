#pragma once
#include <QString>
#include <QVector>
#include <functional>

class QWidget;
class FrameToolBase;

// 工具类型注册项：类型标识 + 显示名 + 分类 + 创建函数 + 参数控件工厂。
struct ToolEntry
{
    QString typeId;
    QString displayName;
    QString category;
    std::function<FrameToolBase*()> create;
    std::function<class QWidget*(FrameToolBase*)> createWidget; // 可为 null
};

// 全局工具注册表（单例），把工具创建与参数控件从 UI 中解耦。
class ToolFactory
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

// 注册全部内置工具（main 启动时调用）。
void registerBuiltinTools();