#pragma once

#include <QGraphicsObject>
#include <QHash>
#include <QList>
#include <vector>
#include "FrameToolBase.h"
#include "Graph/TaskGraph.h"

class PinItem;

// 蓝图式节点：圆角矩形 + 分类着色标题栏 + 左右引脚。
class NodeGraphicsItem : public QGraphicsObject
{
    Q_OBJECT
public:
    NodeGraphicsItem(GraphNodeData* node, QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    QString nodeId() const { return m_id; }
    GraphNodeData* nodeData() { return m_node; }

    PinItem* inputPin(const QString& name);
    PinItem* outputPin(const QString& name);
    PinItem* findPin(const QString& name);
    QList<PinItem*> pinItems() const;

    void setActive(bool b);

signals:
    void editRequested(const QString& nodeId);
    void positionChanged();

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    void layoutPins();
    QRectF bodyRect() const;

    GraphNodeData* m_node;
    QString m_id;
    QString m_category;
    std::vector<FPort> m_inputs;
    std::vector<FPort> m_outputs;
    QHash<QString, PinItem*> m_inputPins;
    QHash<QString, PinItem*> m_outputPins;
    bool m_active{ false };

    double m_width{ 180.0 };
    double m_titleH{ 22.0 };
    double m_pinH{ 20.0 };
};