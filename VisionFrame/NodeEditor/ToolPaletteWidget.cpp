#include "ToolPaletteWidget.h"
#include "ToolFactory.h"
#include <QMimeData>
#include <QTreeWidgetItem>

ToolPaletteWidget::ToolPaletteWidget(QWidget* parent)
    : QTreeWidget(parent)
{
    setHeaderHidden(true);
    setDragEnabled(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setUniformRowHeights(true);
    reload();
    connect(this, &QTreeWidget::itemDoubleClicked, this, &ToolPaletteWidget::onItemDoubleClicked);
}

void ToolPaletteWidget::reload()
{
    clear();
    const QVector<QString> cats = ToolFactory::instance().categories();
    for (const QString& cat : cats)
    {
        auto* top = new QTreeWidgetItem(this, { cat });
        top->setFlags(Qt::ItemIsEnabled);
        for (const auto& e : ToolFactory::instance().tools())
        {
            if (e.category != cat)
                continue;
            auto* child = new QTreeWidgetItem(top, { e.displayName });
            child->setData(0, Qt::UserRole, e.typeId);
            child->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
        }
        addTopLevelItem(top);
        top->setExpanded(true);
    }
}

void ToolPaletteWidget::onItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column);
    if (!item)
        return;
    const QString typeId = item->data(0, Qt::UserRole).toString();
    if (!typeId.isEmpty())
        emit toolActivated(typeId);
}

QMimeData* ToolPaletteWidget::mimeData(const QList<QTreeWidgetItem*> items) const
{
    if (items.isEmpty())
        return nullptr;
    const QString typeId = items.first()->data(0, Qt::UserRole).toString();
    if (typeId.isEmpty())
        return nullptr;
    auto* md = new QMimeData();
    md->setData(QStringLiteral("application/x-vf-tool"), typeId.toUtf8());
    return md;
}