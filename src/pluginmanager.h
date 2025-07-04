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
        bool is_loaded{false};  // 跟踪插件加载状态
    };

    // Get globally unique instance
    static PluginManager &instance();

    // Load plugins
    QList<PluginEntry> loadPlugins(const QString &dirPath);

    // 新增：启动和停止插件
    void startPlugin(PluginEntry &entry);
    void stopPlugin(PluginEntry &entry);

    // Optional: Get loaded plugins
    const QList<PluginEntry> &plugins() const { return m_plugins; }

private:
    // 禁止外部构造
    PluginManager(QObject *parent = nullptr);
    ~PluginManager();

    PluginManager(const PluginManager &) = delete;
    PluginManager &operator=(const PluginManager &) = delete;

    QList<PluginEntry> m_plugins;
};
