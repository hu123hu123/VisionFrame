#include "ImageBlurTool.h"
#include <opencv2/opencv.hpp>

ImageBlurTool::ImageBlurTool() = default;
ImageBlurTool::~ImageBlurTool() = default;

std::string ImageBlurTool::ToolName()
{
    return "ImageBlurTool";
}

void ImageBlurTool::GetPorts(std::vector<FPort>& ports) const
{
    ports.clear();
    FPort in;
    in.name = "Image";
    in.type = PinType::Image;
    in.isInput = true;
    ports.push_back(in);
    FPort out;
    out.name = "Image";
    out.type = PinType::Image;
    out.isInput = false;
    ports.push_back(out);
}

ToolResult ImageBlurTool::execute()
{
    std::map<std::string, NodeData> in, out;
    return execute(in, out);
}

ToolResult ImageBlurTool::execute(const std::map<std::string, NodeData>& in, std::map<std::string, NodeData>& out)
{
    out.clear();
    auto it = in.find("Image");
    if (it == in.end())
        return ToolResult::ToolError;
    const cv::Mat* src = std::any_cast<cv::Mat>(&it->second);
    if (!src || src->empty())
        return ToolResult::ToolError;

    int k = m_kernelSize;
    if (k < 1)
        k = 1;
    if (k % 2 == 0)
        k += 1;

    cv::Mat dst;
    cv::GaussianBlur(*src, dst, cv::Size(k, k), 0);
    out["Image"] = dst;
    return ToolResult::ToolOk;
}

void ImageBlurTool::LoadParam(std::string strFilePath)
{
    (void)strFilePath;
}

void ImageBlurTool::SaveParam(std::string strFilePath)
{
    (void)strFilePath;
}

int ImageBlurTool::kernelSize() const
{
    return m_kernelSize;
}

void ImageBlurTool::setKernelSize(int k)
{
    m_kernelSize = k;
}