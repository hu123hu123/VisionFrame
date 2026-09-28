#pragma once
#include <QWidget>

class QSpinBox;

// 高斯模糊参数控件：核大小。
class ImageBlurToolWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImageBlurToolWidget(QWidget* parent = nullptr);

    int  kernelSize() const;
    void setKernelSize(int k);

signals:
    void kernelSizeChanged(int k);

private:
    QSpinBox* m_spin{ nullptr };
};