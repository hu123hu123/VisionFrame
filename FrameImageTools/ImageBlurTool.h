#pragma once
#include "FrameToolBase.h"

// 高斯模糊工具：输入 Image、输出 Image。
// 参数：核大小（奇数，自动规整）。
class ImageBlurTool : public FrameToolBase
{
public:
    ImageBlurTool();
    ~ImageBlurTool() override;

    std::string ToolName() override;
    ToolResult execute() override;
    ToolResult execute(const std::map<std::string, NodeData>& in, std::map<std::string, NodeData>& out) override;
    void LoadParam(std::string strFilePath) override;
    void SaveParam(std::string strFilePath) override;
    void GetPorts(std::vector<FPort>& ports) const override;

    std::string SaveParamsToJson() const override;
    void LoadParamsFromJson(const std::string& json) override;

    int  kernelSize() const;
    void setKernelSize(int k);

private:
    int m_kernelSize{ 5 };
};