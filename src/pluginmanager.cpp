#include "pluginmanager.h"

#include <QDir>

QList<PluginManager::PluginEntry> PluginManager::loadPlugins(const QString &dirPath) {
    _loaded_plugins.clear();
    QDir dir(dirPath);
    for (const QString &file : dir.entryList(QDir::Files)) {
        QPluginLoader *loader = new QPluginLoader(dir.absoluteFilePath(file));
        QObject *instance = loader->instance();
        if (!instance) continue;

        auto *plugin = qobject_cast<ITrayLoadPlugin *>(instance);
        if (plugin) {
            _loaded_plugins.append({ file, plugin->name(), plugin, loader });
        }
    }
    return _loaded_plugins;
}


QList<PluginManager::PluginEntry> PluginManager::getLoadedPlugins()
{
    return _loaded_plugins;
}
