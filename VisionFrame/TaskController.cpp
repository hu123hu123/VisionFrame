#include "TaskController.h"

namespace
{
    TaskItem* findByName(const QString& name)
    {
        const auto& items = GlobalConfig::instance().projectConfig().taskItems;
        for (TaskItem* t : items)
        {
            if (t->taskName() == name)
                return t;
        }
        return nullptr;
    }
}

void TaskController::AddTask(TaskItem* task)
{
    if (task)
        GlobalConfig::instance().projectConfig().taskItems.append(task);
}

void TaskController::RemoveTask(TaskItem* task)
{
    if (!task)
        return;
    task->StopTask();
    auto& items = GlobalConfig::instance().projectConfig().taskItems;
    items.removeAll(task);
    delete task;
}

void TaskController::StartAllTasks()
{
    const auto& items = GlobalConfig::instance().projectConfig().taskItems;
    for (TaskItem* t : items)
        t->StartTask();
}

void TaskController::StopAllTasks()
{
    const auto& items = GlobalConfig::instance().projectConfig().taskItems;
    for (TaskItem* t : items)
        t->StopTask();
}

void TaskController::ClearAllTasks()
{
    auto& items = GlobalConfig::instance().projectConfig().taskItems;
    for (TaskItem* t : items)
    {
        t->StopTask();
        delete t;
    }
    items.clear();
}

void TaskController::StartTaskByName(const QString& taskName)
{
    TaskItem* t = findByName(taskName);
    if (t)
        t->StartTask();
}

void TaskController::StopTaskByName(const QString& taskName)
{
    TaskItem* t = findByName(taskName);
    if (t)
        t->StopTask();
}

void TaskController::RemoveTaskByName(const QString& taskName)
{
    auto& items = GlobalConfig::instance().projectConfig().taskItems;
    for (int i = 0; i < items.size(); ++i)
    {
        if (items[i]->taskName() == taskName)
        {
            items[i]->StopTask();
            delete items[i];
            items.removeAt(i);
            return;
        }
    }
}