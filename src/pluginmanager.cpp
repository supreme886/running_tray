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
            PluginEntry entry{ fullPath, plugin->name(), plugin, loader, false };  // 初始化为未加载
            m_plugins.append(entry);
        } else {
            delete loader;
        }
    }

    return m_plugins;
}

// 新增：启动插件
void PluginManager::startPlugin(PluginEntry &entry) {
    qDebug() << Q_FUNC_INFO <<__LINE__;
    if (!entry.is_loaded && entry.plugin) {
        qDebug() << Q_FUNC_INFO <<__LINE__;
        entry.plugin->init();
        entry.is_loaded = true;
        qDebug() << "Plugin started:" << entry.name;
    }
}

// 新增：停止插件
void PluginManager::stopPlugin(PluginEntry &entry) {
    qDebug() << Q_FUNC_INFO << entry.is_loaded  << __LINE__;
    if (entry.is_loaded && entry.plugin) {
        qDebug() << Q_FUNC_INFO << entry.plugin->metaObject() <<__LINE__;
        entry.plugin->stop();
        qDebug() << Q_FUNC_INFO <<__LINE__;
        entry.is_loaded = false;
        qDebug() << "Plugin stopped:" << entry.name;
    }
}
