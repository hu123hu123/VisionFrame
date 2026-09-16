#pragma once

#include "FrameToolBase.h"
#include "FrameDataQueue.h"
#include "FrameEnQueueToolWidget.h"
#include <string>
#include <any>
#include <memory>

// 入队工具：把数据放入"目标任务名"对应的队列。
// 目标队列名通常即目标任务(TaskItem)名——TaskItem 把自身队列以任务名注册到
// FrameDataQueueRegistry，本工具通过 setTargetQueueName 选择入队到哪个任务。
class FrameEnQueueTool : public FrameToolBase
{
public:
	FrameEnQueueTool();
	~FrameEnQueueTool() override;

	std::string ToolName() override;
	ToolResult  execute() override;
	void        LoadParam(std::string strFilePath) override;
	void        SaveParam(std::string strFilePath) override;
	QWidget*    InitWidget();

	// 选择入队目标任务（= 目标队列名）。
	void        setTargetQueueName(const std::string& name);
	std::string targetQueueName() const;

	// 设置本次要入队的 payload。未设置则入队空 std::any()。
	void        setPayload(std::any payload);

private:
	void        resolveQueue();

	std::string                            m_targetQueueName;
	std::shared_ptr<FrameDataQueue>        m_queue;
	std::any                               m_payload;
	bool                                   m_hasPayload{ false };
	FrameEnQueueToolWidget*                m_widget{ nullptr };
};
