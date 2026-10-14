#pragma once
#include "FrameToolBase.h"
// 图像保存工具。
class ImageSaveTool : public FrameToolBase
{
public:
    ImageSaveTool();
    ~ImageSaveTool() override;

    std::string ToolName() override;
    ToolResult execute() override;
    ToolResult execute(const std::map<std::string, NodeData>& in, std::map<std::string, NodeData>& out) override;
    void LoadParam(std::string strFilePath) override;
    void SaveParam(std::string strFilePath) override;
    void GetPorts(std::vector<FPort>& ports) const override;
    std::string SaveParamsToJson() const override;
    void LoadParamsFromJson(const std::string& json) override;

public:
    void setSavePath(const std::string& path) { m_savePath = path; }
    std::string savePath() const { return m_savePath; }
private:
    std::string m_savePath; // 保存路径
};