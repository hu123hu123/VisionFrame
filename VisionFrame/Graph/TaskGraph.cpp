#include "TaskGraph.h"
#include <QJsonDocument>
#include <QHash>
#include <QQueue>
#include <QUuid>

QString TaskGraph::newId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

const GraphNodeData* TaskGraph::findNode(const QString& id) const
{
    for (const auto& n : nodes)
        if (n.id == id)
            return &n;
    return nullptr;
}

GraphNodeData* TaskGraph::findNode(const QString& id)
{
    for (auto& n : nodes)
        if (n.id == id)
            return &n;
    return nullptr;
}

const GraphEdgeData* TaskGraph::inputEdge(const QString& toNode, const QString& toPin) const
{
    for (const auto& e : edges)
        if (e.toNode == toNode && e.toPin == toPin)
            return &e;
    return nullptr;
}

static bool getPort(const GraphNodeData& n, const QString& name, bool isInput, FPort& out)
{
    if (!n.tool)
        return false;
    std::vector<FPort> ports;
    n.tool->GetPorts(ports);
    for (const auto& p : ports)
    {
        if (p.isInput == isInput && QString::fromStdString(p.name) == name)
        {
            out = p;
            return true;
        }
    }
    return false;
}

bool TaskGraph::canConnect(const QString& fromNode, const QString& fromPin,
                           const QString& toNode, const QString& toPin,
                           QString* why) const
{
    if (why) why->clear();
    const GraphNodeData* fn = findNode(fromNode);
    const GraphNodeData* tn = findNode(toNode);
    if (!fn || !tn)
    {
        if (why) *why = QString::fromUtf8("节点不存在");
        return false;
    }
    if (fromNode == toNode)
    {
        if (why) *why = QString::fromUtf8("不能连接自身");
        return false;
    }
    FPort fp, tp;
    if (!getPort(*fn, fromPin, false, fp))
    {
        if (why) *why = QString::fromUtf8("输出引脚不存在");
        return false;
    }
    if (!getPort(*tn, toPin, true, tp))
    {
        if (why) *why = QString::fromUtf8("输入引脚不存在");
        return false;
    }
    if (inputEdge(toNode, toPin))
    {
        if (why) *why = QString::fromUtf8("输入引脚已有连线");
        return false;
    }
    if (fp.type != tp.type && fp.type != PinType::Any && tp.type != PinType::Any)
    {
        if (why) *why = QString::fromUtf8("引脚类型不匹配");
        return false;
    }
    // 环检测：临时加入该边，若无法拓扑排序则有环。
    TaskGraph tmp = *this;
    GraphEdgeData e;
    e.id = "tmp";
    e.fromNode = fromNode;
    e.fromPin = fromPin;
    e.toNode = toNode;
    e.toPin = toPin;
    tmp.edges.append(e);
    QStringList order;
    if (!tmp.topoOrder(order, nullptr))
    {
        if (why) *why = QString::fromUtf8("会形成环");
        return false;
    }
    return true;
}

bool TaskGraph::topoOrder(QStringList& order, QString* cycleNode) const
{
    order.clear();
    QHash<QString, int> indeg;
    for (const auto& n : nodes)
        indeg[n.id] = 0;
    for (const auto& e : edges)
        if (indeg.contains(e.toNode))
            indeg[e.toNode]++;

    QQueue<QString> q;
    for (const auto& n : nodes)
        if (indeg.value(n.id) == 0)
            q.enqueue(n.id);

    int count = 0;
    while (!q.isEmpty())
    {
        QString id = q.dequeue();
        order.append(id);
        count++;
        for (const auto& e : edges)
        {
            if (e.fromNode == id && indeg.contains(e.toNode))
            {
                if (--indeg[e.toNode] == 0)
                    q.enqueue(e.toNode);
            }
        }
    }

    if (count < nodes.size())
    {
        if (cycleNode)
        {
            for (const auto& n : nodes)
                if (indeg.value(n.id) > 0)
                {
                    *cycleNode = n.id;
                    break;
                }
        }
        return false;
    }
    return true;
}

QJsonObject TaskGraph::toJson() const
{
    QJsonObject root;
    QJsonArray na;
    for (const auto& n : nodes)
    {
        QJsonObject o;
        o["id"] = n.id;
        o["type"] = n.toolTypeId;
        o["title"] = n.title;
        o["x"] = n.pos.x();
        o["y"] = n.pos.y();
        if (n.tool)
        {
            const std::string ps = n.tool->SaveParamsToJson();
            if (!ps.empty())
            {
                QJsonDocument d = QJsonDocument::fromJson(QByteArray::fromStdString(ps));
                if (d.isObject())
                    o["params"] = d.object();
            }
        }
        na.append(o);
    }
    QJsonArray ea;
    for (const auto& e : edges)
    {
        QJsonObject o;
        o["id"] = e.id;
        o["fromNode"] = e.fromNode;
        o["fromPin"] = e.fromPin;
        o["toNode"] = e.toNode;
        o["toPin"] = e.toPin;
        ea.append(o);
    }
    root["nodes"] = na;
    root["edges"] = ea;
    return root;
}

void TaskGraph::fromJson(const QJsonObject& obj, const std::function<FrameToolBase*(const QString&)>& factory)
{
    nodes.clear();
    edges.clear();
    const QJsonArray na = obj["nodes"].toArray();
    for (const auto& v : na)
    {
        const QJsonObject o = v.toObject();
        GraphNodeData n;
        n.id = o["id"].toString();
        if (n.id.isEmpty())
            n.id = newId();
        n.toolTypeId = o["type"].toString();
        n.title = o["title"].toString();
        n.pos = QPointF(o["x"].toDouble(), o["y"].toDouble());
        if (factory)
            n.tool = factory(n.toolTypeId);
        const QJsonObject po = o["params"].toObject();
        if (!po.isEmpty() && n.tool)
        {
            const std::string ps = QJsonDocument(po).toJson(QJsonDocument::Compact).toStdString();
            n.tool->LoadParamsFromJson(ps);
        }
        nodes.append(n);
    }
    const QJsonArray ea = obj["edges"].toArray();
    for (const auto& v : ea)
    {
        const QJsonObject o = v.toObject();
        GraphEdgeData e;
        e.id = o["id"].toString();
        if (e.id.isEmpty())
            e.id = newId();
        e.fromNode = o["fromNode"].toString();
        e.fromPin = o["fromPin"].toString();
        e.toNode = o["toNode"].toString();
        e.toPin = o["toPin"].toString();
        edges.append(e);
    }
}