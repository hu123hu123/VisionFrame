#pragma once

#include <QObject>
#include <QString>
#include <QDataStream>
#include <QFile>
#include <QDebug>

class ProjectConfig : public QObject
{
    Q_OBJECT
        Q_PROPERTY(QString projectName READ projectName WRITE setProjectName)

public:
    ProjectConfig(const ProjectConfig&) = delete;
    ProjectConfig& operator=(const ProjectConfig&) = delete;

    static ProjectConfig& instance() {
        static ProjectConfig instance;
        return instance;
    }

    QString projectName() const { return m_projectName; }
    void setProjectName(const QString& name) { m_projectName = name; }
    QString prefix() const { return m_prefix; }

    void loadConfig(const QString& configFilePath);   // 从文件加载二进制
    void saveConfig(const QString& configFilePath) const; // 保存为二进制

private:
    ProjectConfig(QObject* parent = nullptr) : QObject(parent) {
        // 注意：不会传入有效 parent，安全
        qDebug() << "ProjectConfig singleton created.";
    }
    ~ProjectConfig() {
        qDebug() << "ProjectConfig singleton destroyed.";
    }

    void serialize(QDataStream& out) const;
    void deserialize(QDataStream& in);

private:
    QString m_projectName;
    QString m_prefix = "vfProj";
};