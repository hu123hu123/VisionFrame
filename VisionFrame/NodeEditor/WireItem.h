#pragma once

#include <QObject>
#include <QGraphicsPathItem>

class PinItem;

// 节点间连线（贝塞尔曲线）。用于展示真实连线或拖拽中的临时连线。
// 通过多重继承 QObject 以便接收节点移动信号并更新路径。
class WireItem : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:
    // 真实连线：from 输出引脚 -> to 输入引脚。
    WireItem(PinItem* from, PinItem* to, QGraphicsItem* parent = nullptr);
    // 临时连线：from 输出引脚 -> 自由端点（拖拽中）。
    WireItem(PinItem* from, const QPointF& sceneEnd, QGraphicsItem* parent = nullptr);
    ~WireItem() override;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    void setEndPoint(const QPointF& scenePt);
    void setEdgeId(const QString& id) { m_edgeId = id; }
    QString edgeId() const { return m_edgeId; }

public slots:
    void updatePath();

private:
    QColor edgeColor() const;

    PinItem* m_fromPin{ nullptr };
    PinItem* m_toPin{ nullptr };
    QPointF m_fromPoint;
    QPointF m_toPoint;
    bool m_hasFromPoint{ false };
    bool m_hasToPoint{ false };
    QColor m_color{ QColor(0x888888) };
    QString m_edgeId;
};