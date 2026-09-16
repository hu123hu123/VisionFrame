#pragma once

#include "FrameToolBase.h"
#include "FrameDataQueue.h"
#include "FrameDeQueueToolWidget.h"
#include <string>
#include <any>
#include <memory>

// 出队工具：作为线程启用条件——TaskItem::doTask() 迭代到本工具时，
// execute() 阻塞直到源队列有数据或被 stop 唤醒。
// 源队列名默认 = 所属任务名（由 TaskItem::addTool 回填 setOwnerQueueName），
// 也可通过 setSourceQueueName 显式指向其它任务队列，实现跨线程读取。
class FrameDeQueueTool : public FrameToolBase
{
public:
	FrameDeQueueTool();
	~FrameDeQueueTool() override;

	std::string ToolName() override;
	ToolResult  execute() override;
	void        LoadParam(std::string strFilePath) override;
	void        SaveParam(std::string strFilePath) override;
	QWidget*    InitWidget();

	// 显式选择源任务/队列名（覆盖默认的 owner 名）。
	void        setSourceQueueName(const std::string& name);
	std::string sourceQueueName() const;

	// 由 TaskItem::addTool 回调注入所属任务名。仅在未显式设置源名时生效。
	void        setOwnerQueueName(const std::string& name) override;

	// 最近一次 dequeue 到的数据，供同链下游工具读取。
	std::any    lastDequeuedData() const;

private:
	void        resolveQueue();

	std::string                            m_sourceQueueName;
	bool                                   m_sourceExplicitlySet{ false };
	std::shared_ptr<FrameDataQueue>        m_queue;
	std::any                               m_lastData;
	FrameDeQueueToolWidget*                m_widget{ nullptr };
};
