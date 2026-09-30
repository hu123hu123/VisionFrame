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
#include <QToolButton>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QAction>
#include <QDockWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>
#include <QFileInfo>

VisionFrame::VisionFrame(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::VisionFrame)
{
    ui->setupUi(this);

    // 蓝图场景与视图绑定
    m_editorScene = new NodeEditorScene(this);
    ui->m_editorView->setScene(m_editorScene);

    // 编辑器与图像显示左右分栏
    splitDockWidget(ui->m_editorDock, ui->m_displayDock, Qt::Horizontal);
    resizeDocks({ ui->m_editorDock, ui->m_displayDock }, { 640, 360 }, Qt::Horizontal);

    // 页面分组切换
    connect(ui->actionTogglePalette, &QAction::toggled, ui->m_paletteDock, &QDockWidget::setVisible);
    connect(ui->actionToggleEditor, &QAction::toggled, ui->m_editorDock, &QDockWidget::setVisible);
    connect(ui->actionToggleDisplay, &QAction::toggled, ui->m_displayDock, &QDockWidget::setVisible);

    // 设置 / 布局管理 下拉菜单
    auto* settingsMenu = new QMenu(this);
    settingsMenu->addAction(ui->actionNewProject);
    settingsMenu->addAction(ui->actionOpenProject);
    settingsMenu->addAction(ui->actionSaveProject);
    ui->m_settingsBtn->setMenu(settingsMenu);

    auto* layoutMenu = new QMenu(this);
    layoutMenu->addAction(ui->actionTogglePalette);
    layoutMenu->addAction(ui->actionToggleEditor);
    layoutMenu->addAction(ui->actionToggleDisplay);
    ui->m_layoutBtn->setMenu(layoutMenu);

    // 标题栏下方：项目功能块（弹簧 + 运行/停止大按钮，定义在 VisonFrame.ui 的 m_runBar）
    connect(ui->m_runToggleBtn, &QPushButton::clicked, this, &VisionFrame::onRunToggle);

    // 蓝图顶部任务行按钮绑定到既有 action（自动跟随 applyMode 启用状态）
    ui->m_addTaskBtn->setDefaultAction(ui->actionAddTask);
    ui->m_delTaskBtn->setDefaultAction(ui->actionDelTask);
    ui->m_stepBtn->setDefaultAction(ui->actionStep);

    // 文件操作
    connect(ui->actionNewProject, &QAction::triggered, this, &VisionFrame::onNewProject);
    connect(ui->actionOpenProject, &QAction::triggered, this, &VisionFrame::onOpenProject);
    connect(ui->actionSaveProject, &QAction::triggered, this, &VisionFrame::onSaveProject);

    // 任务管理
    connect(ui->actionAddTask, &QAction::triggered, this, &VisionFrame::onAddTask);
    connect(ui->actionDelTask, &QAction::triggered, this, &VisionFrame::onRemoveTask);
    connect(ui->m_taskCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &VisionFrame::onTaskSelectionChanged);
    ui->m_taskCombo->setEditable(true);
    ui->m_taskCombo->setInsertPolicy(QComboBox::NoInsert);
    connect(ui->m_taskCombo->lineEdit(), &QLineEdit::editingFinished, this, &VisionFrame::onTaskRename);

    // 运行控制
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
        addTaskInternal(uniqueTaskName(QStringLiteral("Task")));

    refreshTaskCombo();
    updateWindowTitle();
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
    ui->m_taskCombo->setEnabled(editMode);
    ui->actionAddTask->setEnabled(editMode);
    ui->actionDelTask->setEnabled(editMode);
    ui->actionNewProject->setEnabled(editMode);
    ui->actionOpenProject->setEnabled(editMode);
    ui->actionSaveProject->setEnabled(editMode);
    ui->actionStep->setEnabled(editMode);
    ui->m_runToggleBtn->setText(editMode ? QString::fromUtf8("运行") : QString::fromUtf8("停止"));
}

QVector<TaskItem*> VisionFrame::allTasks() const
{
    return ProjectConfig::instance().taskItems;
}

TaskItem* VisionFrame::addTaskInternal(const QString& name)
{
    auto* task = new TaskItem();
    task->setTaskName(name);
    TaskController::instance().AddTask(task);
    return task;
}

void VisionFrame::refreshTaskCombo(TaskItem* selectTask)
{
    ui->m_taskCombo->blockSignals(true);
    ui->m_taskCombo->clear();
    const auto& items = ProjectConfig::instance().taskItems;
    for (TaskItem* t : items)
        ui->m_taskCombo->addItem(t->taskName());
    ui->m_taskCombo->blockSignals(false);

    int idx = selectTask ? items.indexOf(selectTask) : 0;
    if (idx < 0)
        idx = 0;
    if (!items.isEmpty())
        ui->m_taskCombo->setCurrentIndex(idx);
    onTaskSelectionChanged(ui->m_taskCombo->currentIndex());
}

