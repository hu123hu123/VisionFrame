#pragma once
#include <QWidget>

class QLineEdit;
class QPushButton;

// 图像源参数控件：图片路径 + 浏览按钮。
class ImageSourceToolWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImageSourceToolWidget(QWidget* parent = nullptr);

    QString imagePath() const;
    void setImagePath(const QString& path);

signals:
    void imagePathChanged(const QString& path);

private:
    QLineEdit*   m_edit{ nullptr };
    QPushButton* m_browse{ nullptr };
};