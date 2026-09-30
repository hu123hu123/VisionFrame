#include "TemplateMatchToolWidget.h"
#include "ui_TemplateMatchToolWidget.h"
#include "RoiImageViewWidget.h"
#include "ImageViewWidget.h"

#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QFileDialog>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

TemplateMatchToolWidget::TemplateMatchToolWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::TemplateMatchToolWidget)
{
    ui->setupUi(this);

    // 匹配方法下拉（用户数据 = cv::TemplateMatchModes）
    ui->m_methodCombo->addItem(QString::fromUtf8("相关系数匹配（归一化）"), cv::TM_CCOEFF_NORMED);
    ui->m_methodCombo->addItem(QString::fromUtf8("相关匹配（归一化）"), cv::TM_CCORR_NORMED);
    ui->m_methodCombo->addItem(QString::fromUtf8("平方差匹配（归一化）"), cv::TM_SQDIFF_NORMED);

    connect(ui->m_methodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
        [this](int idx) { emit methodChanged(ui->m_methodCombo->itemData(idx).toInt()); });
    connect(ui->m_thresholdSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
        [this](double v) { emit thresholdChanged(v); });

    connect(ui->m_browseBtn, &QPushButton::clicked, this, &TemplateMatchToolWidget::onBrowse);
    connect(ui->m_trainImageEdit, &QLineEdit::editingFinished, this,
        [this]() { loadTrainImage(ui->m_trainImageEdit->text()); });
    connect(ui->m_trainBtn, &QPushButton::clicked, this, &TemplateMatchToolWidget::onTrain);
    connect(ui->m_trainView, &RoiImageViewWidget::roiChanged, this,
        [this](const QRectF& r) {
            if (!r.isEmpty())
                ui->m_statusLabel->setText(QString::fromUtf8("已框选模板区域，点击“训练模板”提交"));
        });
}

TemplateMatchToolWidget::~TemplateMatchToolWidget() = default;

int TemplateMatchToolWidget::method() const
{
    const int idx = ui->m_methodCombo->currentIndex();
    return ui->m_methodCombo->itemData(idx).toInt();
}

void TemplateMatchToolWidget::setMethod(int m)
{
    int idx = ui->m_methodCombo->findData(m);
    ui->m_methodCombo->setCurrentIndex(idx < 0 ? 0 : idx);
}

double TemplateMatchToolWidget::threshold() const
{
    return ui->m_thresholdSpin->value();
}

void TemplateMatchToolWidget::setThreshold(double v)
{
    ui->m_thresholdSpin->setValue(v);
}

void TemplateMatchToolWidget::setTemplate(const cv::Mat& t)
{
    m_template = t.empty() ? cv::Mat() : t.clone();
    if (!m_template.empty())
        ui->m_statusLabel->setText(QString::fromUtf8("已训练模板 %1×%2").arg(m_template.cols).arg(m_template.rows));
}

void TemplateMatchToolWidget::onBrowse()
{
    const QString f = QFileDialog::getOpenFileName(this, QString::fromUtf8("选择训练图像"),
        QString(), QString::fromUtf8("图片 (*.png *.jpg *.jpeg *.bmp *.tif)"));
    if (f.isEmpty())
        return;
    ui->m_trainImageEdit->setText(f);
    loadTrainImage(f);
}

void TemplateMatchToolWidget::loadTrainImage(const QString& path)
{
    if (path.trimmed().isEmpty())
    {
        m_trainImage = cv::Mat();
        ui->m_trainView->clearImage();
        ui->m_statusLabel->setText(QString::fromUtf8("请先加载训练图像"));
        return;
    }
    cv::Mat img = cv::imread(path.toStdString(), cv::IMREAD_COLOR);
    if (img.empty())
    {
        ui->m_statusLabel->setText(QString::fromUtf8("无法加载图像"));
        return;
    }
    m_trainImage = img;
    ui->m_trainView->setImage(matToQImage(img));
    ui->m_trainView->setRoi(QRectF());
    ui->m_trainView->fitToWindow();
    ui->m_statusLabel->setText(QString::fromUtf8("已加载训练图像，请框选模板区域"));
}

void TemplateMatchToolWidget::onTrain()
{
    if (m_trainImage.empty())
    {
        ui->m_statusLabel->setText(QString::fromUtf8("请先加载训练图像"));
        return;
    }
    const QRectF roi = ui->m_trainView->roiRect();
    if (roi.isEmpty() || roi.width() < 2.0 || roi.height() < 2.0)
    {
        ui->m_statusLabel->setText(QString::fromUtf8("请先框选模板区域"));
        return;
    }
    const QRect r = roi.toRect().intersected(QRect(0, 0, m_trainImage.cols, m_trainImage.rows));
    if (r.isEmpty())
    {
        ui->m_statusLabel->setText(QString::fromUtf8("模板区域越界"));
        return;
    }
    m_template = m_trainImage(cv::Rect(r.x(), r.y(), r.width(), r.height())).clone();
    ui->m_statusLabel->setText(QString::fromUtf8("训练完成：模板 %1×%2")
        .arg(m_template.cols).arg(m_template.rows));
    emit templateTrained(m_template);
}