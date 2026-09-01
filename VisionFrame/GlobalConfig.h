#pragma once

#include <QObject>
#include "ProjectConfig.h"   // 包含所有需要注册的单例头文件

class GlobalConfig : public QObject
{
    Q_OBJECT

public:
    // 禁止拷贝
    GlobalConfig(const GlobalConfig&) = delete;
    GlobalConfig& operator=(const GlobalConfig&) = delete;

    // 单例访问
    static GlobalConfig& instance() {
        static GlobalConfig instance;
        return instance;
    }

    // 注册的单例访问接口（只读，因为单例本身允许修改）
    ProjectConfig& projectConfig() const { return m_projectConfig; }

    // 未来可添加更多：OtherConfig& otherConfig() const;

private:
    // 私有构造和析构
    GlobalConfig(QObject* parent = nullptr) : QObject(parent) {
        qDebug() << "GlobalConfig singleton created.";
    }
    ~GlobalConfig() {
        qDebug() << "GlobalConfig destroyed.";
    }

    // 持有所有注册的单例引用（仅做访问代理，不负责生命周期）
    ProjectConfig& m_projectConfig = ProjectConfig::instance();
};