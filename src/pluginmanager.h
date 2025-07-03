#pragma once

#include <QObject>
#include <QList>
#include <QString>
#include <QPluginLoader>
#include "interface/itrayloadplugin.h"

class PluginManager : public QObject {
    Q_OBJECT

public:
    struct PluginEntry {
        QString path;
        QString name;
        ITrayLoadPlugin *plugin;
        QPluginLoader *loader;
        bool is_loaded{false};
    };

    // 🔹 获取全局唯一实例
    static PluginManager &instance();

    // 🔹 加载插件
    QList<PluginEntry> loadPlugins(const QString &dirPath);

    // 可选：获取已加载插件
    const QList<PluginEntry> &plugins() const { return m_plugins; }

private:
    // 禁止外部构造
    PluginManager(QObject *parent = nullptr);
    ~PluginManager();

    PluginManager(const PluginManager &) = delete;
    PluginManager &operator=(const PluginManager &) = delete;

    QList<PluginEntry> m_plugins;
};
