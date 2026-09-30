#include "ImageGrayscaleTool.h"
#include <opencv2/opencv.hpp>

ImageGrayscaleTool::ImageGrayscaleTool() = default;
ImageGrayscaleTool::~ImageGrayscaleTool() = default;

std::string ImageGrayscaleTool::ToolName()
{
    return "ImageGrayscaleTool";
}

void ImageGrayscaleTool::GetPorts(std::vector<FPort>& ports) const
{
    ports.clear();
    FPort in;
    in.name = "InputImage";
    in.type = PinType::Image;
    in.isInput = true;
    ports.push_back(in);
    FPort out;
    out.name = "OutputImage";
    out.type = PinType::Image;
    out.isInput = false;
    ports.push_back(out);
}

ToolResult ImageGrayscaleTool::execute()
{
    std::map<std::string, NodeData> in, out;
    return execute(in, out);
}

ToolResult ImageGrayscaleTool::execute(const std::map<std::string, NodeData>& in, std::map<std::string, NodeData>& out)
{
    out.clear();
    auto it = in.find("InputImage");
    if (it == in.end())
        return ToolResult::ToolError;
    const cv::Mat* src = std::any_cast<cv::Mat>(&it->second);
    if (!src || src->empty())
        return ToolResult::ToolError;

    cv::Mat dst;
    cv::cvtColor(*src, dst, cv::COLOR_BGR2GRAY);
    out["OutputImage"] = dst;
    return ToolResult::ToolOk;
}

void ImageGrayscaleTool::LoadParam(std::string strFilePath)
{
    (void)strFilePath;
}

void ImageGrayscaleTool::SaveParam(std::string strFilePath)
{
    (void)strFilePath;
}