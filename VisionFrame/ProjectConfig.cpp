#include "ProjectConfig.h"
#include <QMetaProperty>
#include <QVariantMap>
#include <QFileInfo>   // 文件信息头文件
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "NodeEditor/ToolFactory.h"

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

// 从配置文件加载
void ProjectConfig::loadConfig(const QString& configFilePath)
{
    // 检查并安全添加后缀
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

// 保存为配置文件
void ProjectConfig::saveConfig(const QString& configFilePath) const
{
    // 检查并安全添加后缀
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

QString ProjectConfig::normPath(QString p) const
{
    if (QFileInfo(p).suffix().isEmpty())
        p += QString(".%1").arg(m_prefix);
    return p;
}

bool ProjectConfig::saveProject(const QString& filePath) const
{
    QJsonObject root;
    root["version"] = 1;
    root["projectName"] = m_projectName;

    QJsonArray arr;
    for (TaskItem* t : taskItems)
    {
        QJsonObject to;
        to["taskName"] = t->taskName();
        to["graph"] = t->graph().toJson();
        arr.append(to);
    }
    root["tasks"] = arr;

    QFile f(normPath(filePath));
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    QJsonDocument doc(root);
    f.write(doc.toJson(QJsonDocument::Indented));   // 稳定后改为 doc.toBinaryData()
    return true;
}

bool ProjectConfig::loadProject(const QString& filePath)
{
    QFile f(normPath(filePath));
    if (!f.open(QIODevice::ReadOnly))
        return false;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());  // 稳定后改为 fromBinaryData
    if (!doc.isObject())
        return false;
    const QJsonObject root = doc.object();

    for (TaskItem* t : taskItems)
    {
        t->StopTask();
        delete t;
    }
    taskItems.clear();

    m_projectName = root["projectName"].toString();
    const QJsonArray arr = root["tasks"].toArray();
    for (const QJsonValue& v : arr)
    {
        const QJsonObject to = v.toObject();
        auto* t = new TaskItem();
        t->setTaskName(to["taskName"].toString(QStringLiteral("Task")));
        t->graph().fromJson(to["graph"].toObject(),
            [&](const QString& tid) { return ToolFactory::instance().create(tid); });
        taskItems.append(t);
    }
    return true;
}
