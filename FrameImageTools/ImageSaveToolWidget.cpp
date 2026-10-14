#include "ImageSaveToolWidget.h"
#include "ui_ImageSaveToolWidget.h"

#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>

namespace
{
    const char* kSuffixes[] = { "png", "jpg", "bmp", "tif" };
}

ImageSaveToolWidget::ImageSaveToolWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ImageSaveToolWidget)
{
    ui->setupUi(this);

    for (const char* s : kSuffixes)
        ui->comboBox_fileSuffix->addItem(QString(".%1").arg(QLatin1String(s)));

    connect(ui->lineEdit_filePath, &QLineEdit::editingFinished, this, [this]() {
        if (!m_updating)
            emit changed();
    });
    connect(ui->comboBox_fileSuffix, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [this](int) {
        if (!m_updating)
        {
            applySuffix();
            emit changed();
        }
    });
    connect(ui->pushButton_browse, &QPushButton::clicked, this, [this]() {
        QString suffix = ui->comboBox_fileSuffix->currentText(); // 如 ".png"
        QString f = QFileDialog::getSaveFileName(this, QString::fromUtf8("保存图像"),
            ui->lineEdit_filePath->text(),
            QString::fromUtf8("图像 (*%1)").arg(suffix));
        if (!f.isEmpty())
        {
            if (!f.endsWith(suffix, Qt::CaseInsensitive))
                f += suffix;
            ui->lineEdit_filePath->setText(f);
            emit changed();
        }
    });
}

ImageSaveToolWidget::~ImageSaveToolWidget()
{
    delete ui;
}

std::string ImageSaveToolWidget::savePath() const
{
    return ui->lineEdit_filePath->text().toStdString();
}

void ImageSaveToolWidget::setSavePath(const std::string& path)
{
    m_updating = true;
    ui->lineEdit_filePath->setText(QString::fromStdString(path));

    // 依据既有扩展名选中格式项
    QString suffix = QFileInfo(QString::fromStdString(path)).suffix().toLower();
    if (!suffix.isEmpty())
        suffix.prepend('.');
    int idx = ui->comboBox_fileSuffix->findText(suffix);
    if (idx >= 0)
        ui->comboBox_fileSuffix->setCurrentIndex(idx);

    m_updating = false;
}

void ImageSaveToolWidget::applySuffix()
{
    QString suffix = ui->comboBox_fileSuffix->currentText();
    QString path = ui->lineEdit_filePath->text();
    if (path.isEmpty())
        return;
    QString dir = QFileInfo(path).path();
    QString base = QFileInfo(path).completeBaseName();
    ui->lineEdit_filePath->setText(QString("%1/%2%3").arg(dir, base, suffix));
}
