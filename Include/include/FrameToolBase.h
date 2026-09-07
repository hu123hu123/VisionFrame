#pragma once
#include <string>

#ifdef FARMETOOLBASE_EXPORTS
#define FARMETOOLBASE_API __declspec(dllexport)
#else
#define FARMETOOLBASE_API __declspec(dllimport)
#endif

enum ToolResult
{

};
class FARMETOOLBASE_API FrameToolBase
{
public:
	virtual ~FrameToolBase() = default;
	virtual std::string ToolName() const = 0;
	virtual ToolResult execute() const = 0;
	virtual void LoadParam(std::string strFilePath) const = 0;
	virtual void SaveParam(std::string strFilePath) const = 0;
};

