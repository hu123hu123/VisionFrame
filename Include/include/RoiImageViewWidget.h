#pragma once

#include "ImageViewWidget.h"
#include <QRectF>

class RectRoiItem;

// 可绘制矩形 ROI 的图像显示视图：
//   空白处按下拖拽 -> 新建矩形；拖内部移动；拖 8 手柄改大小（一次只保留一个）。
// roiRect()/roiChanged 均使用图像像素坐标。
class FRAMEIMAGEVIEW_API RoiImageViewWidget : public ImageViewWidget
{
    Q_OBJECT
public:
    explicit RoiImageViewWidget(QWidget* parent = nullptr);

    void  setRoi(const QRectF& imageRect);  // 用像素坐标设置/替换 ROI；empty 则清除
    QRectF roiRect() const;                 // 当前 ROI（像素坐标）；无则 empty
    void  clearRoi();

signals:
    void roiChanged(const QRectF& imageRect); // 新建/移动/改大小结束时发出

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

private:
    QPointF clampToImage(const QPointF& scenePos) const;

    RectRoiItem* m_roi = nullptr;
    bool         m_drawing = false;
    QPointF      m_drawStart;
};