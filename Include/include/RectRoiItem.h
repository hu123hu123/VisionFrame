#pragma once

#include "FrameImageViewExport.h"
#include <QObject>
#include <QGraphicsRectItem>

// 矩形 ROI 图元：带 8 个尺寸手柄，可拖动内部移动、拖角/边改大小。
// 假定置于场景原点、无旋转无位移，因此 rect() 坐标即图像像素坐标。
class FRAMEIMAGEVIEW_API RectRoiItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    enum Handle {
        H_None = -1,
        H_TopLeft = 0, H_TopRight, H_BottomRight, H_BottomLeft,
        H_Top, H_Right, H_Bottom, H_Left
    };

    explicit RectRoiItem(const QRectF& rect, QGraphicsItem* parent = nullptr);

    // 当前 ROI 矩形（图像像素坐标）。
    QRectF roiRect() const { return rect(); }

signals:
    void changed(); // 用户移动/改大小结束时发出

protected:
    void hoverMoveEvent(QGraphicsSceneHoverEvent* e) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* e) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* e) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* e) override;
    void paint(QPainter* p, const QStyleOptionGraphicsItem* opt, QWidget* w) override;
    QRectF boundingRect() const override;

private:
    Handle  handleAt(const QPointF& pos) const;
    QPointF handlePos(Handle h) const;
    void    applyCursor(Handle h);

    enum class Mode { None, Move, Resize };

    Mode    m_mode = Mode::None;
    Handle  m_resizeHandle = H_None;
    QPointF m_dragOffset;             // 移动时鼠标相对矩形左上角的偏移
    QRectF  m_pressRect;              // 改大小时按下的起始矩形
    qreal   m_grip = 6.0;             // 手柄视觉边长的一半
    qreal   m_gripTolerance = 8.0;    // 命中手柄的最大距离
};