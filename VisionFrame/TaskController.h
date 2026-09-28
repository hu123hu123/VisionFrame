#pragma once
#include <QVector>
#include "TaskItem.h"
#include "GlobalConfig.h"

// 任务控制器：单例，管理 ProjectConfig::taskItems 中任务的启停与增删。
class TaskController
{
public:
    void AddTask(TaskItem* task);
    void RemoveTask(TaskItem* task);
    void StartAllTasks();
    void StopAllTasks();
    void ClearAllTasks();
    void StartTaskByName(const QString& taskName);
    void StopTaskByName(const QString& taskName);
    void RemoveTaskByName(const QString& taskName);
    static TaskController& instance()
    {
        static TaskController m_instance;
        return m_instance;
    }
};