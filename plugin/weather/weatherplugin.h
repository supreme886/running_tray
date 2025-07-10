#pragma once

#include <QObject>
#include <QtPlugin>
#include <QSettings>
#include <QMap>
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
#include "rlottie.h"

class QIcon;
class QSystemTrayIcon;
class QMenu;
class QAction;
class WeatherPlugin;

using namespace rlottie;

#ifdef Q_OS_WIN
class ThemeEventFilter : public QAbstractNativeEventFilter {
public:
    ThemeEventFilter(WeatherPlugin* plugin) : m_plugin(plugin) {}
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
private:
    WeatherPlugin* m_plugin;
};
#endif

// 在类声明中添加成员变量
class WeatherPlugin : public ITrayLoadPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ITrayLoadPlugin_iid FILE "weatherplugin.json")
    Q_INTERFACES(ITrayLoadPlugin)
public:
    QString name() const override { return "Weather Plugin"; }
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
#ifdef Q_OS_LINUX
    void onDBusThemeChanged();
#endif
    
private:
    void reloadAnimation();
    void setupThemeMonitoring();
    void cleanupThemeMonitoring();
    bool isDarkTheme() const;
    void updateIconPathsForTheme();
    
    QMap<QString, QString> iconPaths;
    bool currentIsDarkTheme = false;
    
    int m_iconCurIndex{0};
    int iconSize = 16;
    bool autoScaleIcon = true;
    
    // 平台特有成员变量
#ifdef Q_OS_WIN
    ThemeEventFilter* m_themeFilter = nullptr;
#elif defined(Q_OS_MACOS)
    void* m_themeObserver = nullptr;  // NSObject observer
#elif defined(Q_OS_LINUX)
    QDBusInterface* m_settingsInterface = nullptr;
#endif

    QTimer* iconUpdateTimer = nullptr;
    QSystemTrayIcon* trayIcon = nullptr;
    QMenu* trayMenu = nullptr;

    std::unique_ptr<rlottie::Animation> m_animation;
};
