#pragma once

#include <QObject>
#include <QString>
#include <QDataStream>
#include <QFile>
#include <QDebug>
#include "TaskItem.h"

class ProjectConfig : public QObject
{
    Q_OBJECT
        Q_PROPERTY(QString projectName READ projectName WRITE setProjectName)

public:
    ProjectConfig(const ProjectConfig&) = delete;
    ProjectConfig& operator=(const ProjectConfig&) = delete;

    QVector<TaskItem*> taskItems;//任务列表

    static ProjectConfig& instance() {
        static ProjectConfig instance;
        return instance;
    }

    QString projectName() const { return m_projectName; }
    void setProjectName(const QString& name) { m_projectName = name; }
    QString prefix() const { return m_prefix; }

    void loadConfig(const QString& configFilePath);   // 从文件加载配置
    void saveConfig(const QString& configFilePath) const; // 保存为配置文件

    bool saveProject(const QString& filePath) const;  // 项目保存（JSON，预留二进制切换）
    bool loadProject(const QString& filePath);        // 项目加载

private:
    ProjectConfig(QObject* parent = nullptr) : QObject(parent) {
        // 注意：不传递无效 parent，保证安全
        qDebug() << "ProjectConfig singleton created.";
    }
    ~ProjectConfig() {
        qDebug() << "ProjectConfig singleton destroyed.";
    }

    void serialize(QDataStream& out) const;
    void deserialize(QDataStream& in);
    QString normPath(QString p) const;

private:
    QString m_projectName;
    QString m_prefix = "vfProj";
};
