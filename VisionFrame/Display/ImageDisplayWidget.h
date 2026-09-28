#pragma once

#include <QWidget>
#include <QVector>

class QComboBox;
class QTimer;
class TaskItem;
class ImageView;

// 图像显示面板：任务 -> 工具 -> 图片输出 三级下拉选择并渲染 cv::Mat。
class ImageDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImageDisplayWidget(QWidget* parent = nullptr);

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

    QComboBox* m_taskCombo{ nullptr };
    QComboBox* m_toolCombo{ nullptr };
    QComboBox* m_outputCombo{ nullptr };
    QTimer* m_timer{ nullptr };
    QVector<TaskItem*> m_tasks;
    ImageView* m_view{ nullptr };
};