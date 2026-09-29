#pragma once

#include <QGraphicsScene>
#include <QHash>
#include "Graph/TaskGraph.h"

class NodeGraphicsItem;
class PinItem;
class WireItem;

// 蓝图编辑器场景：维护 TaskGraph 与图形项的同步，处理节点增删、连线。
class NodeEditorScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit NodeEditorScene(QObject* parent = nullptr);

    void setGraph(TaskGraph* g);
    TaskGraph* graph() const { return m_graph; }

    void rebuild();
    void addNode(const QString& typeId, const QPointF& scenePos);

    void applyRunStates(const QHash<QString, NodeRunState>& states);

    bool editEnabled() const { return m_editEnabled; }
    void setEditEnabled(bool b) { m_editEnabled = b; }

signals:
    void graphChanged();
    void nodeEditRequested(const QString& nodeId);

public slots:
    void deleteSelection();

private slots:
    void onWireDragStarted(PinItem* pin);
    void onWireDragMoved(PinItem* pin, const QPointF& scenePos);
    void onWireDragEnded(PinItem* pin, const QPointF& scenePos);

private:
    void addWireItem(const GraphEdgeData& e);
    void tryConnect(PinItem* from, PinItem* to);

    TaskGraph* m_graph{ nullptr };
    QHash<QString, NodeGraphicsItem*> m_nodeItems;
    PinItem* m_wireSource{ nullptr };
    WireItem* m_tempWire{ nullptr };
    bool m_editEnabled{ true };
};