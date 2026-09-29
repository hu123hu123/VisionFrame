#include "NodeEditorScene.h"
#include "NodeGraphicsItem.h"
#include "PinItem.h"
#include "WireItem.h"
#include "NodeEditor/ToolFactory.h"
#include <QDebug>

NodeEditorScene::NodeEditorScene(QObject* parent)
    : QGraphicsScene(parent)
{
    setItemIndexMethod(QGraphicsScene::NoIndex);
    setSceneRect(-10000, -10000, 20000, 20000);
}

void NodeEditorScene::setGraph(TaskGraph* g)
{
    m_graph = g;
    rebuild();
}

void NodeEditorScene::rebuild()
{
    clear();
    m_nodeItems.clear();
    m_wireSource = nullptr;
    m_tempWire = nullptr;

    if (!m_graph)
        return;

    for (auto& n : m_graph->nodes)
    {
        auto* item = new NodeGraphicsItem(&n);
        connect(item, &NodeGraphicsItem::editRequested, this, &NodeEditorScene::nodeEditRequested);
        addItem(item);
        m_nodeItems.insert(n.id, item);

        const QList<PinItem*> pins = item->pinItems();
        for (PinItem* pin : pins)
        {
            connect(pin, &PinItem::wireDragStarted, this, &NodeEditorScene::onWireDragStarted);
            connect(pin, &PinItem::wireDragMoved, this, &NodeEditorScene::onWireDragMoved);
            connect(pin, &PinItem::wireDragEnded, this, &NodeEditorScene::onWireDragEnded);
        }
    }

    for (const auto& e : m_graph->edges)
        addWireItem(e);
}

void NodeEditorScene::addNode(const QString& typeId, const QPointF& scenePos)
{
    if (!m_graph)
        return;
    FrameToolBase* tool = ToolFactory::instance().create(typeId);
    if (!tool)
        return;

    GraphNodeData n;
    n.id = m_graph->newId();
    n.toolTypeId = typeId;
    n.title = ToolFactory::instance().displayName(typeId);
    n.pos = scenePos;
    n.tool = tool;
    m_graph->nodes.append(n);

    rebuild();
    emit graphChanged();
}

void NodeEditorScene::applyRunStates(const QHash<QString, NodeRunState>& states)
{
    for (auto it = m_nodeItems.begin(); it != m_nodeItems.end(); ++it)
        it.value()->setRunState(states.value(it.key(), NodeRunState::Idle));
}

void NodeEditorScene::deleteSelection()
{
    if (!m_editEnabled || !m_graph)
        return;

    bool changed = false;
    const QList<QGraphicsItem*> sel = selectedItems();
    for (QGraphicsItem* it : sel)
    {
        if (auto* ni = dynamic_cast<NodeGraphicsItem*>(it))
        {
            const QString id = ni->nodeId();
            if (ni->nodeData())
            {
                delete ni->nodeData()->tool;
                ni->nodeData()->tool = nullptr;
            }
            for (int i = m_graph->edges.size() - 1; i >= 0; --i)
                if (m_graph->edges[i].fromNode == id || m_graph->edges[i].toNode == id)
                    m_graph->edges.removeAt(i);
            for (int i = m_graph->nodes.size() - 1; i >= 0; --i)
                if (m_graph->nodes[i].id == id)
                    m_graph->nodes.removeAt(i);
            changed = true;
        }
        else if (auto* wi = dynamic_cast<WireItem*>(it))
        {
            const QString eid = wi->edgeId();
            if (!eid.isEmpty())
            {
                for (int i = m_graph->edges.size() - 1; i >= 0; --i)
                    if (m_graph->edges[i].id == eid)
                    {
                        m_graph->edges.removeAt(i);
                        changed = true;
                        break;
                    }
            }
        }
    }

    if (changed)
    {
        rebuild();
        emit graphChanged();
    }
}

void NodeEditorScene::onWireDragStarted(PinItem* pin)
{
    if (!m_editEnabled || !m_graph)
        return;
    m_wireSource = pin;
    m_tempWire = new WireItem(pin, pin->centerScenePos());
    addItem(m_tempWire);
}

void NodeEditorScene::onWireDragMoved(PinItem* pin, const QPointF& scenePos)
{
    Q_UNUSED(pin);
    if (m_tempWire)
        m_tempWire->setEndPoint(scenePos);
}

void NodeEditorScene::onWireDragEnded(PinItem* pin, const QPointF& scenePos)
{
    Q_UNUSED(pin);
    if (m_tempWire)
    {
        removeItem(m_tempWire);
        delete m_tempWire;
        m_tempWire = nullptr;
    }

    PinItem* target = dynamic_cast<PinItem*>(itemAt(scenePos, QTransform()));
    if (m_wireSource && target && target != m_wireSource && target->isInput())
        tryConnect(m_wireSource, target);

    m_wireSource = nullptr;
}

void NodeEditorScene::tryConnect(PinItem* from, PinItem* to)
{
    if (!m_graph || from->isInput() || !to->isInput())
        return;

    QString why;
    if (!m_graph->canConnect(from->nodeId(), from->name(), to->nodeId(), to->name(), &why))
    {
        qDebug().noquote() << "连线被拒绝:" << why;
        return;
    }

    GraphEdgeData e;
    e.id = m_graph->newId();
    e.fromNode = from->nodeId();
    e.fromPin = from->name();
    e.toNode = to->nodeId();
    e.toPin = to->name();
    m_graph->edges.append(e);

    addWireItem(e);
    emit graphChanged();
}

void NodeEditorScene::addWireItem(const GraphEdgeData& e)
{
    NodeGraphicsItem* fn = m_nodeItems.value(e.fromNode);
    NodeGraphicsItem* tn = m_nodeItems.value(e.toNode);
    if (!fn || !tn)
        return;
    PinItem* fp = fn->outputPin(e.fromPin);
    PinItem* tp = tn->inputPin(e.toPin);
    if (!fp || !tp)
        return;

    auto* w = new WireItem(fp, tp);
    w->setEdgeId(e.id);
    addItem(w);
}