#include "FrameDeQueueTool.h"
#include "FrameDataQueueRegistry.h"
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>

FrameDeQueueTool::FrameDeQueueTool() = default;

FrameDeQueueTool::~FrameDeQueueTool()
{
	delete m_widget;
}

std::string FrameDeQueueTool::ToolName()
{
	return "FrameDeQueueTool";
}

void FrameDeQueueTool::resolveQueue()
{
	if (m_sourceQueueName.empty())
		return;
	if (!m_queue)
		m_queue = FrameDataQueueRegistry::instance().getOrCreate(m_sourceQueueName);
}

ToolResult FrameDeQueueTool::execute()
{
	resolveQueue();
	if (!m_queue)
		return ToolResult::ToolError; // 未配置源名

	// 阻塞点：线程启用条件。数据到达或被 stop 唤醒后返回。
	if (m_queue->dequeue(m_lastData))
		return ToolResult::ToolOk;
	return ToolResult::ToolInterrupted; // 被 stop 唤醒
}

void FrameDeQueueTool::LoadParam(std::string strFilePath)
{
	// TODO: 从文件加载 m_sourceQueueName。
	(void)strFilePath;
}

void FrameDeQueueTool::SaveParam(std::string strFilePath)
{
	// TODO: 把 m_sourceQueueName 写入参数文件。
	(void)strFilePath;
}

QWidget* FrameDeQueueTool::InitWidget()
{
	if (!m_widget)
	{
		m_widget = new FrameDeQueueToolWidget();
		m_widget->setSourceQueueName(QString::fromStdString(m_sourceQueueName));
		QObject::connect(m_widget, &FrameDeQueueToolWidget::sourceQueueNameChanged,
			[this](const QString& name) { setSourceQueueName(name.toStdString()); });
	}
	return m_widget;
}

void FrameDeQueueTool::setSourceQueueName(const std::string& name)
{
	if (m_sourceQueueName != name)
		m_queue.reset();
	m_sourceQueueName = name;
	m_sourceExplicitlySet = true;
	if (m_widget)
		m_widget->setSourceQueueName(QString::fromStdString(name));
}

std::string FrameDeQueueTool::sourceQueueName() const
{
	return m_sourceQueueName;
}

void FrameDeQueueTool::setOwnerQueueName(const std::string& name)
{
	// 仅在用户未显式指定源名时，默认从所属任务队列读取。
	if (m_sourceExplicitlySet && !m_sourceQueueName.empty())
		return;
	if (m_sourceQueueName != name)
		m_queue.reset();
	m_sourceQueueName = name;
	if (m_widget)
		m_widget->setSourceQueueName(QString::fromStdString(name));
}

std::any FrameDeQueueTool::lastDequeuedData() const
{
	return m_lastData;
}
