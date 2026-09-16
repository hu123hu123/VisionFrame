#include "TaskItem.h"
#include "FrameDataQueueRegistry.h"

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
	if (m_dataQueue)
		m_dataQueue->reset(); // 清除可能的 stop 标志，允许复用
	if (!this->isRunning())
		this->start();
}

void TaskItem::StopTask()
{
	m_stop = true;
	// 唤醒所有阻塞在 DeQueueTool::execute() 中的等待者，让线程优雅退出。
	if (m_dataQueue)
		m_dataQueue->stop();
	this->requestInterruption();
	this->wait();
}

void TaskItem::run()
{
	while (!m_stop)
	{
		doTask();
	}
}

void TaskItem::doTask()
{
	for (FrameToolBase* tool : m_tools)
	{
		if (m_stop)
			return;
		tool->execute(); // FrameDeQueueTool 在此阻塞，作为线程启用条件
		if (m_stop)
			return;
	}
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
