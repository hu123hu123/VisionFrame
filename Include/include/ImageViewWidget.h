#pragma once

#include "FrameImageViewExport.h"
#include <QGraphicsView>
#include <QImage>
#include <opencv2/core.hpp>

class QGraphicsScene;
class QGraphicsPixmapItem;

// 可复用图像显示视图：暗色背景、滚轮缩放、中键平移、自适应(fit)、像素坐标映射。
// 仅接受 QImage，不依赖任何业务模型。
class FRAMEIMAGEVIEW_API ImageViewWidget : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ImageViewWidget(QWidget* parent = nullptr);

    void setImage(const QImage& img);      // 替换当前图像
    void clearImage();
    bool hasImage() const;

    void fitToWindow();                    // 自适应窗口
    void setZoom(qreal scale);             // 相对原始图缩放

    QRectF imageRect() const;                               // 图像在场景中的矩形
    QPointF mapToImage(const QPointF& scenePos) const;      // 场景 -> 图像像素坐标
    QPointF mapFromImage(const QPointF& imagePos) const;    // 图像像素 -> 场景

protected:
    void wheelEvent(QWheelEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;

private:
    QGraphicsScene*      m_scene = nullptr;
    QGraphicsPixmapItem* m_pixmapItem = nullptr;
    QImage               m_image;
    bool                 m_panning = false;
    QPoint               m_lastPan;
    bool                 m_autoFit = true;
};

// cv::Mat -> QImage（8U3C/8U1C/其它转 8U3C），供显示层复用。
FRAMEIMAGEVIEW_API QImage matToQImage(const cv::Mat& m);