#pragma once

#include <QObject>

// 仅在Windows平台包含Windows.h
#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "interface/itrayloadplugin.h"

class QIcon;
class QSystemTrayIcon;
class QMenu;

class RunningCatPlugin : public ITrayLoadPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ITrayLoadPlugin_iid)
    Q_INTERFACES(ITrayLoadPlugin)

public:
    QString name() const override { return "Running Cat Plugin"; }
    QSystemTrayIcon* init() override;
    void stop() override;
    void setStatusCallback(std::function<void(int)> callback) override;

    QIcon updateIcon();

private slots:
    void updateCPUUsage();
    void onIconUpdateTimeout();
    
private:
    QStringList iconPaths;
    int currentIndex = 0;
    int currentRefreshInterval = 1000;
    
    // Windows平台特有成员变量
#ifdef Q_OS_WIN
    ULARGE_INTEGER lastIdleTime;
    ULARGE_INTEGER lastKernelTime;
    ULARGE_INTEGER lastUserTime;
#endif
    
    QTimer* cpuTimer = nullptr;
    QTimer* iconUpdateTimer = nullptr;
    QSystemTrayIcon* trayIcon = nullptr;
    QMenu* trayMenu = nullptr;
    
#ifdef Q_OS_WIN
    ULARGE_INTEGER fileTimeToULargeInteger(const FILETIME& ft);
#endif
};
