#pragma once
#include "FrameToolBase.h"
#include <opencv2/imgproc.hpp>

// 仿射变换工具：输入 InputImage，按 XYZR（X=平移X、Y=平移Y、Z=全局缩放、R=旋转角度）做相似变换，输出 OutputImage。
// X/Y/Z/R 也可从上游引脚输入；引脚无值时回退到 UI 参数。
class AffineTransformTool : public FrameToolBase
{
public:
    AffineTransformTool();
    ~AffineTransformTool() override;

    std::string ToolName() override;
    ToolResult execute() override;
    ToolResult execute(const std::map<std::string, NodeData>& in, std::map<std::string, NodeData>& out) override;
    void LoadParam(std::string strFilePath) override;
    void SaveParam(std::string strFilePath) override;
    void GetPorts(std::vector<FPort>& ports) const override;

    double angle() const { return m_angle; }
    void   setAngle(double a) { m_angle = a; }
    double scale() const { return m_scale; }
    void   setScale(double s) { m_scale = s; }
    double transX() const { return m_transX; }
    void   setTransX(double t) { m_transX = t; }
    double transY() const { return m_transY; }
    void   setTransY(double t) { m_transY = t; }
    int    interpolation() const { return m_interpolation; }
    void   setInterpolation(int i) { m_interpolation = i; }

private:
    double m_angle{ 0.0 };
    double m_scale{ 1.0 };
    double m_transX{ 0.0 };
    double m_transY{ 0.0 };
    int    m_interpolation{ cv::INTER_LINEAR };
};