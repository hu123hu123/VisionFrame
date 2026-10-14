#include "TaskItem.h"
#include "FrameDataQueueRegistry.h"
#include "FrameLog.h"

TaskItem::TaskItem(QObject *parent)
	: QThread(parent)
{
	m_dataQueue = std::make_shared<FrameDataQueue>();
}

TaskItem::~TaskItem()
{
	StopTask();
}

void TaskItem::StartTask()
{
	m_stop = false;
	VFL_INFO("task") << "任务[" << m_taskName.toStdString() << "] 启动";
	if (m_dataQueue)
		m_dataQueue->reset(); // 清除可能的 stop 标志，允许复用
	if (!this->isRunning())
		this->start();
}

void TaskItem::StopTask()
{
	m_stop = true;
	VFL_INFO("task") << "任务[" << m_taskName.toStdString() << "] 停止";
	// 唤醒所有阻塞在 DeQueueTool::execute() 中的等待者，让线程优雅退出。
	if (m_dataQueue)
		m_dataQueue->stop();
	this->requestInterruption();
	this->wait();
	{
		QMutexLocker lock(&m_outputMutex);
		m_currentNodeId.clear();
		m_nodeStates.clear();
	}
}

void TaskItem::run()
{
	while (!m_stop)
	{
		doTask();
		if (!m_stop)
			QThread::msleep(m_frameIntervalMs.load());
	}
	// 停止后各节点最后一帧输出不再被引用：统一释放，避免图像常驻内存
	releaseGraphOutputs();
}

void TaskItem::releaseGraphOutputs()
{
	QMutexLocker lock(&m_outputMutex);
	for (auto& n : m_graph.nodes)
		n.outputs.clear();
}

void TaskItem::doTask()
{
	if (!m_graph.nodes.isEmpty())
	{
		executeGraphOnce();
	}
	else if (!m_tools.isEmpty())
	{
		const std::string tn = m_taskName.toStdString();
		// 兼容旧线性执行（无图任务）
		for (FrameToolBase* tool : m_tools)
		{
			if (m_stop)
				return;
			const std::string name = tool->ToolName();
			VFL_DEBUG("task") << "任务[" << tn << "] 工具[" << name << "] 进入";
			{
				QMutexLocker lock(&m_outputMutex);
				m_toolStates[tool] = NodeRunState::Running;
			}
			const ToolResult r = tool->execute();
			{
				QMutexLocker lock(&m_outputMutex);
				m_toolStates[tool] = (r == ToolResult::ToolOk) ? NodeRunState::Success : NodeRunState::Failed;
			}
			if (r == ToolResult::ToolOk)
				VFL_DEBUG("task") << "任务[" << tn << "] 工具[" << name << "] 结束";
			else if (r == ToolResult::ToolError)
				VFL_ERROR("task") << "任务[" << tn << "] 工具[" << name << "] 失败";
			else
				VFL_WARN("task") << "任务[" << tn << "] 工具[" << name << "] 中断";
			if (m_stop)
				return;
		}
	}
}

void TaskItem::executeGraphOnce()
{
	QStringList order;
	{
		QMutexLocker lock(&m_outputMutex);
		QString cycle;
		if (!m_graph.topoOrder(order, &cycle))
		{
			// 图有环，无法执行
			m_currentNodeId = cycle;
			return;
		}
	}

	const std::string tn = m_taskName.toStdString();
	for (const QString& id : order)
	{
		if (m_stop)
			return;
		GraphNodeData* node = m_graph.findNode(id);
		if (!node || !node->tool)
			continue;

		std::map<std::string, NodeData> inputs;
		for (const auto& e : m_graph.edges)
		{
			if (e.toNode == id)
			{
				const GraphNodeData* up = m_graph.findNode(e.fromNode);
				if (up)
				{
					auto it = up->outputs.find(e.fromPin.toStdString());
					if (it != up->outputs.end())
						inputs[e.toPin.toStdString()] = it->second;
				}
			}
		}

		{
			QMutexLocker lock(&m_outputMutex);
			m_currentNodeId = id;
			m_nodeStates[id] = NodeRunState::Running;
		}

		const std::string name = node->tool->ToolName();
		VFL_DEBUG("task") << "任务[" << tn << "] 节点[" << id.toStdString() << "] 工具[" << name << "] 进入";

		std::map<std::string, NodeData> outputs;
		const ToolResult r = node->tool->execute(inputs, outputs);

		{
			QMutexLocker lock(&m_outputMutex);
			node->outputs = std::move(outputs);
			m_nodeStates[id] = (r == ToolResult::ToolOk)
				? NodeRunState::Success : NodeRunState::Failed;
		}

		if (r == ToolResult::ToolOk)
			VFL_DEBUG("task") << "任务[" << tn << "] 节点[" << id.toStdString() << "] 工具[" << name << "] 结束";
		else if (r == ToolResult::ToolError)
			VFL_ERROR("task") << "任务[" << tn << "] 节点[" << id.toStdString() << "] 工具[" << name << "] 失败";
		else
			VFL_WARN("task") << "任务[" << tn << "] 节点[" << id.toStdString() << "] 工具[" << name << "] 中断";
	}
	m_frameCounter.fetch_add(1);
}

void TaskItem::runOnce()
{
	m_stop = false;
	doTask();
}

