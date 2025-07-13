#include "configmanager.h"
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QMutexLocker>
#include <QDebug>

ConfigManager& ConfigManager::instance()
{
    static ConfigManager instance;
    return instance;
}

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
{
    // 设置配置文件路径
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (configDir.isEmpty()) {
        configDir = QCoreApplication::applicationDirPath() + "/config";
    }
    
    m_configFilePath = QDir(configDir).filePath("running_tray_config.json");
    
    ensureConfigDirectory();
    loadFromFile();
}

ConfigManager::~ConfigManager()
{
    saveToFile();
}

void ConfigManager::ensureConfigDirectory()
{
    QFileInfo fileInfo(m_configFilePath);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

void ConfigManager::initializeDefaultConfig()
{
    if (m_configData.isEmpty()) {
        m_configData = QJsonObject {
            {"app", QJsonObject{
                {"version", "1.0"},
                {"autoStart", false},
                {"minimizeToTray", true},
                {"theme", "auto"}
            }},
            {"plugins", QJsonObject{}}
        };
    }
}

void ConfigManager::savePluginConfig(const QString& pluginName, const QJsonObject& config)
{
    QMutexLocker locker(&m_mutex);
    
    QJsonObject plugins = m_configData["plugins"].toObject();
    plugins[pluginName] = config;
    m_configData["plugins"] = plugins;
    
    saveToFile();
    
    qDebug() << "Saved config for plugin:" << pluginName;
}

QJsonObject ConfigManager::loadPluginConfig(const QString& pluginName)
{
    QMutexLocker locker(&m_mutex);
    
    QJsonObject plugins = m_configData["plugins"].toObject();
    return plugins[pluginName].toObject();
}

void ConfigManager::saveAppConfig(const QJsonObject& config)
{
    QMutexLocker locker(&m_mutex);
    
    m_configData["app"] = config;
    saveToFile();
    
    qDebug() << "Saved app config";
}

QJsonObject ConfigManager::loadAppConfig()
{
    QMutexLocker locker(&m_mutex);
    
    return m_configData["app"].toObject();
}

void ConfigManager::setValue(const QString& pluginName, const QString& key, const QVariant& value)
{
    QMutexLocker locker(&m_mutex);
    
    QJsonObject plugins = m_configData["plugins"].toObject();
    QJsonObject pluginConfig = plugins[pluginName].toObject();
    
    pluginConfig[key] = QJsonValue::fromVariant(value);
    plugins[pluginName] = pluginConfig;
    m_configData["plugins"] = plugins;
    
    saveToFile();
    
    emit configChanged(pluginName, key, value);
}

QVariant ConfigManager::getValue(const QString& pluginName, const QString& key, const QVariant& defaultValue)
{
    QMutexLocker locker(&m_mutex);
    
    QJsonObject plugins = m_configData["plugins"].toObject();
    QJsonObject pluginConfig = plugins[pluginName].toObject();
    
    if (pluginConfig.contains(key)) {
        return pluginConfig[key].toVariant();
    }
    
    return defaultValue;
}

bool ConfigManager::saveToFile()
{
    QFile file(m_configFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open config file for writing:" << m_configFilePath;
        return false;
    }
    
    QJsonDocument doc(m_configData);
    file.write(doc.toJson());
    file.close();
    
    return true;
}

bool ConfigManager::loadFromFile()
{
    QFile file(m_configFilePath);
    if (!file.exists()) {
        qDebug() << "Config file does not exist, creating default:" << m_configFilePath;
        initializeDefaultConfig();
        saveToFile();
        return true;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open config file for reading:" << m_configFilePath;
        initializeDefaultConfig();
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse config file:" << error.errorString();
        initializeDefaultConfig();
        return false;
    }
    
    m_configData = doc.object();
    
    // 确保基本结构存在
    if (!m_configData.contains("app")) {
        m_configData["app"] = QJsonObject{};
    }
    if (!m_configData.contains("plugins")) {
        m_configData["plugins"] = QJsonObject{};
    }
    
    qDebug() << "Loaded config from:" << m_configFilePath;
    return true;
}

QString ConfigManager::getConfigFilePath() const
{
    return m_configFilePath;
}