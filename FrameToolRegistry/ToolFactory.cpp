#include "ToolFactory.h"

ToolFactory& ToolFactory::instance()
{
    static ToolFactory s;
    return s;
}

void ToolFactory::registerTool(const ToolEntry& e)
{
    m_tools.append(e);
}

const QVector<ToolEntry>& ToolFactory::tools() const
{
    return m_tools;
}

const ToolEntry* ToolFactory::find(const QString& typeId) const
{
    for (const auto& e : m_tools)
        if (e.typeId == typeId)
            return &e;
    return nullptr;
}

FrameToolBase* ToolFactory::create(const QString& typeId) const
{
    const ToolEntry* e = find(typeId);
    return (e && e->create) ? e->create() : nullptr;
}

QVector<QString> ToolFactory::categories() const
{
    QVector<QString> out;
    for (const auto& e : m_tools)
        if (!out.contains(e.category))
            out.append(e.category);
    return out;
}

QString ToolFactory::displayName(const QString& typeId) const
{
    const ToolEntry* e = find(typeId);
    return e ? e->displayName : typeId;
}
