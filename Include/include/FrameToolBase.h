#pragma once
#include <string>

#ifdef FARMETOOLBASE_EXPORTS
#define FARMETOOLBASE_API __declspec(dllexport)
#else
#define FARMETOOLBASE_API __declspec(dllimport)
#endif

// 工具执行结果。
enum ToolResult
{
	ToolOk = 0,           // 正常完成
	ToolInterrupted = 1, // 被 stop 唤醒（队列停止等），任务应退出
	ToolError = 2         // 执行异常
};

class FARMETOOLBASE_API FrameToolBase
{
public:
	virtual ~FrameToolBase() = default;
	virtual std::string ToolName() = 0;
	virtual ToolResult execute() = 0;
	virtual void LoadParam(std::string strFilePath) = 0;
	virtual void SaveParam(std::string strFilePath) = 0;

	// 由 TaskItem 在 addTool 时回调，把"所属任务的队列名"注入工具。
	// FrameDeQueueTool 据此默认从所属任务队列出队（线程启用条件）；
	// 其它工具默认空实现。
	virtual void setOwnerQueueName(const std::string& name) { (void)name; }
};
