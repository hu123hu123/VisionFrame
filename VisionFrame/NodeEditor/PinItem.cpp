#include "PinItem.h"
#include "NodeGraphicsItem.h"
#include "NodeEditor/Style.h"
#include <QPainter>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>

PinItem::PinItem(const FPort& port, bool isInput, NodeGraphicsItem* parent)
    : QGraphicsObject(parent), m_port(port), m_input(isInput), m_node(parent)
{
    setAcceptHoverEvents(true);
    setCursor(QCursor(Qt::CrossCursor));
}

QString PinItem::nodeId() const
{
    return m_node ? m_node->nodeId() : QString();
}

QPainterPath PinItem::shape() const
{
    QPainterPath p;
    p.addEllipse(QRectF(-8, -8, 16, 16));
    return p;
}

void PinItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(QColor(0x0a0a0a), 1));
    painter->setBrush(NodeStyle::pinColor(dataType()));
    painter->drawEllipse(boundingRect());
}

void PinItem::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
    if (e->button() == Qt::LeftButton && !m_input)
    {
        m_dragging = true;
        grabMouse();
        emit wireDragStarted(this);
        e->accept();
        return;
    }
    QGraphicsObject::mousePressEvent(e);
}

void PinItem::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{
    if (m_dragging)
    {
        emit wireDragMoved(this, e->scenePos());
        e->accept();
        return;
    }
    QGraphicsObject::mouseMoveEvent(e);
}

void PinItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
    if (m_dragging)
    {
        m_dragging = false;
        ungrabMouse();
        emit wireDragEnded(this, e->scenePos());
        e->accept();
        return;
    }
    QGraphicsObject::mouseReleaseEvent(e);
}