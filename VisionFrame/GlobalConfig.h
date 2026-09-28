#pragma once

#include <QObject>
#include "ProjectConfig.h"   // 需要注册的头文件

class GlobalConfig : public QObject
{
    Q_OBJECT

public:
    // 禁止拷贝
    GlobalConfig(const GlobalConfig&) = delete;
    GlobalConfig& operator=(const GlobalConfig&) = delete;

    // 单例模式
    static GlobalConfig& instance() {
        static GlobalConfig instance;
        return instance;
    }

    // 注意：全局接口，只允许读取，不允许直接修改
    ProjectConfig& projectConfig() const { return m_projectConfig; }

    // 未来可添加更多：OtherConfig& otherConfig() const;

private:
    // 私有构造函数
    GlobalConfig(QObject* parent = nullptr) : QObject(parent) {
        qDebug() << "GlobalConfig singleton created.";
    }
    ~GlobalConfig() {
        qDebug() << "GlobalConfig destroyed.";
    }

    // 全局配置注册的全局引用，避免使用时访问时出现问题
    ProjectConfig& m_projectConfig = ProjectConfig::instance();
};
