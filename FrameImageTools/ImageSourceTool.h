#pragma once
#include "FrameToolBase.h"

// 图像源工具：无输入，输出一个 Image 引脚（cv::Mat）。
// 参数：图片文件路径。
class ImageSourceTool : public FrameToolBase
{
public:
    ImageSourceTool();
    ~ImageSourceTool() override;

    std::string ToolName() override;
    ToolResult execute() override;
    ToolResult execute(const std::map<std::string, NodeData>& in, std::map<std::string, NodeData>& out) override;
    void LoadParam(std::string strFilePath) override;
    void SaveParam(std::string strFilePath) override;
    void GetPorts(std::vector<FPort>& ports) const override;

    std::string SaveParamsToJson() const override;
    void LoadParamsFromJson(const std::string& json) override;

    std::string imagePath() const;
    void setImagePath(const std::string& path);

private:
    std::string m_imagePath;
};