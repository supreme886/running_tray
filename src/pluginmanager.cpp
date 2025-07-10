#include "pluginmanager.h"
#include <QDir>
#include <QDebug>

PluginManager::PluginManager(QObject *parent)
    : QObject(parent) {}

PluginManager::~PluginManager() {
    // 清理 loader
    for (auto &entry : m_plugins) {
        stopPlugin(entry);  // 停止所有插件
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
        
        // 检查插件是否可以加载
        if (!loader->load()) {
            qWarning() << "Failed to load plugin:" << loader->errorString();
            delete loader;
            continue;
        }
        
        // 打印元数据信息
        qDebug() << "Plugin metaData:" << loader->metaData();
        
        QObject *instance = loader->instance();
        if (!instance) {
            qWarning() << "Failed to create instance:" << loader->errorString();
            delete loader;
            continue;
        }
        
        qDebug() << "Loaded plugin class name:" << instance->metaObject()->className();
        auto *plugin = qobject_cast<ITrayLoadPlugin *>(instance);
        if (plugin) {
            PluginEntry entry{ fullPath, plugin->name(), plugin, loader, true };  // 初始化为未加载
            m_plugins.append(entry);
            if (m_plugins.size() == 1) {
                m_plugins[0].is_loaded = true;
            }
        } else {
            delete loader;
        }
    }

    return m_plugins;
}

void PluginManager::startPlugin(PluginEntry &entry) {
    qDebug() << Q_FUNC_INFO <<__LINE__;
    if (!entry.is_loaded && entry.plugin) {
        qDebug() << Q_FUNC_INFO <<__LINE__;
        entry.plugin->init();
        entry.is_loaded = true;
        qDebug() << "Plugin started:" << entry.name;
    }
}

void PluginManager::stopPlugin(PluginEntry &entry) {
    if (entry.plugin) {
        if (entry.is_loaded) {
            entry.plugin->stop();
            entry.is_loaded = false;
        }
    } else {
        qDebug() << "Attempted to stop invalid plugin entry";
    }
}
