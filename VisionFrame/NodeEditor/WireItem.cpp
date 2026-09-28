#include "WireItem.h"
#include "PinItem.h"
#include "NodeGraphicsItem.h"
#include "NodeEditor/Style.h"
#include <QPainter>

WireItem::WireItem(PinItem* from, PinItem* to, QGraphicsItem* parent)
    : QGraphicsPathItem(parent)
{
    m_fromPin = from;
    m_toPin = to;
    m_color = edgeColor();
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    if (from && from->node())
        connect(from->node(), &NodeGraphicsItem::positionChanged, this, &WireItem::updatePath);
    if (to && to->node())
        connect(to->node(), &NodeGraphicsItem::positionChanged, this, &WireItem::updatePath);

    updatePath();
}

WireItem::WireItem(PinItem* from, const QPointF& sceneEnd, QGraphicsItem* parent)
    : QGraphicsPathItem(parent)
{
    m_fromPin = from;
    m_toPoint = sceneEnd;
    m_hasToPoint = true;
    m_color = edgeColor();
    if (from && from->node())
        connect(from->node(), &NodeGraphicsItem::positionChanged, this, &WireItem::updatePath);

    updatePath();
}

WireItem::~WireItem() = default;

QColor WireItem::edgeColor() const
{
    if (m_fromPin)
        return NodeStyle::pinColor(m_fromPin->dataType());
    if (m_toPin)
        return NodeStyle::pinColor(m_toPin->dataType());
    return QColor(0x888888);
}

void WireItem::setEndPoint(const QPointF& scenePt)
{
    m_toPoint = scenePt;
    m_hasToPoint = true;
    m_toPin = nullptr;
    updatePath();
}

void WireItem::updatePath()
{
    QPointF s = m_fromPin ? m_fromPin->centerScenePos() : m_fromPoint;
    QPointF e = m_toPin ? m_toPin->centerScenePos() : (m_hasToPoint ? m_toPoint : s);

    QPainterPath path;
    path.moveTo(s);
    qreal dx = qMax<qreal>(40.0, qAbs(e.x() - s.x()) * 0.5);
    path.cubicTo(QPointF(s.x() + dx, s.y()), QPointF(e.x() - dx, e.y()), e);
    setPath(path);
}

void WireItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing);
    QPen pen(m_color, isSelected() ? 3.0 : 1.8);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path());
}