#pragma once
#include "FrameToolBase.h"

// 灰度工具：输入 Image（彩色）、输出 Image（灰度）。
class ImageGrayscaleTool : public FrameToolBase
{
public:
    ImageGrayscaleTool();
    ~ImageGrayscaleTool() override;

    std::string ToolName() override;
    ToolResult execute() override;
    ToolResult execute(const std::map<std::string, NodeData>& in, std::map<std::string, NodeData>& out) override;
    void LoadParam(std::string strFilePath) override;
    void SaveParam(std::string strFilePath) override;
    void GetPorts(std::vector<FPort>& ports) const override;
};