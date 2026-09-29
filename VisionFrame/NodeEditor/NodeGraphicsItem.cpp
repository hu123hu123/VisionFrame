#include "NodeGraphicsItem.h"
#include "PinItem.h"
#include "NodeEditor/Style.h"
#include "NodeEditor/ToolFactory.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

NodeGraphicsItem::NodeGraphicsItem(GraphNodeData* node, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_node(node), m_id(node ? node->id : QString())
{
    setFlag(ItemIsMovable, true);
    setFlag(ItemIsSelectable, true);
    setFlag(ItemSendsGeometryChanges, true);

    if (node && node->tool)
    {
        std::vector<FPort> ports;
        node->tool->GetPorts(ports);
        for (const auto& p : ports)
        {
            if (p.isInput)
                m_inputs.push_back(p);
            else
                m_outputs.push_back(p);
        }
        const ToolEntry* e = ToolFactory::instance().find(node->toolTypeId);
        m_category = e ? e->category : QString();
    }

    setPos(node ? node->pos : QPointF());
    layoutPins();
}

void NodeGraphicsItem::setRunState(NodeRunState s)
{
    if (m_runState == s)
        return;
    m_runState = s;
    update();
}

QRectF NodeGraphicsItem::bodyRect() const
{
    int rows = qMax<int>(m_inputs.size(), m_outputs.size());
    if (rows < 1)
        rows = 1;
    return QRectF(0, 0, m_width, m_titleH + rows * m_pinH + 8.0);
}

QRectF NodeGraphicsItem::boundingRect() const
{
    // 外扩 margin 覆盖选中/运行态描边，避免拖动时旧描边残留在 boundingRect 外形成拖影。
    return bodyRect().adjusted(-5.0, -5.0, 5.0, 5.0);
}

void NodeGraphicsItem::layoutPins()
{
    for (size_t i = 0; i < m_inputs.size(); ++i)
    {
        auto* pin = new PinItem(m_inputs[i], true, this);
        double y = m_titleH + i * m_pinH + m_pinH / 2.0;
        pin->setPos(0, y);
        m_inputPins.insert(pin->name(), pin);
    }
    for (size_t i = 0; i < m_outputs.size(); ++i)
    {
        auto* pin = new PinItem(m_outputs[i], false, this);
        double y = m_titleH + i * m_pinH + m_pinH / 2.0;
        pin->setPos(m_width, y);
        m_outputPins.insert(pin->name(), pin);
    }
}

PinItem* NodeGraphicsItem::findPin(const QString& name)
{
    PinItem* p = m_inputPins.value(name, nullptr);
    if (p)
        return p;
    return m_outputPins.value(name, nullptr);
}

PinItem* NodeGraphicsItem::inputPin(const QString& name)
{
    return m_inputPins.value(name, nullptr);
}

PinItem* NodeGraphicsItem::outputPin(const QString& name)
{
    return m_outputPins.value(name, nullptr);
}

QList<PinItem*> NodeGraphicsItem::pinItems() const
{
    QList<PinItem*> list = m_inputPins.values();
    list.append(m_outputPins.values());
    return list;
}

void NodeGraphicsItem::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*)
{
    p->setRenderHint(QPainter::Antialiasing);
    QRectF r = bodyRect();

    // 主体
    p->setPen(QPen(NodeStyle::nodeBorder(), 1));
    p->setBrush(NodeStyle::nodeBody());
    p->drawRoundedRect(r, 5, 5);

    // 标题栏
    QRectF titleRect(r.left(), r.top(), r.width(), m_titleH);
    p->setPen(Qt::NoPen);
    p->setBrush(NodeStyle::categoryColor(m_category));
    p->drawRoundedRect(titleRect, 5, 5);
    p->drawRect(QRectF(titleRect.left(), titleRect.top() + 4.0, titleRect.width(), titleRect.height() - 4.0));

    p->setPen(NodeStyle::titleBarText());
    QFont f = p->font();
    f.setBold(true);
    p->setFont(f);
    p->drawText(titleRect, Qt::AlignCenter, m_node ? m_node->title : QString());
    p->setFont(QFont());

    // 引脚名称
    const QList<PinItem*> pins = pinItems();
    for (PinItem* pin : pins)
    {
        double y = pin->y();
        p->setPen(NodeStyle::text());
        if (pin->isInput())
            p->drawText(QRectF(14.0, y - 8.0, m_width / 2.0 - 18.0, 16.0),
                Qt::AlignLeft | Qt::AlignVCenter, pin->name());
        else
            p->drawText(QRectF(m_width / 2.0, y - 8.0, m_width / 2.0 - 14.0, 16.0),
                Qt::AlignRight | Qt::AlignVCenter, pin->name());
    }

    // 运行态高亮描边：执行中=绿、失败=红，否则仅保留选中描边
    if (m_runState == NodeRunState::Running)
    {
        p->setPen(QPen(NodeStyle::activeBorder(), 2.5));
        p->setBrush(Qt::NoBrush);
        p->drawRoundedRect(r.adjusted(-3, -3, 3, 3), 7, 7);
    }
    else if (m_runState == NodeRunState::Failed)
    {
        p->setPen(QPen(NodeStyle::failBorder(), 2.5));
        p->setBrush(Qt::NoBrush);
        p->drawRoundedRect(r.adjusted(-3, -3, 3, 3), 7, 7);
    }
    else if (isSelected())
    {
        p->setPen(QPen(NodeStyle::selectBorder(), 2));
        p->setBrush(Qt::NoBrush);
        p->drawRoundedRect(r.adjusted(-2, -2, 2, 2), 6, 6);
    }
}

void NodeGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e)
{
    emit editRequested(m_id);
    QGraphicsObject::mouseDoubleClickEvent(e);
}

QVariant NodeGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionHasChanged)
    {
        if (m_node)
            m_node->pos = pos();
        emit positionChanged();
    }
    return QGraphicsObject::itemChange(change, value);
}