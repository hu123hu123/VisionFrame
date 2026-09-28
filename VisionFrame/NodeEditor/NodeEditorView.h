#pragma once

#include <QGraphicsView>
#include <QPoint>

// 蓝图编辑器视图：网格背景、滚轮缩放、中键拖拽平移、Delete 删除、接收工具拖入。
class NodeEditorView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit NodeEditorView(QWidget* parent = nullptr);

    QPointF sceneCenter() const;

signals:
    void toolDropped(const QString& typeId, const QPointF& scenePos);

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void wheelEvent(QWheelEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void dragEnterEvent(QDragEnterEvent* e) override;
    void dragMoveEvent(QDragMoveEvent* e) override;
    void dropEvent(QDropEvent* e) override;

private:
    bool m_panning{ false };
    QPoint m_lastPanPos;
};