#pragma once

#include <QMainWindow>

class NodeEditorScene;
class QTimer;
class TaskItem;

namespace Ui {
class VisionFrame;
}

// 主窗口：左侧工具面板 + 中央蓝图编辑器 + 右侧图像显示，支持编辑/运行切换与 dock 收放。
// 页面布局（dock + 工具栏）在 VisonFrame.ui 中定义，这里只保留运行逻辑相关成员。
class VisionFrame : public QMainWindow
{
    Q_OBJECT
public:
    explicit VisionFrame(QWidget* parent = nullptr);
    ~VisionFrame() override;

private slots:
    void onAddTask();
    void onRemoveTask();
    void onTaskSelectionChanged(int idx);
    void onRunToggle();
    void onStepRun();
    void onToolActivated(const QString& typeId);
    void onToolDropped(const QString& typeId, const QPointF& scenePos);
    void onNodeEditRequested(const QString& nodeId);
    void onHighlightTick();
    void onOpenProject();
    void onSaveProject();
    void onNewProject();
    void onTaskRename();

private:
    void applyMode(bool editMode);
    void bindToTask(TaskItem* task);
    void refreshTaskCombo(TaskItem* selectTask = nullptr);
    TaskItem* addTaskInternal(const QString& name);
    QVector<TaskItem*> allTasks() const;
    void updateWindowTitle();
    QString uniqueTaskName(const QString& base) const;

    Ui::VisionFrame* ui{ nullptr };
    NodeEditorScene*  m_editorScene{ nullptr };
    QTimer*           m_highlightTimer{ nullptr };
    TaskItem*         m_currentTask{ nullptr };
    QString           m_projectFilePath;
    bool              m_running{ false };
};