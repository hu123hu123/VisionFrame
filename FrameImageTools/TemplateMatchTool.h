#pragma once
#include "FrameToolBase.h"
#include <opencv2/imgproc.hpp>

// 模板匹配工具：输入 Image（搜索图），用训练好的模板做模板匹配。
// 输出：Image(标注匹配框)、Score(匹配得分，统一为“越高越好”)、X/Y(匹配左上角)、Found(是否过阈值)。
class TemplateMatchTool : public FrameToolBase
{
public:
    TemplateMatchTool();
    ~TemplateMatchTool() override;

    std::string ToolName() override;
    ToolResult execute() override;
    ToolResult execute(const std::map<std::string, NodeData>& in, std::map<std::string, NodeData>& out) override;
    void LoadParam(std::string strFilePath) override;
    void SaveParam(std::string strFilePath) override;
    void GetPorts(std::vector<FPort>& ports) const override;

    std::string SaveParamsToJson() const override;
    void LoadParamsFromJson(const std::string& json) override;

    bool hasTemplate() const { return !m_template.empty(); }
    const cv::Mat& templateImage() const { return m_template; }
    void setTemplate(const cv::Mat& t) { m_template = t.empty() ? cv::Mat() : t.clone(); }

    int    method() const { return m_method; }
    void   setMethod(int m) { m_method = m; }

    double threshold() const { return m_threshold; }
    void   setThreshold(double v) { m_threshold = v; }

private:
    cv::Mat m_template;
    int     m_method{ cv::TM_CCOEFF_NORMED };
    double  m_threshold{ 0.8 };
};