bool TaskItem::snapshotNodeOutputs(const QString& nodeId, std::map<std::string, NodeData>& out) const
{
	QMutexLocker lock(&m_outputMutex);
	for (const auto& n : m_graph.nodes)
	{
		if (n.id == nodeId)
		{
			out = n.outputs;
			return true;
		}
	}
	return false;
}

int TaskItem::frameCounter() const
{
	return m_frameCounter.load();
}

QString TaskItem::currentNodeId() const
{
	QMutexLocker lock(&m_outputMutex);
	return m_currentNodeId;
}

QHash<QString, NodeRunState> TaskItem::nodeRunStates() const
{
	QMutexLocker lock(&m_outputMutex);
	return m_nodeStates;
}

QVector<TaskItem::ToolRunState> TaskItem::toolRunStates() const
{
	QVector<ToolRunState> result;
	QMutexLocker lock(&m_outputMutex);
	if (!m_graph.nodes.isEmpty())
	{
		for (const auto& n : m_graph.nodes)
		{
			ToolRunState ts;
			if (!n.title.isEmpty())
				ts.name = n.title;
			else if (n.tool)
				ts.name = QString::fromStdString(n.tool->ToolName());
			else
				ts.name = n.id;
			ts.state = m_nodeStates.value(n.id, NodeRunState::Idle);
			result.append(ts);
		}
	}
	else
	{
		for (FrameToolBase* tool : m_tools)
		{
			ToolRunState ts;
			ts.name = QString::fromStdString(tool->ToolName());
			ts.state = m_toolStates.value(tool, NodeRunState::Idle);
			result.append(ts);
		}
	}
	return result;
}

void TaskItem::setFrameIntervalMs(int ms)
{
	if (ms < 1)
		ms = 1;
	m_frameIntervalMs.store(ms);
}

int TaskItem::frameIntervalMs() const
{
	return m_frameIntervalMs.load();
}

TaskGraph& TaskItem::graph()
{
	return m_graph;
}

void TaskItem::setGraph(const TaskGraph& g)
{
	m_graph = g;
}

void TaskItem::registerQueue()
{
	if (!m_dataQueue || m_taskName.isEmpty())
		return;
	FrameDataQueueRegistry::instance().registerQueue(
		m_taskName.toStdString(), m_dataQueue);
}

// —— 任务名 ——
QString TaskItem::taskName() const
{
	return m_taskName;
}

void TaskItem::setTaskName(const QString& name)
{
	if (m_taskName == name)
		return;
	m_taskName = name;
	registerQueue();
	// 回填到所有工具（FrameDeQueueTool 据此默认从本任务队列读取）
	for (FrameToolBase* tool : m_tools)
		tool->setOwnerQueueName(m_taskName.toStdString());
}

// —— 工具列表 ——
void TaskItem::addTool(FrameToolBase* tool)
{
	if (!tool)
		return;
	m_tools.append(tool);
	if (!m_taskName.isEmpty())
		tool->setOwnerQueueName(m_taskName.toStdString());
}

void TaskItem::addToolAt(int index, FrameToolBase* tool)
{
	if (!tool)
		return;
	m_tools.insert(index, tool);
	if (!m_taskName.isEmpty())
		tool->setOwnerQueueName(m_taskName.toStdString());
}

void TaskItem::removeToolAt(int index)
{
	if (index < 0 || index >= m_tools.size())
		return;
	m_tools.removeAt(index);
}

void TaskItem::removeTool(QString toolName)
{
	for (int i = 0; i < m_tools.size(); ++i)
	{
		if (m_tools[i]->ToolName() == toolName.toStdString())
		{
			m_tools.removeAt(i);
			return;
		}
	}
}

FrameToolBase* TaskItem::getTool(QString toolName)
{
	for (FrameToolBase* tool : m_tools)
	{
		if (tool->ToolName() == toolName.toStdString())
			return tool;
	}
	return nullptr;
}

FrameToolBase* TaskItem::getToolAt(int index)
{
	if (index < 0 || index >= m_tools.size())
		return nullptr;
	return m_tools[index];
}

QVector<FrameToolBase*> TaskItem::getTools()
{
	return m_tools;
}

void TaskItem::setTools(QVector<FrameToolBase*> tools)
{
	m_tools = tools;
	if (!m_taskName.isEmpty())
	{
		for (FrameToolBase* tool : m_tools)
			tool->setOwnerQueueName(m_taskName.toStdString());
	}
}

void TaskItem::clearTools()
{
	m_tools.clear();
}

// —— 相机 ——
void TaskItem::addCamera(FrameCameraBase* camera)
{
	if (camera)
		m_cameras.append(camera);
}

void TaskItem::addCameraAt(int index, FrameCameraBase* camera)
{
	if (camera)
		m_cameras.insert(index, camera);
}

void TaskItem::removeCameraAt(int index)
{
	if (index < 0 || index >= m_cameras.size())
		return;
	m_cameras.removeAt(index);
}

// —— 队列 ——
std::any TaskItem::dequeueData()
{
	if (!m_dataQueue)
		return {};
	std::any out;
	m_dataQueue->dequeue(out);
	return out;
}

std::shared_ptr<FrameDataQueue> TaskItem::dataQueue() const
{
	return m_dataQueue;
}
