#include "ImageDisplayWidget.h"
#include "ui_ImageDisplayWidget.h"
#include "ImageViewWidget.h"
#include "TaskItem.h"

#include <QComboBox>
#include <QToolButton>
#include <QTimer>
#include <opencv2/opencv.hpp>

ImageDisplayWidget::ImageDisplayWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ImageDisplayWidget)
{
    ui->setupUi(this);

    connect(ui->m_taskCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &ImageDisplayWidget::onTaskChanged);
    connect(ui->m_toolCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &ImageDisplayWidget::onToolChanged);
    connect(ui->m_outputCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [this](int) { reloadPixmap(); });

    // 选择区可隐藏：勾选 = 显示，取消 = 隐藏（图像区占满）
    connect(ui->m_toggleSelector, &QToolButton::toggled, this, [this](bool checked) {
        ui->m_selectorBar->setVisible(checked);
        ui->m_toggleSelector->setArrowType(checked ? Qt::DownArrow : Qt::RightArrow);
    });

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ImageDisplayWidget::reloadPixmap);
    m_timer->start(33);
}

ImageDisplayWidget::~ImageDisplayWidget() = default;

void ImageDisplayWidget::setTasks(const QVector<TaskItem*>& tasks)
{
    m_tasks = tasks;
    rebuildToolCombo();
}

void ImageDisplayWidget::refreshTasks(const QVector<TaskItem*>& tasks, TaskItem* current)
{
    m_tasks = tasks;
    ui->m_taskCombo->blockSignals(true);
    ui->m_taskCombo->clear();
    for (TaskItem* t : tasks)
        ui->m_taskCombo->addItem(t->taskName());
    if (current)
    {
        const int idx = tasks.indexOf(current);
        if (idx >= 0)
            ui->m_taskCombo->setCurrentIndex(idx);
    }
    ui->m_taskCombo->blockSignals(false);
    rebuildToolCombo();
}

void ImageDisplayWidget::refreshCombos()
{
    rebuildToolCombo();
}

TaskItem* ImageDisplayWidget::currentTask() const
{
    const int i = ui->m_taskCombo->currentIndex();
    return (i >= 0 && i < m_tasks.size()) ? m_tasks[i] : nullptr;
}

void ImageDisplayWidget::onTaskChanged(int)
{
    rebuildToolCombo();
}

void ImageDisplayWidget::onToolChanged(int)
{
    rebuildOutputCombo();
    reloadPixmap();
}

void ImageDisplayWidget::rebuildToolCombo()
{
    ui->m_toolCombo->blockSignals(true);
    ui->m_toolCombo->clear();
    TaskItem* t = currentTask();
    if (t)
    {
        const auto& nodes = t->graph().nodes;
        for (const auto& n : nodes)
        {
            if (!n.tool)
                continue;
            std::vector<FPort> ports;
            n.tool->GetPorts(ports);
            bool hasImageOut = false;
            for (const auto& p : ports)
                if (!p.isInput && p.type == PinType::Image)
                {
                    hasImageOut = true;
                    break;
                }
            if (hasImageOut)
                ui->m_toolCombo->addItem(n.title, n.id);
        }
    }
    ui->m_toolCombo->blockSignals(false);
    rebuildOutputCombo();
    reloadPixmap();
}

void ImageDisplayWidget::rebuildOutputCombo()
{
    ui->m_outputCombo->blockSignals(true);
    ui->m_outputCombo->clear();
    TaskItem* t = currentTask();
    const QString nodeId = ui->m_toolCombo->currentData().toString();
    if (t && !nodeId.isEmpty())
    {
        const GraphNodeData* n = t->graph().findNode(nodeId);
        if (n && n->tool)
        {
            std::vector<FPort> ports;
            n->tool->GetPorts(ports);
            for (const auto& p : ports)
                if (!p.isInput && p.type == PinType::Image)
                    ui->m_outputCombo->addItem(QString::fromStdString(p.name), QString::fromStdString(p.name));
        }
    }
    ui->m_outputCombo->blockSignals(false);
}

void ImageDisplayWidget::reloadPixmap()
{
    TaskItem* t = currentTask();
    const QString nodeId = ui->m_toolCombo->currentData().toString();
    const QString outName = ui->m_outputCombo->currentData().toString();

    QImage img;
    if (t && !nodeId.isEmpty() && !outName.isEmpty())
    {
        std::map<std::string, NodeData> outs;
        if (t->snapshotNodeOutputs(nodeId, outs))
        {
            auto it = outs.find(outName.toStdString());
            if (it != outs.end())
            {
                const cv::Mat* m = std::any_cast<cv::Mat>(&it->second);
                if (m && !m->empty())
                    img = matToQImage(*m);
            }
        }
    }

    ui->m_view->setImage(img);
}