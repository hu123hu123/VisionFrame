#include "TaskStatusPanel.h"
#include "TaskItem.h"

#include <QTreeWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QSet>

namespace {

QString stateText(NodeRunState s)
{
    switch (s)
    {
    case NodeRunState::Running: return QString::fromUtf8("运行中");
    case NodeRunState::Success: return QString::fromUtf8("成功");
    case NodeRunState::Failed:  return QString::fromUtf8("失败");
    case NodeRunState::Idle:
    default:                    return QString::fromUtf8("空闲");
    }
}

QColor stateColor(NodeRunState s)
{
    switch (s)
    {
    case NodeRunState::Running: return QColor(46, 125, 50);   // 绿
    case NodeRunState::Success: return QColor(21, 101, 192);  // 蓝
    case NodeRunState::Failed:  return QColor(198, 40, 40);   // 红
    case NodeRunState::Idle:
    default:                    return QColor(144, 144, 144); // 灰
    }
}

} // namespace

TaskStatusPanel::TaskStatusPanel(QWidget* parent)
    : QWidget(parent)
{
    m_tree = new QTreeWidget(this);
    m_tree->setColumnCount(2);
    m_tree->setHeaderLabels(QStringList() << QString::fromUtf8("任务 / 工具") << QString::fromUtf8("状态"));
    m_tree->setRootIsDecorated(true);
    m_tree->setAlternatingRowColors(true);
    m_tree->setUniformRowHeights(true);

    QHeaderView* header = m_tree->header();
    header->setStretchLastSection(false);
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(m_tree);
}

void TaskStatusPanel::refresh(const QVector<TaskItem*>& tasks)
{
    // 数据未变化时跳过重建，避免高频刷新下的闪烁与折叠状态丢失。
    QString signature;
    for (const TaskItem* t : tasks)
    {
        signature += t->taskName();
        signature += QLatin1Char(t->isRunning() ? 'R' : 'S');
        const auto tools = t->toolRunStates();
        for (const auto& ts : tools)
        {
            signature += QLatin1Char('\x1e');
            signature += ts.name;
            signature += QString::number(int(ts.state));
        }
        signature += QLatin1Char('\x1f');
    }
    if (signature == m_signature)
        return;

    QSet<QString> expanded;
    for (int i = 0; i < m_tree->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* item = m_tree->topLevelItem(i);
        if (item->isExpanded())
            expanded.insert(item->text(0));
    }

    m_tree->clear();

    const QColor runningColor(46, 125, 50);
    const QColor stoppedColor(144, 144, 144);

    for (const TaskItem* t : tasks)
    {
        auto* taskItem = new QTreeWidgetItem(m_tree);
        taskItem->setText(0, t->taskName());
        taskItem->setText(1, t->isRunning() ? QString::fromUtf8("运行中") : QString::fromUtf8("已停止"));
        taskItem->setForeground(1, QBrush(t->isRunning() ? runningColor : stoppedColor));

        QFont tf = taskItem->font(0);
        tf.setBold(true);
        taskItem->setFont(0, tf);

        const auto tools = t->toolRunStates();
        for (const auto& ts : tools)
        {
            auto* toolItem = new QTreeWidgetItem(taskItem);
            toolItem->setText(0, ts.name);
            toolItem->setText(1, stateText(ts.state));
            toolItem->setForeground(1, QBrush(stateColor(ts.state)));
        }

        taskItem->setExpanded(expanded.contains(t->taskName()));
    }

    m_signature = signature;
}