#pragma once

#include <QObject>
#include <QtPlugin>
#include <QSettings>
#include <QTimer>  // 添加 QTimer 包含

// 平台特定头文件
#ifdef Q_OS_WIN
#include <windows.h>
#include <QAbstractNativeEventFilter>
#elif defined(Q_OS_MACOS)
#include <mach/mach.h>
#include <mach/processor_info.h>
#include <mach/mach_host.h>
#elif defined(Q_OS_LINUX)
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusInterface>
#endif

#include "interface/itrayloadplugin.h"

class QIcon;
class QSystemTrayIcon;
class QMenu;
class QAction;
class RunningCatPlugin;  // 添加前向声明

#ifdef Q_OS_WIN
class ThemeEventFilter : public QAbstractNativeEventFilter {
public:
    ThemeEventFilter(RunningCatPlugin* plugin) : m_plugin(plugin) {}
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
private:
    RunningCatPlugin* m_plugin;
};
#endif

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
    void onThemeChanged();  // 公开给事件过滤器调用
    
    // 新增：图标尺寸配置接口
    void setIconSize(int size);
    int getIconSize() const;
    void setAutoScaleIcon(bool enabled);
    bool isAutoScaleEnabled() const;

private slots:
    void updateCPUUsage();
    void onIconUpdateTimeout();
#ifdef Q_OS_LINUX
    void onDBusThemeChanged();
#endif
    
private:
    void initializeIconPaths();
    void setupThemeMonitoring();
    void cleanupThemeMonitoring();
    bool isDarkTheme() const;
    void updateIconPathsForTheme();
    QIcon createScaledIcon(const QString& iconPath);  // 新增：创建缩放图标的方法
    
    QStringList iconPaths;
    QStringList darkIconPaths;
    QStringList lightIconPaths;
    int currentIndex = 0;
    int currentRefreshInterval = 1000;
    bool currentIsDarkTheme = false;
    
    // 新增：图标尺寸相关成员变量
    int iconSize = 16;  // 默认图标尺寸
    bool autoScaleIcon = true;  // 是否自动根据DPI缩放
    
    // 平台特有成员变量
#ifdef Q_OS_WIN
    ULARGE_INTEGER lastIdleTime;
    ULARGE_INTEGER lastKernelTime;
    ULARGE_INTEGER lastUserTime;
    ULARGE_INTEGER fileTimeToULargeInteger(const FILETIME& ft);
    ThemeEventFilter* m_themeFilter = nullptr;
#elif defined(Q_OS_MACOS)
    // macOS平台特有成员变量
    processor_info_array_t lastCpuInfo;
    mach_msg_type_number_t lastNumCpuInfo;
    natural_t lastNumCpus;
    void* m_themeObserver = nullptr;  // NSObject observer
#elif defined(Q_OS_LINUX)
    QDBusInterface* m_settingsInterface = nullptr;
#endif
    
    QTimer* cpuTimer = nullptr;
    QTimer* iconUpdateTimer = nullptr;
    QSystemTrayIcon* trayIcon = nullptr;
    QMenu* trayMenu = nullptr;
    // 注意：不再需要 themeCheckTimer，因为我们使用原生API监听
    
    // 添加菜单项引用
    QAction* size16Action;
    QAction* size24Action;
    QAction* size32Action;
    QAction* autoScaleAction;
};
