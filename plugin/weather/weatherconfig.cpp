#include "weatherconfig.h"
#include <QFile>
#include <QCoreApplication>
#include <QDir>
#include <QJsonDocument>
#include <QDebug>

WeatherConfig::WeatherConfig(QObject *parent) : QObject(parent)
{
    m_configPath = QDir(QCoreApplication::applicationDirPath()).filePath("weather_config.json");
    load(); // 尝试加载配置文件
}

QString WeatherConfig::apiKey() const
{
    return m_configData["api_key"].toString();
}

void WeatherConfig::setApiKey(const QString &newApiKey)
{
    m_configData["api_key"] = newApiKey;
}

bool WeatherConfig::load()
{
    QFile file(m_configPath);
    if (!file.exists()) {
        qDebug() << "Config file not found, creating default: " << m_configPath;
        return save(); // 创建默认配置文件
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open config file: " << file.errorString();
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Config file parse error: " << parseError.errorString();
        return false;
    }

    if (!doc.isObject()) {
        qWarning() << "Invalid config file format";
        return false;
    }

    m_configData = doc.object();
    return true;
}

bool WeatherConfig::save()
{
    QFile file(m_configPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open config file for writing: " << file.errorString();
        return false;
    }

    QJsonDocument doc(m_configData);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}