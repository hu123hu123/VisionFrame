#include "AffineTransformToolWidget.h"
#include "ui_AffineTransformToolWidget.h"

#include <QDoubleSpinBox>
#include <QComboBox>
#include <opencv2/imgproc.hpp>

AffineTransformToolWidget::AffineTransformToolWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::AffineTransformToolWidget)
{
    ui->setupUi(this);

    ui->m_interpolationCombo->addItem(QString::fromUtf8("最近邻"), cv::INTER_NEAREST);
    ui->m_interpolationCombo->addItem(QString::fromUtf8("线性"), cv::INTER_LINEAR);
    ui->m_interpolationCombo->addItem(QString::fromUtf8("三次样条"), cv::INTER_CUBIC);

    connect(ui->m_angleSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double) { emit changed(); });
    connect(ui->m_scaleSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double) { emit changed(); });
    connect(ui->m_transXSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double) { emit changed(); });
    connect(ui->m_transYSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double) { emit changed(); });
    connect(ui->m_interpolationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) { emit changed(); });
}

AffineTransformToolWidget::~AffineTransformToolWidget() = default;

double AffineTransformToolWidget::angle() const { return ui->m_angleSpin->value(); }
void AffineTransformToolWidget::setAngle(double v) { ui->m_angleSpin->setValue(v); }

double AffineTransformToolWidget::scale() const { return ui->m_scaleSpin->value(); }
void AffineTransformToolWidget::setScale(double v) { ui->m_scaleSpin->setValue(v); }

double AffineTransformToolWidget::transX() const { return ui->m_transXSpin->value(); }
void AffineTransformToolWidget::setTransX(double v) { ui->m_transXSpin->setValue(v); }

double AffineTransformToolWidget::transY() const { return ui->m_transYSpin->value(); }
void AffineTransformToolWidget::setTransY(double v) { ui->m_transYSpin->setValue(v); }

int AffineTransformToolWidget::interpolation() const { return ui->m_interpolationCombo->currentData().toInt(); }
void AffineTransformToolWidget::setInterpolation(int v)
{
    int idx = ui->m_interpolationCombo->findData(v);
    ui->m_interpolationCombo->setCurrentIndex(idx < 0 ? 0 : idx);
}