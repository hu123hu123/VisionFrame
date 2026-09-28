#pragma once

#include <QGraphicsObject>
#include "FrameToolBase.h"

class NodeGraphicsItem;

// 节点引脚（端口）。输出引脚可拖出连线，输入引脚接受连入。
class PinItem : public QGraphicsObject
{
    Q_OBJECT
public:
    PinItem(const FPort& port, bool isInput, NodeGraphicsItem* parent);

    QRectF boundingRect() const override { return QRectF(-6, -6, 12, 12); }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    bool isInput() const { return m_input; }
    PinType dataType() const { return m_port.type; }
    QString name() const { return QString::fromStdString(m_port.name); }
    QString nodeId() const;
    NodeGraphicsItem* node() const { return m_node; }
    QPointF centerScenePos() const { return mapToScene(0, 0); }

    // 命中检测用小圆，交互热区略大。
    QPainterPath shape() const override;

signals:
    void wireDragStarted(PinItem* pin);
    void wireDragMoved(PinItem* pin, const QPointF& scenePos);
    void wireDragEnded(PinItem* pin, const QPointF& scenePos);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* e) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* e) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* e) override;

private:
    FPort m_port;
    bool m_input;
    bool m_dragging{ false };
    NodeGraphicsItem* m_node;
};