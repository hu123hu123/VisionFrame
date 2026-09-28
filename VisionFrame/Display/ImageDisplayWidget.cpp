#include "ImageDisplayWidget.h"
#include "TaskItem.h"
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>
#include <QTimer>
#include <opencv2/opencv.hpp>

// 专用图像渲染区（独立于下拉控件，避免图像被控件遮挡）。
class ImageView : public QWidget
{
public:
    using QWidget::QWidget;

    void setImage(const QImage& img)
    {
        m_image = img;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.fillRect(rect(), QColor(0x1c1c1c));
        if (m_image.isNull())
        {
            p.setPen(QColor(0x888888));
            p.drawText(rect(), Qt::AlignCenter, QString::fromUtf8("无图像数据"));
            return;
        }
        QSize scaled = m_image.size();
        scaled.scale(size(), Qt::KeepAspectRatio);
        const QRect r((width() - scaled.width()) / 2, (height() - scaled.height()) / 2,
            scaled.width(), scaled.height());
        p.drawImage(r, m_image);
    }

private:
    QImage m_image;
};

static QImage matToQImage(const cv::Mat& m)
{
    if (m.empty())
        return QImage();
    if (m.type() == CV_8UC3)
    {
        QImage img(m.data, m.cols, m.rows, static_cast<int>(m.step), QImage::Format_BGR888);
        return img.copy();
    }
    if (m.type() == CV_8UC1)
    {
        QImage img(m.data, m.cols, m.rows, static_cast<int>(m.step), QImage::Format_Grayscale8);
        return img.copy();
    }
    cv::Mat c;
    m.convertTo(c, CV_8UC3);
    QImage img(c.data, c.cols, c.rows, static_cast<int>(c.step), QImage::Format_BGR888);
    return img.copy();
}

ImageDisplayWidget::ImageDisplayWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(6, 6, 6, 6);
    lay->setSpacing(6);

    m_taskCombo = new QComboBox(this);
    m_toolCombo = new QComboBox(this);
    m_outputCombo = new QComboBox(this);

    lay->addWidget(m_taskCombo);
    lay->addWidget(m_toolCombo);
    lay->addWidget(m_outputCombo);

    m_view = new ImageView(this);
    lay->addWidget(m_view, 1);

    connect(m_taskCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &ImageDisplayWidget::onTaskChanged);
    connect(m_toolCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &ImageDisplayWidget::onToolChanged);
    connect(m_outputCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [this](int) { reloadPixmap(); });

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ImageDisplayWidget::reloadPixmap);
    m_timer->start(33);
}

void ImageDisplayWidget::setTasks(const QVector<TaskItem*>& tasks)
{
    m_tasks = tasks;
    rebuildToolCombo();
}

void ImageDisplayWidget::refreshTasks(const QVector<TaskItem*>& tasks, TaskItem* current)
{
    m_tasks = tasks;
    m_taskCombo->blockSignals(true);
    m_taskCombo->clear();
    for (TaskItem* t : tasks)
        m_taskCombo->addItem(t->taskName());
    if (current)
    {
        const int idx = tasks.indexOf(current);
        if (idx >= 0)
            m_taskCombo->setCurrentIndex(idx);
    }
    m_taskCombo->blockSignals(false);
    rebuildToolCombo();
}

void ImageDisplayWidget::refreshCombos()
{
    rebuildToolCombo();
}

TaskItem* ImageDisplayWidget::currentTask() const
{
    const int i = m_taskCombo->currentIndex();
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
    m_toolCombo->blockSignals(true);
    m_toolCombo->clear();
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
                m_toolCombo->addItem(n.title, n.id);
        }
    }
    m_toolCombo->blockSignals(false);
    rebuildOutputCombo();
    reloadPixmap();
}

void ImageDisplayWidget::rebuildOutputCombo()
{
    m_outputCombo->blockSignals(true);
    m_outputCombo->clear();
    TaskItem* t = currentTask();
    const QString nodeId = m_toolCombo->currentData().toString();
    if (t && !nodeId.isEmpty())
    {
        const GraphNodeData* n = t->graph().findNode(nodeId);
        if (n && n->tool)
        {
            std::vector<FPort> ports;
            n->tool->GetPorts(ports);
            for (const auto& p : ports)
                if (!p.isInput && p.type == PinType::Image)
                    m_outputCombo->addItem(QString::fromStdString(p.name), QString::fromStdString(p.name));
        }
    }
    m_outputCombo->blockSignals(false);
}

void ImageDisplayWidget::reloadPixmap()
{
    TaskItem* t = currentTask();
    const QString nodeId = m_toolCombo->currentData().toString();
    const QString outName = m_outputCombo->currentData().toString();

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

    if (img.isNull())
        m_view->setImage(QImage());
    else
        m_view->setImage(img);
}