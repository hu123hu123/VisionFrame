#pragma once

#include <QWidget>
#include <QVector>

class QTimer;
class TaskItem;

namespace Ui { class ImageDisplayWidget; }

// 图像显示面板：任务 -> 工具 -> 图片输出 三级下拉选择并渲染 cv::Mat。
// 选择区（三下拉）可经顶部按钮隐藏，隐藏后图像区占满面板。
class ImageDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImageDisplayWidget(QWidget* parent = nullptr);
    ~ImageDisplayWidget() override;

    void setTasks(const QVector<TaskItem*>& tasks);
    void refreshTasks(const QVector<TaskItem*>& tasks, TaskItem* current = nullptr);
    void refreshCombos();
    TaskItem* currentTask() const;

private slots:
    void onTaskChanged(int idx);
    void onToolChanged(int idx);

private:
    void rebuildToolCombo();
    void rebuildOutputCombo();
    void reloadPixmap();

    Ui::ImageDisplayWidget* ui{ nullptr };
    QTimer* m_timer{ nullptr };
    QVector<TaskItem*> m_tasks;
};