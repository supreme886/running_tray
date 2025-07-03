#include "pluginmanager.h"
#include <QDir>
#include <QDebug>

PluginManager::PluginManager(QObject *parent)
    : QObject(parent) {}

PluginManager::~PluginManager() {
    // 清理 loader
    for (auto &entry : m_plugins) {
        delete entry.loader;
    }
}

PluginManager &PluginManager::instance() {
    static PluginManager instance;
    return instance;
}

QList<PluginManager::PluginEntry> PluginManager::loadPlugins(const QString &dirPath) {
    QDir dir(dirPath);
    if (!dir.exists()) {
        qWarning() << "Plugin directory does not exist:" << dirPath;
        return {};
    }

    m_plugins.clear();

    for (const QString &file : dir.entryList(QDir::Files)) {
        auto fullPath = dir.absoluteFilePath(file);
        QPluginLoader *loader = new QPluginLoader(fullPath);
        QObject *instance = loader->instance();
        if (!instance) {
            qWarning() << "Failed to load plugin:" << file << loader->errorString();
            delete loader;
            continue;
        }

        auto *plugin = qobject_cast<ITrayLoadPlugin *>(instance);
        if (plugin) {
            PluginEntry entry{ fullPath, plugin->name(), plugin, loader };
            m_plugins.append(entry);
        } else {
            delete loader;
        }
    }

    return m_plugins;
}
