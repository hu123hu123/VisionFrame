#include "RoiImageViewWidget.h"
#include "RectRoiItem.h"

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QMouseEvent>

namespace {
constexpr qreal kMinRoiSize = 4.0;
}

RoiImageViewWidget::RoiImageViewWidget(QWidget* parent)
    : ImageViewWidget(parent)
{
}

QPointF RoiImageViewWidget::clampToImage(const QPointF& p) const
{
    const QRectF r = imageRect();
    if (r.isNull())
        return p;
    QPointF c = p;
    c.setX(qBound(r.left(), c.x(), r.right()));
    c.setY(qBound(r.top(), c.y(), r.bottom()));
    return c;
}

void RoiImageViewWidget::setRoi(const QRectF& imageRect)
{
    if (imageRect.isNull())
    {
        clearRoi();
        return;
    }
    if (!m_roi)
    {
        m_roi = new RectRoiItem(QRectF());
        scene()->addItem(m_roi);
        m_roi->setZValue(10);
        connect(m_roi, &RectRoiItem::changed, this, [this] {
            if (m_roi)
                emit roiChanged(m_roi->roiRect());
        });
    }
    m_roi->setRect(imageRect);
}

QRectF RoiImageViewWidget::roiRect() const
{
    return m_roi ? m_roi->roiRect() : QRectF();
}

void RoiImageViewWidget::clearRoi()
{
    if (m_roi)
    {
        scene()->removeItem(m_roi);
        m_roi->deleteLater();
        m_roi = nullptr;
    }
}

void RoiImageViewWidget::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton && hasImage())
    {
        QGraphicsItem* it = itemAt(e->pos());
        if (m_roi == nullptr || it != m_roi)
        {
            // 空白处按下：开始绘制新 ROI（替换旧 ROI）
            clearRoi();
            m_drawing = true;
            m_drawStart = clampToImage(mapToScene(e->pos()));
            m_roi = new RectRoiItem(QRectF(m_drawStart, m_drawStart));
            scene()->addItem(m_roi);
            m_roi->setZValue(10);
            connect(m_roi, &RectRoiItem::changed, this, [this] {
                if (m_roi)
                    emit roiChanged(m_roi->roiRect());
            });
            e->accept();
            return;
        }
    }
    ImageViewWidget::mousePressEvent(e);
}

void RoiImageViewWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (m_drawing && m_roi && (e->buttons() & Qt::LeftButton))
    {
        const QPointF cur = clampToImage(mapToScene(e->pos()));
        m_roi->setRect(QRectF(m_drawStart, cur).normalized());
        e->accept();
        return;
    }
    ImageViewWidget::mouseMoveEvent(e);
}

void RoiImageViewWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (m_drawing && m_roi && e->button() == Qt::LeftButton)
    {
        m_drawing = false;
        const QRectF r = m_roi->roiRect();
        if (r.width() < kMinRoiSize || r.height() < kMinRoiSize)
        {
            clearRoi();
            emit roiChanged(QRectF());
        }
        else
        {
            emit roiChanged(r);
        }
        e->accept();
        return;
    }
    ImageViewWidget::mouseReleaseEvent(e);
}