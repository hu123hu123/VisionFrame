#include "VisonFrame.h"
#include "ui_VisonFrame.h"

#include "NodeEditor/NodeEditorView.h"
#include "NodeEditor/NodeEditorScene.h"
#include "NodeEditor/ToolPaletteWidget.h"
#include "NodeEditor/ToolFactory.h"
#include "Display/ImageDisplayWidget.h"

#include "TaskItem.h"
#include "TaskController.h"
#include "GlobalConfig.h"

#include <QToolBar>
#include <QComboBox>
#include <QPushButton>
#include <QAction>
#include <QActionGroup>
#include <QDockWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QTimer>

VisionFrame::VisionFrame(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::VisionFrame)
{
    ui->setupUi(this);

    // 编辑/运行 互斥切换（QActionGroup 需在代码中建立）
    auto* modeGroup = new QActionGroup(this);
    modeGroup->setExclusive(true);
    modeGroup->addAction(ui->actionEdit);
    modeGroup->addAction(ui->actionRunMode);

    // 蓝图场景与视图绑定
    m_editorScene = new NodeEditorScene(this);
    ui->m_editorView->setScene(m_editorScene);

    // 编辑器与图像显示左右分栏
    splitDockWidget(ui->m_editorDock, ui->m_displayDock, Qt::Horizontal);
    resizeDocks({ ui->m_editorDock, ui->m_displayDock }, { 640, 360 }, Qt::Horizontal);

    // dock 收放（收起进侧边 / 展开）
    ui->mainToolBar->addAction(ui->m_paletteDock->toggleViewAction());
    ui->mainToolBar->addAction(ui->m_editorDock->toggleViewAction());
    ui->mainToolBar->addAction(ui->m_displayDock->toggleViewAction());

    connect(ui->actionEdit, &QAction::toggled, this, [this](bool c) { if (c) applyMode(true); });
    connect(ui->actionRunMode, &QAction::toggled, this, [this](bool c) { if (c) applyMode(false); });
    connect(ui->actionAddTask, &QAction::triggered, this, &VisionFrame::onAddTask);
    connect(ui->actionDelTask, &QAction::triggered, this, &VisionFrame::onRemoveTask);
    connect(ui->m_taskCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &VisionFrame::onTaskSelectionChanged);
    connect(ui->actionRun, &QAction::triggered, this, &VisionFrame::onStartRun);
    connect(ui->actionStop, &QAction::triggered, this, &VisionFrame::onStopRun);
    connect(ui->actionStep, &QAction::triggered, this, &VisionFrame::onStepRun);

    connect(ui->m_palette, &ToolPaletteWidget::toolActivated, this, &VisionFrame::onToolActivated);
    connect(ui->m_editorView, &NodeEditorView::toolDropped, this, &VisionFrame::onToolDropped);
    connect(m_editorScene, &NodeEditorScene::nodeEditRequested, this, &VisionFrame::onNodeEditRequested);
    connect(m_editorScene, &NodeEditorScene::graphChanged, this, [this]() {
        ui->m_display->refreshCombos();
    });

    m_highlightTimer = new QTimer(this);
    connect(m_highlightTimer, &QTimer::timeout, this, &VisionFrame::onHighlightTick);
    m_highlightTimer->start(30);

    applyMode(true);

    if (ProjectConfig::instance().taskItems.isEmpty())
        addTaskInternal(QString::fromUtf8("Task1"));

    refreshTaskCombo();
}

VisionFrame::~VisionFrame()
{
    m_highlightTimer->stop();
    TaskController::instance().ClearAllTasks();
    delete ui;
}

void VisionFrame::applyMode(bool editMode)
{
    ui->m_editorView->setInteractive(editMode);
    m_editorScene->setEditEnabled(editMode);
    ui->m_palette->setEnabled(editMode);
    ui->actionRun->setVisible(!editMode);
    ui->actionStop->setVisible(!editMode);
    ui->actionStep->setVisible(!editMode);

    if (editMode)
        TaskController::instance().StopAllTasks();
}

