#pragma once
#include <QWidget>

namespace Ui { class ImageSaveToolWidget; }

// 图像保存参数控件：保存路径 + 文件格式。修改即发 changed() 由宿主同步回工具。
class ImageSaveToolWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImageSaveToolWidget(QWidget* parent = nullptr);
    ~ImageSaveToolWidget() override;

    std::string savePath() const;
    void setSavePath(const std::string& path);

signals:
    void changed();

private:
    void applySuffix(); // 用当前格式替换路径中的扩展名

    Ui::ImageSaveToolWidget* ui{ nullptr };
    bool m_updating{ false }; // 程序设值时抑制 changed()
};
