#pragma once
#include <QWidget>

namespace Ui { class AffineTransformToolWidget; }

// 仿射变换参数控件：旋转角度、缩放、平移、插值方式。
class AffineTransformToolWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AffineTransformToolWidget(QWidget* parent = nullptr);
    ~AffineTransformToolWidget() override;

    double angle() const;
    void   setAngle(double v);
    double scale() const;
    void   setScale(double v);
    double transX() const;
    void   setTransX(double v);
    double transY() const;
    void   setTransY(double v);
    int    interpolation() const;
    void   setInterpolation(int v);

signals:
    void changed();

private:
    Ui::AffineTransformToolWidget* ui{ nullptr };
};