#include "ProjectConfig.h"
#include <QMetaProperty>
#include <QVariantMap>
#include <QFileInfo>   // 新增头文件

void ProjectConfig::serialize(QDataStream& out) const
{
    QVariantMap map;
    const QMetaObject* meta = metaObject();
    for (int i = meta->propertyOffset(); i < meta->propertyCount(); ++i) {
        QMetaProperty prop = meta->property(i);
        QString name = QString::fromUtf8(prop.name());
        QVariant value = prop.read(this);
        map.insert(name, value);
    }
    out << map;
}

void ProjectConfig::deserialize(QDataStream& in)
{
    QVariantMap map;
    in >> map;
    if (in.status() != QDataStream::Ok) {
        qWarning() << "Failed to deserialize data stream";
        return;
    }

    const QMetaObject* meta = metaObject();
    for (int i = meta->propertyOffset(); i < meta->propertyCount(); ++i) {
        QMetaProperty prop = meta->property(i);
        QString name = QString::fromUtf8(prop.name());
        if (map.contains(name)) {
            QVariant value = map.value(name);
            if (value.canConvert(prop.userType())) {
                prop.write(this, value);
            }
            else {
                qWarning() << "Type mismatch for property" << name
                    << "expected" << prop.typeName();
            }
        }
    }
}

// 从二进制文件加载
void ProjectConfig::loadConfig(const QString& configFilePath)
{
    // 检查并补全后缀
    QString filePath = configFilePath;
    QFileInfo info(filePath);
    if (info.suffix().isEmpty()) {
        filePath += m_prefix;   
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open config file for reading:" << filePath;
        return;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_14);

    deserialize(in);

    file.close();
    qDebug() << "Config loaded from" << filePath;
}

// 保存为二进制文件
void ProjectConfig::saveConfig(const QString& configFilePath) const
{
    // 检查并补全后缀
    QString filePath = configFilePath;
    QFileInfo info(filePath);
    if (info.suffix().isEmpty()) {
        filePath += m_prefix;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "Cannot open config file for writing:" << filePath;
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_14);

    serialize(out);

    file.close();
    qDebug() << "Config saved to" << filePath;
}