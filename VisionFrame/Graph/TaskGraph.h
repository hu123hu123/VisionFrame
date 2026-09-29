#pragma once

#include <QString>
#include <QStringList>
#include <QVector>
#include <QPointF>
#include <QJsonObject>
#include <QJsonArray>
#include <functional>
#include <map>
#include <string>
#include "FrameToolBase.h"

// 节点运行态（供执行高亮）：Idle=默认，Running=执行中(绿)，Success=成功(复原)，Failed=失败(红)。
enum class NodeRunState : int { Idle = 0, Running, Success, Failed };

// 图中的一个节点：对应一个工具实例。
struct GraphNodeData
{
    QString  id;                       // UUID
    QString  toolTypeId;               // = FrameToolBase::ToolName()
    QString  title;                    // 显示标题
    QPointF  pos;                      // 场景坐标（节点左上角）
    FrameToolBase* tool{ nullptr };    // 拥有所有权
    std::map<std::string, NodeData> outputs;  // 最近一次执行输出（按输出引脚名）
};

// 图中的一条连线：由 output 引脚指向 input 引脚。
struct GraphEdgeData
{
    QString id;
    QString fromNode;
    QString toNode;
    QString fromPin;   // 上游节点输出引脚名
    QString toPin;     // 下游节点输入引脚名
};

// 任务流图数据模型：与 UI 渲染、线程执行解耦。
class TaskGraph
{
public:
    QVector<GraphNodeData> nodes;
    QVector<GraphEdgeData> edges;

    // Kahn 拓扑排序；成功返回 true 并填 order；失败（有环）返回 false，cycleNode 填一个入环节点 id。
    bool topoOrder(QStringList& order, QString* cycleNode = nullptr) const;

    // 连线合法性：输出->输入；类型匹配；输入引脚最多一条入边；不形成环。
    bool canConnect(const QString& fromNode, const QString& fromPin,
                    const QString& toNode, const QString& toPin,
                    QString* why = nullptr) const;

    const GraphNodeData* findNode(const QString& id) const;
    GraphNodeData* findNode(const QString& id);

    // 查找指向某输入引脚的唯一入边；不存在返回 nullptr。
    const GraphEdgeData* inputEdge(const QString& toNode, const QString& toPin) const;

    QJsonObject toJson() const;
    void fromJson(const QJsonObject& obj, const std::function<FrameToolBase*(const QString&)>& factory);

    QString newId() const;
};