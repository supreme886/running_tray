#pragma once

#include <QObject>
#include <QSystemTrayIcon>
#include <QStringList>

#include <windows.h>  // 添加 Windows API 头文件
#include "interface/itrayloadplugin.h"

class RunningCatPlugin : public QObject, public ITrayLoadPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ITrayLoadPlugin_iid)
    Q_INTERFACES(ITrayLoadPlugin)

public:
    QString name() const override { return "Running Cat Plugin"; }
    void init() override;
    QIcon updateIcon() override;

    int refreshInterval() const override { return currentRefreshInterval; }

private slots:
    void updateCPUUsage();

private:
    QStringList iconPaths;
    int currentIndex = 0;
    int currentRefreshInterval = 1000; // 默认1秒刷新间隔
    ULARGE_INTEGER lastIdleTime;
    ULARGE_INTEGER lastKernelTime;
    ULARGE_INTEGER lastUserTime;
    QTimer* cpuTimer = nullptr;

    ULARGE_INTEGER fileTimeToULargeInteger(const FILETIME& ft);
};
