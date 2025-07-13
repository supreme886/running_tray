#pragma once

#include <QObject>
#include <QSettings>

// 平台特定头文件
#ifdef Q_OS_WIN
#include <windows.h>
#include <QAbstractNativeEventFilter>
#elif defined(Q_OS_LINUX)
#include <QDBusInterface>
#endif

#include "common_exports.h"

class QTimer;

#ifdef Q_OS_WIN
class ThemeEventFilter : public QAbstractNativeEventFilter {
public:
    ThemeEventFilter(class ThemeManager* manager) : m_manager(manager) {}
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
private:
    class ThemeManager* m_manager;
};
#endif

class COMMON_LIBRARY_EXPORT ThemeManager : public QObject {
    Q_OBJECT

public:
    explicit ThemeManager(QObject *parent = nullptr);
    ~ThemeManager();
    
    // 启动和停止主题监控
    void startMonitoring();
    void stopMonitoring();
    
    // 检查当前是否为暗色主题
    bool isDarkTheme() const;
    
    // 手动触发主题检查
    void checkThemeChange();
    
signals:
    // 主题变化信号
    void themeChanged(bool isDarkTheme);
    
private slots:
    void onThemeCheckTimeout();
#ifdef Q_OS_LINUX
    void onDBusThemeChanged();
#endif
    
private:
    void setupPlatformMonitoring();
    void cleanupPlatformMonitoring();
    
    bool m_isMonitoring = false;
    bool m_currentIsDarkTheme = false;
    
    // 平台特定成员
#ifdef Q_OS_WIN
    ThemeEventFilter* m_themeFilter = nullptr;
#elif defined(Q_OS_MACOS)
    QTimer* m_themeTimer = nullptr;
#elif defined(Q_OS_LINUX)
    QDBusInterface* m_settingsInterface = nullptr;
#endif
};
