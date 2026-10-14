#include "ImageSaveTool.h"
#include <opencv2/opencv.hpp>
#include <QJsonDocument>
#include <QJsonObject>

ImageSaveTool::ImageSaveTool()
{
}

ImageSaveTool::~ImageSaveTool()
{
}

std::string ImageSaveTool::ToolName()
{
    return "ImageSaveTool";
}

ToolResult ImageSaveTool::execute()
{
    std::map<std::string, NodeData> in, out;
    return execute(in, out);
}

ToolResult ImageSaveTool::execute(const std::map<std::string, NodeData>& in, std::map<std::string, NodeData>& out)
{
    (void)out;
    out.clear();
    if (m_savePath.empty())
        return ToolResult::ToolError;
    auto it = in.find("InputImage");
    if (it == in.end() || it->second.type() != typeid(cv::Mat))
        return ToolResult::ToolError;
    const auto& img = std::any_cast<const cv::Mat&>(it->second);
    if (img.empty())
        return ToolResult::ToolError;
    if (!cv::imwrite(m_savePath, img))
        return ToolResult::ToolError;
    return ToolResult::ToolOk;
}

void ImageSaveTool::LoadParam(std::string strFilePath)
{
    (void)strFilePath; // 参数由 JSON 序列化承载
}

void ImageSaveTool::SaveParam(std::string strFilePath)
{
    (void)strFilePath;
}
std::string ImageSaveTool::SaveParamsToJson() const
{
    QJsonObject o;
    o["savePath"] = QString::fromStdString(m_savePath);
    return QJsonDocument(o).toJson(QJsonDocument::Compact).toStdString();
}

void ImageSaveTool::LoadParamsFromJson(const std::string& json)
{
    QJsonObject o = QJsonDocument::fromJson(QByteArray::fromStdString(json)).object();
    m_savePath = o["savePath"].toString().toStdString();
}
void ImageSaveTool::GetPorts(std::vector<FPort>& ports) const
{
    // Implementation for getting the ports of the image save tool
        ports.clear();
    FPort in;
    in.name = "InputImage";
    in.type = PinType::Image;
    in.isInput = true;
    ports.push_back(in);
}