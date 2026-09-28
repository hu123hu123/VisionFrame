#pragma once
#include <string>
#include <vector>
#include <map>
#include <any>

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

// 引脚数据类型（蓝图式节点端口）。Image 引脚内部承载 cv::Mat（由图像工具与显示层解释）。
enum class PinType { Image, Bool, Int, Double, String, Any };

// 引脚（端口）描述。
struct FPort
{
	std::string name;
	PinType     type = PinType::Any;
	bool        isInput = true;   // true=输入引脚, false=输出引脚
	bool        optional = false; // 输入引脚是否可缺省
};

// 引脚数据：声明为 Image 的引脚其 std::any 内为 cv::Mat。
using NodeData = std::any;

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

	// 声明节点端口（蓝图式引脚）。默认返回空（既有队列工具无引脚）。
	virtual void GetPorts(std::vector<FPort>& ports) const { (void)ports; }

	// 数据流执行：按端口名接收输入、写出输出。默认回退到无参 execute()，
	// 以兼容不声明引脚的历史工具。
	virtual ToolResult execute(const std::map<std::string, NodeData>& in,
		std::map<std::string, NodeData>& out)
	{
		(void)in; (void)out;
		return execute();
	}
};
