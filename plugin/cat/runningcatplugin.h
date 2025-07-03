#pragma once

#include <QObject>
#include <QSystemTrayIcon>
#include <QStringList>
#include "interface/itrayloadplugin.h"

class RunningCatPlugin : public QObject, public ITrayLoadPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ITrayLoadPlugin_iid)
    Q_INTERFACES(ITrayLoadPlugin)

public:
    QString name() const override { return "Running Cat Plugin"; }
    void init() override;
    QIcon updateIcon() override;

private:
    QStringList iconPaths;
    int currentIndex = 0;
};
