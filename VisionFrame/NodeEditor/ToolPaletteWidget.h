#pragma once

#include <QTreeWidget>

// 工具面板：按分类分组列出工具，支持双击添加与拖拽到画布。
class ToolPaletteWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit ToolPaletteWidget(QWidget* parent = nullptr);
    void reload();

signals:
    void toolActivated(const QString& typeId);

protected:
    QMimeData* mimeData(const QList<QTreeWidgetItem*> items) const override;

private:
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);
};