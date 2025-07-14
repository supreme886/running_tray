#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QString>
#include <QVariant>
#include <QMutex>
#include "common_exports.h"

class COMMON_LIBRARY_EXPORT ConfigManager : public QObject
{
    Q_OBJECT

public:
    static ConfigManager& instance();
    
    // 插件配置管理
    void savePluginConfig(const QString& pluginName, const QJsonObject& config);
    QJsonObject loadPluginConfig(const QString& pluginName);
    
    // 应用全局配置管理
    void saveAppConfig(const QJsonObject& config);
    QJsonObject loadAppConfig();
    
    // 通用配置操作
    void setValue(const QString& pluginName, const QString& key, const QVariant& value);
    QVariant getValue(const QString& pluginName, const QString& key, const QVariant& defaultValue = QVariant());

    void setPluginConfig(const QString& pluginName, const QVariantMap& config);
    QVariantMap getPluginConfig(const QString& pluginName);
    
    // 配置文件操作
    bool saveToFile();
    bool loadFromFile();
    
    // 获取配置文件路径
    QString getConfigFilePath() const;
    
signals:
    void configChanged(const QString& pluginName, const QString& key, const QVariant& value);
    
private:
    explicit ConfigManager(QObject *parent = nullptr);
    ~ConfigManager();
    
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    QString m_configFilePath;
    QJsonObject m_configData;
    QMutex m_mutex;
    
    void ensureConfigDirectory();
    void initializeDefaultConfig();
};

#endif // CONFIGMANAGER_H
