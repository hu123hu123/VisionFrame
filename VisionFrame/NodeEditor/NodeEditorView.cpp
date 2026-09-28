#include "NodeEditorView.h"
#include "NodeEditorScene.h"
#include "NodeEditor/Style.h"
#include <QPainter>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <cmath>

NodeEditorView::NodeEditorView(QWidget* parent)
    : QGraphicsView(parent)
{
    setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    setAcceptDrops(true);
    setBackgroundBrush(NodeStyle::background());
}

QPointF NodeEditorView::sceneCenter() const
{
    return mapToScene(viewport()->rect().center());
}

void NodeEditorView::drawBackground(QPainter* p, const QRectF& rect)
{
    p->fillRect(rect, NodeStyle::background());

    const qreal s = 20.0;
    p->setPen(QPen(NodeStyle::grid(), 0));
    qreal left = std::floor(rect.left() / s) * s;
    qreal top = std::floor(rect.top() / s) * s;
    for (qreal x = left; x < rect.right(); x += s)
        p->drawLine(QPointF(x, rect.top()), QPointF(x, rect.bottom()));
    for (qreal y = top; y < rect.bottom(); y += s)
        p->drawLine(QPointF(rect.left(), y), QPointF(rect.right(), y));
}

void NodeEditorView::wheelEvent(QWheelEvent* e)
{
    const double factor = (e->angleDelta().y() > 0) ? 1.15 : (1.0 / 1.15);
    scale(factor, factor);
    e->accept();
}

void NodeEditorView::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace)
    {
        if (auto* s = qobject_cast<NodeEditorScene*>(scene()))
        {
            s->deleteSelection();
            e->accept();
            return;
        }
    }
    QGraphicsView::keyPressEvent(e);
}

void NodeEditorView::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::MiddleButton)
    {
        m_panning = true;
        m_lastPanPos = e->pos();
        viewport()->setCursor(Qt::ClosedHandCursor);
        e->accept();
        return;
    }
    QGraphicsView::mousePressEvent(e);
}

void NodeEditorView::mouseMoveEvent(QMouseEvent* e)
{
    if (m_panning)
    {
        const QPoint d = e->pos() - m_lastPanPos;
        m_lastPanPos = e->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - d.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - d.y());
        e->accept();
        return;
    }
    QGraphicsView::mouseMoveEvent(e);
}

void NodeEditorView::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() == Qt::MiddleButton && m_panning)
    {
        m_panning = false;
        viewport()->unsetCursor();
        e->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(e);
}

void NodeEditorView::dragEnterEvent(QDragEnterEvent* e)
{
    if (e->mimeData()->hasFormat(QStringLiteral("application/x-vf-tool")))
        e->acceptProposedAction();
    else
        QGraphicsView::dragEnterEvent(e);
}

void NodeEditorView::dragMoveEvent(QDragMoveEvent* e)
{
    if (e->mimeData()->hasFormat(QStringLiteral("application/x-vf-tool")))
        e->acceptProposedAction();
    else
        QGraphicsView::dragMoveEvent(e);
}

void NodeEditorView::dropEvent(QDropEvent* e)
{
    if (e->mimeData()->hasFormat(QStringLiteral("application/x-vf-tool")))
    {
        const QString typeId = QString::fromUtf8(e->mimeData()->data(QStringLiteral("application/x-vf-tool")));
        emit toolDropped(typeId, mapToScene(e->pos()));
        e->acceptProposedAction();
        return;
    }
    QGraphicsView::dropEvent(e);
}