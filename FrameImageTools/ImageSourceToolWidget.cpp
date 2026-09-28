#include "ImageSourceToolWidget.h"
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>

ImageSourceToolWidget::ImageSourceToolWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    m_edit = new QLineEdit(this);
    m_edit->setPlaceholderText(QString::fromUtf8("图片文件路径..."));
    m_browse = new QPushButton(QString::fromUtf8("浏览..."), this);
    lay->addWidget(m_edit, 1);
    lay->addWidget(m_browse);

    connect(m_edit, &QLineEdit::editingFinished, this, [this]() {
        emit imagePathChanged(m_edit->text());
    });
    connect(m_browse, &QPushButton::clicked, this, [this]() {
        QString f = QFileDialog::getOpenFileName(this, QString::fromUtf8("选择图片"),
            QString(), QString::fromUtf8("图片 (*.png *.jpg *.jpeg *.bmp *.tif)"));
        if (!f.isEmpty())
        {
            m_edit->setText(f);
            emit imagePathChanged(f);
        }
    });
}

QString ImageSourceToolWidget::imagePath() const
{
    return m_edit->text();
}

void ImageSourceToolWidget::setImagePath(const QString& path)
{
    m_edit->setText(path);
}