void VisionFrame::bindToTask(TaskItem* task)
{
    m_editorScene->setGraph(task ? &task->graph() : nullptr);
    ui->m_display->refreshTasks(allTasks(), task);
}

QString VisionFrame::uniqueTaskName(const QString& base) const
{
    const auto& items = allTasks();
    auto exists = [&items](const QString& name) {
        for (const TaskItem* t : items)
            if (t->taskName() == name)
                return true;
        return false;
    };

    if (!exists(base))
        return base;

    int counter = 2;
    while (true) {
        const QString candidate = base + QStringLiteral("_") + QString::number(counter++);
        if (!exists(candidate))
            return candidate;
    }
}

void VisionFrame::updateWindowTitle()
{
    const QString name = ProjectConfig::instance().projectName().trimmed();
    setWindowTitle(name.isEmpty() ? QStringLiteral("VisionFrame")
                                  : QStringLiteral("VisionFrame - %1").arg(name));
}

void VisionFrame::onAddTask()
{
    refreshTaskCombo(addTaskInternal(uniqueTaskName(QStringLiteral("Task"))));
}

void VisionFrame::onRemoveTask()
{
    const int idx = ui->m_taskCombo->currentIndex();
    const auto& items = ProjectConfig::instance().taskItems;
    if (idx < 0 || idx >= items.size())
        return;

    if (items.size() <= 1) {
        QMessageBox::warning(this, QString::fromUtf8("提示"), QString::fromUtf8("至少保留一个任务。"));
        return;
    }

    TaskItem* t = items[idx];
    if (QMessageBox::question(this, QString::fromUtf8("确认"),
            QString::fromUtf8("确定删除任务“%1”吗？").arg(t->taskName())) != QMessageBox::Yes)
        return;

    if (m_currentTask == t) {
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

void VisionFrame::onTaskRename()
{
    if (!m_currentTask)
        return;

    const QString newName = ui->m_taskCombo->currentText().trimmed();
    if (newName.isEmpty() || newName == m_currentTask->taskName()) {
        refreshTaskCombo(m_currentTask);
        return;
    }

    for (const TaskItem* t : allTasks()) {
        if (t != m_currentTask && t->taskName() == newName) {
            QMessageBox::warning(this, QString::fromUtf8("提示"),
                QString::fromUtf8("任务名“%1”已存在。").arg(newName));
            refreshTaskCombo(m_currentTask);
            return;
        }
    }

    m_currentTask->setTaskName(newName);
    ui->m_taskCombo->setItemText(ui->m_taskCombo->currentIndex(), newName);
    ui->m_display->refreshTasks(allTasks(), m_currentTask);
}

void VisionFrame::onRunToggle()
{
    if (m_running)
        TaskController::instance().StopAllTasks();
    else
        TaskController::instance().StartAllTasks();
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
    bool running = false;
    for (const TaskItem* t : allTasks()) {
        if (t->isRunning()) { running = true; break; }
    }
    if (running != m_running) {
        m_running = running;
        applyMode(!running);
    }

    if (!m_currentTask)
        return;
    m_editorScene->applyRunStates(m_currentTask->nodeRunStates());
}

void VisionFrame::onNewProject()
{
    TaskController::instance().StopAllTasks();
    TaskController::instance().ClearAllTasks();
    applyMode(true);

    addTaskInternal(uniqueTaskName(QStringLiteral("Task")));
    m_projectFilePath.clear();
    ProjectConfig::instance().setProjectName(QString());
    refreshTaskCombo();
    updateWindowTitle();
}

void VisionFrame::onOpenProject()
{
    const QString p = QFileDialog::getOpenFileName(this,
        QString::fromUtf8("打开项目"), QString(), QString::fromUtf8("VisionFrame 项目 (*.vfProj)"));
    if (p.isEmpty())
        return;

    TaskController::instance().StopAllTasks();
    applyMode(true);
    m_editorScene->setGraph(nullptr);

    if (ProjectConfig::instance().loadProject(p)) {
        m_projectFilePath = p;
        ProjectConfig::instance().setProjectName(QFileInfo(p).completeBaseName());
        refreshTaskCombo();
        updateWindowTitle();
    }
}

void VisionFrame::onSaveProject()
{
    QString p = m_projectFilePath;
    if (p.isEmpty())
        p = QFileDialog::getSaveFileName(this,
            QString::fromUtf8("保存项目"), QString(), QString::fromUtf8("VisionFrame 项目 (*.vfProj)"));
    if (p.isEmpty())
        return;

    if (ProjectConfig::instance().saveProject(p)) {
        m_projectFilePath = p;
        ProjectConfig::instance().setProjectName(QFileInfo(p).completeBaseName());
        updateWindowTitle();
    }
}