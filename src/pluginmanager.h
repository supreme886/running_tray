#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include <QPluginLoader>
#include "interface/itrayloadplugin.h"

class PluginManager {
public:
    struct PluginEntry {
        QString path;
        QString name;
        ITrayLoadPlugin *plugin;
        QPluginLoader *loader;
    };

    QList<PluginEntry> loadPlugins(const QString &path);

    QList<PluginEntry> getLoadedPlugins();
private:
    QList<PluginEntry> _loaded_plugins;
};


#endif // PLUGINMANAGER_H
