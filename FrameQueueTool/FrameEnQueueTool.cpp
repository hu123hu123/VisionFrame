#include "FrameEnQueueTool.h"
#include "FrameDataQueueRegistry.h"
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>

FrameEnQueueTool::FrameEnQueueTool() = default;

FrameEnQueueTool::~FrameEnQueueTool()
{
	delete m_widget;
}

std::string FrameEnQueueTool::ToolName()
{
	return "FrameEnQueueTool";
}

void FrameEnQueueTool::resolveQueue()
{
	if (m_targetQueueName.empty())
		return;
	if (!m_queue)
		m_queue = FrameDataQueueRegistry::instance().getOrCreate(m_targetQueueName);
}

ToolResult FrameEnQueueTool::execute()
{
	resolveQueue();
	if (!m_queue)
		return ToolResult::ToolError; // 未配置目标任务名

	// 未显式设置 payload 时入队空 any，否则入队指定 payload
	m_queue->enqueue(m_hasPayload ? m_payload : std::any());
	return ToolResult::ToolOk;
}

void FrameEnQueueTool::LoadParam(std::string strFilePath)
{
	// TODO: 从文件加载 m_targetQueueName（参数文件路径/格式后续统一约定）。
	(void)strFilePath;
}

void FrameEnQueueTool::SaveParam(std::string strFilePath)
{
	// TODO: 把 m_targetQueueName 写入参数文件。
	(void)strFilePath;
}

QWidget* FrameEnQueueTool::InitWidget()
{
	if (!m_widget)
	{
		m_widget = new FrameEnQueueToolWidget();
		// 把当前配置同步到界面
		m_widget->setTargetQueueName(QString::fromStdString(m_targetQueueName));
		// 界面编辑后回写工具配置
		QObject::connect(m_widget, &FrameEnQueueToolWidget::targetQueueNameChanged,
			[this](const QString& name) { m_targetQueueName = name.toStdString(); m_queue.reset(); });
	}
	return m_widget;
}

void FrameEnQueueTool::setTargetQueueName(const std::string& name)
{
	if (m_targetQueueName != name)
		m_queue.reset(); // 目标换了，重新懒解析
	m_targetQueueName = name;
	if (m_widget)
		m_widget->setTargetQueueName(QString::fromStdString(name));
}

std::string FrameEnQueueTool::targetQueueName() const
{
	return m_targetQueueName;
}

void FrameEnQueueTool::setPayload(std::any payload)
{
	m_payload = std::move(payload);
	m_hasPayload = true;
}
