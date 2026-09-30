#pragma once
#include <QWidget>
#include <opencv2/core.hpp>

namespace Ui { class TemplateMatchToolWidget; }

// 模板匹配参数控件：加载训练图像、在预览图上框选模板 ROI、设置匹配方法与阈值、训练模板。
class TemplateMatchToolWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TemplateMatchToolWidget(QWidget* parent = nullptr);
    ~TemplateMatchToolWidget() override;

    int    method() const;
    void   setMethod(int m);
    double threshold() const;
    void   setThreshold(double v);

    const cv::Mat& templateImage() const { return m_template; }
    void           setTemplate(const cv::Mat& t);

signals:
    void methodChanged(int m);
    void thresholdChanged(double v);
    void templateTrained(const cv::Mat& tpl);

private slots:
    void onBrowse();
    void onTrain();

private:
    void loadTrainImage(const QString& path);

    Ui::TemplateMatchToolWidget* ui{ nullptr };
    cv::Mat m_template;
    cv::Mat m_trainImage;
};