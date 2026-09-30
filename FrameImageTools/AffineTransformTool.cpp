#include "AffineTransformTool.h"
#include <QJsonDocument>
#include <QJsonObject>

AffineTransformTool::AffineTransformTool() = default;
AffineTransformTool::~AffineTransformTool() = default;

std::string AffineTransformTool::ToolName()
{
    return "AffineTransformTool";
}

void AffineTransformTool::GetPorts(std::vector<FPort>& ports) const
{
    ports.clear();

    auto addDouble = [&ports](const char* name) {
        FPort p;
        p.name = name;
        p.type = PinType::Double;
        p.isInput = true;
        p.optional = true;
        ports.push_back(p);
    };
    addDouble("X");
    addDouble("Y");
    addDouble("Scale");
    addDouble("Rotation");

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

ToolResult AffineTransformTool::execute()
{
    std::map<std::string, NodeData> in, out;
    return execute(in, out);
}

ToolResult AffineTransformTool::execute(const std::map<std::string, NodeData>& in, std::map<std::string, NodeData>& out)
{
    out.clear();

    // X/Y/Z/R 从上游引脚读取，引脚未接则回退到 UI 参数。
    auto readDouble = [&in](const char* name, double fallback) -> double {
        auto it = in.find(name);
        if (it == in.end())
            return fallback;
        const double* v = std::any_cast<double>(&it->second);
        return v ? *v : fallback;
    };

    const double transX = readDouble("X", m_transX);
    const double transY = readDouble("Y", m_transY);
    const double scale  = readDouble("Scale", m_scale);
    const double angle  = readDouble("Rotation", m_angle);

    auto it = in.find("InputImage");
    if (it == in.end())
        return ToolResult::ToolError;
    const cv::Mat* src = std::any_cast<cv::Mat>(&it->second);
    if (!src || src->empty())
        return ToolResult::ToolError;

    const cv::Point2f center(static_cast<float>(src->cols) / 2.0f, static_cast<float>(src->rows) / 2.0f);
    cv::Mat m = cv::getRotationMatrix2D(center, angle, scale);
    m.at<double>(0, 2) += transX;
    m.at<double>(1, 2) += transY;

    cv::Mat dst;
    cv::warpAffine(*src, dst, m, src->size(), m_interpolation, cv::BORDER_CONSTANT, cv::Scalar());
    out["OutputImage"] = dst;
    return ToolResult::ToolOk;
}

void AffineTransformTool::LoadParam(std::string strFilePath)
{
    (void)strFilePath;
}

void AffineTransformTool::SaveParam(std::string strFilePath)
{
    (void)strFilePath;
}

std::string AffineTransformTool::SaveParamsToJson() const
{
    QJsonObject o;
    o["angle"] = m_angle;
    o["scale"] = m_scale;
    o["transX"] = m_transX;
    o["transY"] = m_transY;
    o["interpolation"] = m_interpolation;
    return QJsonDocument(o).toJson(QJsonDocument::Compact).toStdString();
}

void AffineTransformTool::LoadParamsFromJson(const std::string& json)
{
    QJsonObject o = QJsonDocument::fromJson(QByteArray::fromStdString(json)).object();
    m_angle = o["angle"].toDouble(m_angle);
    m_scale = o["scale"].toDouble(m_scale);
    m_transX = o["transX"].toDouble(m_transX);
    m_transY = o["transY"].toDouble(m_transY);
    m_interpolation = o["interpolation"].toInt(m_interpolation);
}