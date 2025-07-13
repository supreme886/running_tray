#pragma once

#include <QObject>
#include <QtPlugin>
#include <QSettings>
#include <QTimer>

// 平台特定头文件
#ifdef Q_OS_WIN
#include <windows.h>
#elif defined(Q_OS_MACOS)
#include <mach/mach.h>
#include <mach/processor_info.h>
#include <mach/mach_host.h>
#endif

#include "interface/itrayloadplugin.h"

class QIcon;
class QSystemTrayIcon;
class QMenu;
class QAction;
class ThemeManager;  // 使用新的主题管理器

class RunningCatPlugin : public ITrayLoadPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ITrayLoadPlugin_iid FILE "runningcatplugin.json")
    Q_INTERFACES(ITrayLoadPlugin)

public:
    QString name() const override { return "Running Cat Plugin"; }
    QSystemTrayIcon* init() override;
    void stop() override;
    void setStatusCallback(std::function<void(int)> callback) override;
    bool hasSettings() override;
    QWidget* createSettingsWidget() override;

    QIcon updateIcon();
    
    // 图标尺寸配置接口
    void setIconSize(int size);
    int getIconSize() const;
    void setAutoScaleIcon(bool enabled);
    bool isAutoScaleEnabled() const;

private slots:
    void updateCPUUsage();
    void onIconUpdateTimeout();
    void onThemeChanged();  // 主题变化槽函数
    
private:
    void initializeIconPaths();
    void updateIconPathsForTheme();
    QIcon createScaledIcon(const QString& iconPath);
    
    QStringList iconPaths;
    QStringList darkIconPaths;
    QStringList lightIconPaths;
    int currentIndex = 0;
    int currentRefreshInterval = 1000;
    bool currentIsDarkTheme = false;
    
    // 图标尺寸相关成员变量
    int iconSize = 16;
    bool autoScaleIcon = true;
    
    // 主题管理器
    ThemeManager* m_themeManager = nullptr;
    
    // 平台特有成员变量（移除主题监控相关）
#ifdef Q_OS_WIN
    ULARGE_INTEGER lastIdleTime;
    ULARGE_INTEGER lastKernelTime;
    ULARGE_INTEGER lastUserTime;
    ULARGE_INTEGER fileTimeToULargeInteger(const FILETIME& ft);
#elif defined(Q_OS_MACOS)
    processor_info_array_t lastCpuInfo;
    mach_msg_type_number_t lastNumCpuInfo;
    natural_t lastNumCpus;
#endif
    
    QTimer* cpuTimer = nullptr;
    QTimer* iconUpdateTimer = nullptr;
    QSystemTrayIcon* trayIcon = nullptr;
    QMenu* trayMenu = nullptr;
};
