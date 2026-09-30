#include "ImageViewWidget.h"

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QPixmap>
#include <opencv2/opencv.hpp>

static const QColor kBackground(0x1c1c1c);

ImageViewWidget::ImageViewWidget(QWidget* parent)
    : QGraphicsView(parent)
{
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);

    m_pixmapItem = new QGraphicsPixmapItem();
    m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
    m_scene->addItem(m_pixmapItem);

    setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    setBackgroundBrush(kBackground);
}

void ImageViewWidget::setImage(const QImage& img)
{
    const bool sizeChanged = m_image.size() != img.size();
    m_image = img;

    if (img.isNull())
    {
        m_pixmapItem->setPixmap(QPixmap());
        m_scene->setSceneRect(QRectF());
    }
    else
    {
        m_pixmapItem->setPixmap(QPixmap::fromImage(img));
        const QRectF r = m_pixmapItem->boundingRect();
        if (m_scene->sceneRect() != r)
            m_scene->setSceneRect(r);
    }

    if (sizeChanged)
    {
        m_autoFit = true;
        fitToWindow();
    }
    viewport()->update();
}

void ImageViewWidget::clearImage()
{
    setImage(QImage());
}

bool ImageViewWidget::hasImage() const
{
    return !m_image.isNull();
}

void ImageViewWidget::fitToWindow()
{
    if (m_image.isNull())
        return;
    fitInView(m_pixmapItem, Qt::KeepAspectRatio);
}

void ImageViewWidget::setZoom(qreal scale)
{
    if (scale <= 0)
        return;
    m_autoFit = false;
    resetTransform();
    QGraphicsView::scale(scale, scale);
}

QRectF ImageViewWidget::imageRect() const
{
    return m_pixmapItem->boundingRect();
}

QPointF ImageViewWidget::mapToImage(const QPointF& scenePos) const
{
    return m_pixmapItem->mapFromScene(scenePos);
}

QPointF ImageViewWidget::mapFromImage(const QPointF& imagePos) const
{
    return m_pixmapItem->mapToScene(imagePos);
}

void ImageViewWidget::wheelEvent(QWheelEvent* e)
{
    m_autoFit = false;
    const double factor = (e->angleDelta().y() > 0) ? 1.15 : (1.0 / 1.15);
    scale(factor, factor);
    e->accept();
}

void ImageViewWidget::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::MiddleButton)
    {
        m_autoFit = false;
        m_panning = true;
        m_lastPan = e->pos();
        viewport()->setCursor(Qt::ClosedHandCursor);
        e->accept();
        return;
    }
    QGraphicsView::mousePressEvent(e);
}

void ImageViewWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (m_panning)
    {
        const QPoint d = e->pos() - m_lastPan;
        m_lastPan = e->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - d.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - d.y());
        e->accept();
        return;
    }
    QGraphicsView::mouseMoveEvent(e);
}

void ImageViewWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() == Qt::MiddleButton && m_panning)
    {
        m_panning = false;
        viewport()->unsetCursor();
        e->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(e);
}

void ImageViewWidget::resizeEvent(QResizeEvent* e)
{
    QGraphicsView::resizeEvent(e);
    if (m_autoFit)
        fitToWindow();
}

QImage matToQImage(const cv::Mat& m)
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