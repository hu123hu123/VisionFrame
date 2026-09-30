#include "ImageSourceTool.h"
#include <opencv2/opencv.hpp>

ImageSourceTool::ImageSourceTool() = default;
ImageSourceTool::~ImageSourceTool() = default;

std::string ImageSourceTool::ToolName()
{
    return "ImageSourceTool";
}

void ImageSourceTool::GetPorts(std::vector<FPort>& ports) const
{
    ports.clear();
    FPort out;
    out.name = "OutputImage";
    out.type = PinType::Image;
    out.isInput = false;
    ports.push_back(out);
}

ToolResult ImageSourceTool::execute()
{
    std::map<std::string, NodeData> in, out;
    return execute(in, out);
}

ToolResult ImageSourceTool::execute(const std::map<std::string, NodeData>& in, std::map<std::string, NodeData>& out)
{
    (void)in;
    out.clear();
    if (m_imagePath.empty())
        return ToolResult::ToolError;
    cv::Mat img = cv::imread(m_imagePath, cv::IMREAD_COLOR);
    if (img.empty())
        return ToolResult::ToolError;
    out["OutputImage"] = img;
    return ToolResult::ToolOk;
}

void ImageSourceTool::LoadParam(std::string strFilePath)
{
    (void)strFilePath;
}

void ImageSourceTool::SaveParam(std::string strFilePath)
{
    (void)strFilePath;
}

std::string ImageSourceTool::imagePath() const
{
    return m_imagePath;
}

void ImageSourceTool::setImagePath(const std::string& path)
{
    m_imagePath = path;
}