QVector<TaskItem*> VisionFrame::allTasks() const
{
    return ProjectConfig::instance().taskItems;
}

void VisionFrame::addTaskInternal(const QString& name)
{
    auto* task = new TaskItem();
    task->setTaskName(name);
    TaskController::instance().AddTask(task);
}

void VisionFrame::refreshTaskCombo()
{
    ui->m_taskCombo->blockSignals(true);
    ui->m_taskCombo->clear();
    const auto& items = ProjectConfig::instance().taskItems;
    for (TaskItem* t : items)
        ui->m_taskCombo->addItem(t->taskName());
    ui->m_taskCombo->blockSignals(false);

    if (!items.isEmpty())
        ui->m_taskCombo->setCurrentIndex(0);
    onTaskSelectionChanged(ui->m_taskCombo->currentIndex());
}

void VisionFrame::bindToTask(TaskItem* task)
{
    m_editorScene->setGraph(task ? &task->graph() : nullptr);
    ui->m_display->refreshTasks(allTasks(), task);
}

void VisionFrame::onAddTask()
{
    const int n = ProjectConfig::instance().taskItems.size() + 1;
    addTaskInternal(QString("Task%1").arg(n));
    refreshTaskCombo();
}

void VisionFrame::onRemoveTask()
{
    const int idx = ui->m_taskCombo->currentIndex();
    const auto& items = ProjectConfig::instance().taskItems;
    if (idx < 0 || idx >= items.size())
        return;

    TaskItem* t = items[idx];
    if (m_currentTask == t)
    {
        m_currentTask = nullptr;
        m_editorScene->setGraph(nullptr);
    }
    TaskController::instance().RemoveTask(t);
    refreshTaskCombo();
}

void VisionFrame::onTaskSelectionChanged(int idx)
{
    const auto& items = ProjectConfig::instance().taskItems;
    m_currentTask = (idx >= 0 && idx < items.size()) ? items[idx] : nullptr;
    bindToTask(m_currentTask);
}

void VisionFrame::onStartRun()
{
    if (m_currentTask)
        m_currentTask->StartTask();
}

void VisionFrame::onStopRun()
{
    TaskController::instance().StopAllTasks();
}

void VisionFrame::onStepRun()
{
    if (m_currentTask && !m_currentTask->isRunning())
        m_currentTask->runOnce();
}

void VisionFrame::onToolActivated(const QString& typeId)
{
    m_editorScene->addNode(typeId, ui->m_editorView->sceneCenter());
}

void VisionFrame::onToolDropped(const QString& typeId, const QPointF& scenePos)
{
    m_editorScene->addNode(typeId, scenePos);
}

void VisionFrame::onNodeEditRequested(const QString& nodeId)
{
    if (!m_currentTask)
        return;
    GraphNodeData* node = m_currentTask->graph().findNode(nodeId);
    if (!node || !node->tool)
        return;

    const ToolEntry* e = ToolFactory::instance().find(node->toolTypeId);
    if (!e)
        return;
    if (!e->createWidget)
    {
        QMessageBox::information(this, QString::fromUtf8("参数"), QString::fromUtf8("该工具无参数。"));
        return;
    }

    QWidget* w = e->createWidget(node->tool);
    if (!w)
        return;

    QDialog dlg(this);
    dlg.setWindowTitle(node->title);
    auto* lay = new QVBoxLayout(&dlg);
    lay->addWidget(w);
    auto* closeBtn = new QPushButton(QString::fromUtf8("关闭"), &dlg);
    lay->addWidget(closeBtn);
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    dlg.exec();
}

void VisionFrame::onHighlightTick()
{
    if (!m_currentTask)
        return;
    m_editorScene->applyRunStates(m_currentTask->nodeRunStates());
}