#include "ImageBlurToolWidget.h"
#include <QSpinBox>
#include <QLabel>
#include <QHBoxLayout>

ImageBlurToolWidget::ImageBlurToolWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    auto* label = new QLabel(QString::fromUtf8("核大小:"), this);
    m_spin = new QSpinBox(this);
    m_spin->setRange(1, 99);
    m_spin->setSingleStep(2);
    m_spin->setValue(5);
    lay->addWidget(label);
    lay->addWidget(m_spin);
    lay->addStretch(1);

    connect(m_spin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int v) {
        emit kernelSizeChanged(v);
    });
}

int ImageBlurToolWidget::kernelSize() const
{
    return m_spin->value();
}

void ImageBlurToolWidget::setKernelSize(int k)
{
    m_spin->setValue(k);
}