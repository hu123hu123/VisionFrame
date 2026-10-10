#pragma once

#include <QWidget>
#include <QVector>
#include <QString>

class QTreeWidget;
class TaskItem;

// “运行状态”面板：树形展示 任务 -> 工具 -> 当前状态（空闲/运行中/成功/失败）。
class TaskStatusPanel : public QWidget
{
    Q_OBJECT
public:
    explicit TaskStatusPanel(QWidget* parent = nullptr);

    // 用任务列表刷新视图（由主窗口定时器周期调用）。
    void refresh(const QVector<TaskItem*>& tasks);

private:
    QTreeWidget* m_tree{ nullptr };
    QString      m_signature;   // 上次渲染快照签名，数据未变时跳过重建
};