#pragma once

#include <QObject>
#include <QtPlugin>
#include <QSettings>
#include <QMap>
#include <QTimer>
#include <QMutex>

// 平台特定头文件（移除主题监控相关）
#ifdef Q_OS_WIN
#include <windows.h>
#elif defined(Q_OS_MACOS)
#include <mach/mach.h>
#include <mach/processor_info.h>
#include <mach/mach_host.h>
#endif

#include "interface/itrayloadplugin.h"
#include "rlottie.h"
#include "weatherconfig.h"

class QIcon;
class QSystemTrayIcon;
class QMenu;
class QAction;
class QNetworkReply;
class QNetworkAccessManager;
class ThemeManager;  // 使用新的主题管理器

using namespace rlottie;

// 移除 ThemeEventFilter 类声明

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

    void loadWeatherConfig();
    QString getAnimationFileForWeather(const QString&);

    QIcon updateIcon();
    
    // 图标尺寸配置接口
    void setIconSize(int size);
    int getIconSize() const;
    void setAutoScaleIcon(bool enabled);
    bool isAutoScaleEnabled() const;

    void fetchLocationByIP(const QString& ip);
    void fetchWeatherData(const QString& cityCode);
    void updateWeatherAnimation(const QString& weatherCode);

private slots:
    void fetchPublicIP();
    void onThemeChanged(bool isDarkTheme);  // 主题变化槽函数
    
private:
    void reloadAnimation(const QString& animationFileName = QString());
    void updateIconPathsForTheme();
    
    QMap<QString, QString> iconPaths;
    bool currentIsDarkTheme = false;
    
    int m_iconCurIndex{0};
    int iconSize = 16;
    bool autoScaleIcon = true;
    
    // 主题管理器
    ThemeManager* m_themeManager = nullptr;
    
    // 移除平台特定的主题监控成员变量

    QTimer* iconUpdateTimer = nullptr;
    QSystemTrayIcon* trayIcon = nullptr;
    QMenu* trayMenu = nullptr;

    std::unique_ptr<rlottie::Animation> m_animation;

    QTimer* weatherUpdateTimer;
    QString currentCityCode;
    QString weatherApiKey;
    QString publicIp;
    WeatherConfig *m_config = nullptr;
    QJsonObject weatherConfig;
    mutable QMutex m_animationMutex;
};