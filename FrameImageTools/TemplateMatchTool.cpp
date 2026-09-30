#include "TemplateMatchTool.h"
#include <opencv2/imgcodecs.hpp>
#include <QJsonDocument>
#include <QJsonObject>
#include <QByteArray>

TemplateMatchTool::TemplateMatchTool() = default;
TemplateMatchTool::~TemplateMatchTool() = default;

std::string TemplateMatchTool::ToolName()
{
    return "TemplateMatchTool";
}

void TemplateMatchTool::GetPorts(std::vector<FPort>& ports) const
{
    ports.clear();

    FPort in;
    in.name = "InputImage";
    in.type = PinType::Image;
    in.isInput = true;
    ports.push_back(in);

    FPort outImg;
    outImg.name = "OutputImage";
    outImg.type = PinType::Image;
    outImg.isInput = false;
    ports.push_back(outImg);

    FPort outScore;
    outScore.name = "Score";
    outScore.type = PinType::Double;
    outScore.isInput = false;
    ports.push_back(outScore);

    FPort outX;
    outX.name = "X";
    outX.type = PinType::Double;
    outX.isInput = false;
    ports.push_back(outX);

    FPort outY;
    outY.name = "Y";
    outY.type = PinType::Double;
    outY.isInput = false;
    ports.push_back(outY);

    FPort outFound;
    outFound.name = "Found";
    outFound.type = PinType::Bool;
    outFound.isInput = false;
    ports.push_back(outFound);
}

ToolResult TemplateMatchTool::execute()
{
    std::map<std::string, NodeData> in, out;
    return execute(in, out);
}

ToolResult TemplateMatchTool::execute(const std::map<std::string, NodeData>& in, std::map<std::string, NodeData>& out)
{
    out.clear();

    if (m_template.empty())
        return ToolResult::ToolError;

    auto it = in.find("InputImage");
    if (it == in.end())
        return ToolResult::ToolError;
    const cv::Mat* src = std::any_cast<cv::Mat>(&it->second);
    if (!src || src->empty())
        return ToolResult::ToolError;

    // 统一转灰度做匹配，避免模板与搜索图通道数不一致
    cv::Mat srcGray, tplGray;
    if (src->channels() == 3)
        cv::cvtColor(*src, srcGray, cv::COLOR_BGR2GRAY);
    else
        srcGray = *src;
    if (m_template.channels() == 3)
        cv::cvtColor(m_template, tplGray, cv::COLOR_BGR2GRAY);
    else
        tplGray = m_template;

    if (srcGray.cols < tplGray.cols || srcGray.rows < tplGray.rows)
        return ToolResult::ToolError;

    cv::Mat result;
    cv::matchTemplate(srcGray, tplGray, result, m_method);

    double minVal = 0.0, maxVal = 0.0;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

    const bool lowerBetter = (m_method == cv::TM_SQDIFF || m_method == cv::TM_SQDIFF_NORMED);
    const double score = lowerBetter ? (1.0 - minVal) : maxVal; // 统一为“越高越好”
    const cv::Point loc = lowerBetter ? minLoc : maxLoc;
    const bool found = (score >= m_threshold);

    cv::Mat vis = src->clone();
    const cv::Scalar color = (vis.channels() == 1) ? cv::Scalar(255) : cv::Scalar(0, 0, 255);
    cv::rectangle(vis, loc, cv::Point(loc.x + tplGray.cols, loc.y + tplGray.rows), color, 2);

    out["OutputImage"] = vis;
    out["Score"] = score;
    out["X"] = static_cast<double>(loc.x);
    out["Y"] = static_cast<double>(loc.y);
    out["Found"] = found;
    return ToolResult::ToolOk;
}

void TemplateMatchTool::LoadParam(std::string strFilePath)
{
    (void)strFilePath;
}

void TemplateMatchTool::SaveParam(std::string strFilePath)
{
    (void)strFilePath;
}

std::string TemplateMatchTool::SaveParamsToJson() const
{
    QJsonObject o;
    o["method"] = m_method;
    o["threshold"] = m_threshold;
    if (!m_template.empty())
    {
        std::vector<uchar> buf;
        cv::imencode(".png", m_template, buf);
        QByteArray data(reinterpret_cast<const char*>(buf.data()), static_cast<int>(buf.size()));
        o["template"] = QString::fromLatin1(data.toBase64());
    }
    else
    {
        o["template"] = QString();
    }
    return QJsonDocument(o).toJson(QJsonDocument::Compact).toStdString();
}

void TemplateMatchTool::LoadParamsFromJson(const std::string& json)
{
    QJsonObject o = QJsonDocument::fromJson(QByteArray::fromStdString(json)).object();
    m_method = o["method"].toInt(m_method);
    m_threshold = o["threshold"].toDouble(m_threshold);
    const QByteArray b64 = o["template"].toString().toLatin1();
    if (!b64.isEmpty())
    {
        QByteArray raw = QByteArray::fromBase64(b64);
        std::vector<uchar> buf(raw.begin(), raw.end());
        m_template = cv::imdecode(buf, cv::IMREAD_COLOR);
    }
    else
    {
        m_template = cv::Mat();
    }
}