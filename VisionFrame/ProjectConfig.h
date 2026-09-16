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

    void loadConfig(const QString& configFilePath);   // ���ļ����ض�����
    void saveConfig(const QString& configFilePath) const; // ����Ϊ������

private:
    ProjectConfig(QObject* parent = nullptr) : QObject(parent) {
        // ע�⣺���ᴫ����Ч parent����ȫ
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