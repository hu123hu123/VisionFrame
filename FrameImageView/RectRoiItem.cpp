#include "RectRoiItem.h"

#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QLineF>

static const QColor kRoiBorder(0x2dff8c);
static const QColor kRoiFill(0x2d, 0xff, 0x8c, 28);
static const QColor kHandleFill(0xffffff);

RectRoiItem::RectRoiItem(const QRectF& rect, QGraphicsItem* parent)
    : QGraphicsRectItem(rect, parent)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptHoverEvents(true);
}

QRectF RectRoiItem::boundingRect() const
{
    const qreal m = m_grip + 1.0;
    return rect().adjusted(-m, -m, m, m);
}

QPointF RectRoiItem::handlePos(Handle h) const
{
    const QRectF r = rect();
    switch (h)
    {
    case H_TopLeft:     return r.topLeft();
    case H_TopRight:    return r.topRight();
    case H_BottomRight: return r.bottomRight();
    case H_BottomLeft:  return r.bottomLeft();
    case H_Top:         return QPointF(r.center().x(), r.top());
    case H_Right:       return QPointF(r.right(), r.center().y());
    case H_Bottom:      return QPointF(r.center().x(), r.bottom());
    case H_Left:        return QPointF(r.left(), r.center().y());
    default:            return r.topLeft();
    }
}

RectRoiItem::Handle RectRoiItem::handleAt(const QPointF& pos) const
{
    Handle best = H_None;
    qreal bestDist = m_gripTolerance;
    for (int h = H_TopLeft; h <= H_Left; ++h)
    {
        const qreal d = QLineF(pos, handlePos(static_cast<Handle>(h))).length();
        if (d <= bestDist)
        {
            bestDist = d;
            best = static_cast<Handle>(h);
        }
    }
    return best;
}

void RectRoiItem::applyCursor(Handle h)
{
    Qt::CursorShape cs = Qt::SizeAllCursor;
    switch (h)
    {
    case H_TopLeft:     case H_BottomRight: cs = Qt::SizeFDiagCursor; break;
    case H_TopRight:    case H_BottomLeft:  cs = Qt::SizeBDiagCursor; break;
    case H_Top:         case H_Bottom:      cs = Qt::SizeVerCursor;   break;
    case H_Left:        case H_Right:       cs = Qt::SizeHorCursor;   break;
    default: break;
    }
    setCursor(cs);
}

void RectRoiItem::hoverMoveEvent(QGraphicsSceneHoverEvent* e)
{
    applyCursor(handleAt(e->pos()));
    QGraphicsRectItem::hoverMoveEvent(e);
}

void RectRoiItem::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
    if (e->button() != Qt::LeftButton)
    {
        e->ignore();
        return;
    }
    const Handle h = handleAt(e->pos());
    if (h != H_None)
    {
        m_mode = Mode::Resize;
        m_resizeHandle = h;
        m_pressRect = rect();
        e->accept();
        return;
    }
    if (rect().contains(e->pos()))
    {
        m_mode = Mode::Move;
        m_dragOffset = e->pos() - rect().topLeft();
        e->accept();
        return;
    }
    QGraphicsRectItem::mousePressEvent(e);
}

void RectRoiItem::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{
    if (m_mode == Mode::Move)
    {
        setRect(QRectF(e->pos() - m_dragOffset, rect().size()));
        e->accept();
        return;
    }
    if (m_mode == Mode::Resize)
    {
        QRectF r = m_pressRect;
        const QPointF p = e->pos();
        switch (m_resizeHandle)
        {
        case H_TopLeft:     r.setTopLeft(p);     break;
        case H_TopRight:    r.setTopRight(p);    break;
        case H_BottomRight: r.setBottomRight(p); break;
        case H_BottomLeft:  r.setBottomLeft(p);  break;
        case H_Top:         r.setTop(p.y());     break;
        case H_Right:       r.setRight(p.x());   break;
        case H_Bottom:      r.setBottom(p.y());  break;
        case H_Left:        r.setLeft(p.x());    break;
        default: break;
        }
        setRect(r.normalized());
        e->accept();
        return;
    }
    QGraphicsRectItem::mouseMoveEvent(e);
}

void RectRoiItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
    if (m_mode != Mode::None)
    {
        m_mode = Mode::None;
        m_resizeHandle = H_None;
        emit changed();
        e->accept();
        return;
    }
    QGraphicsRectItem::mouseReleaseEvent(e);
}

void RectRoiItem::paint(QPainter* p, const QStyleOptionGraphicsItem* opt, QWidget* w)
{
    (void)opt; (void)w;

    p->setBrush(kRoiFill);
    p->setPen(QPen(kRoiBorder, 0));
    p->drawRect(rect());

    p->setPen(Qt::NoPen);
    p->setBrush(kHandleFill);
    for (int h = H_TopLeft; h <= H_Left; ++h)
    {
        QRectF hh(QPointF(), QSizeF(m_grip, m_grip));
        hh.moveCenter(handlePos(static_cast<Handle>(h)));
        p->drawRect(hh);
    }
}