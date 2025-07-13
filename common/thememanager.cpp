#include "thememanager.h"
#include <QApplication>
#include <QTimer>
#include <QDebug>

#ifdef Q_OS_LINUX
#include <QDBusConnection>
#include <QDBusReply>
#endif

// Windows 事件过滤器实现
#ifdef Q_OS_WIN
bool ThemeEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result) {
    if (eventType == "windows_generic_MSG") {
        MSG* msg = static_cast<MSG*>(message);
        if (msg->message == WM_SETTINGCHANGE) {
            QString setting = QString::fromWCharArray(reinterpret_cast<const wchar_t*>(msg->lParam));
            if (setting == "ImmersiveColorSet" || 
                setting.contains("Theme") || 
                setting == "ColorPrevalence" ||
                setting == "SystemUsesLightTheme") {
                // 延迟检查，确保系统主题切换完成
                QTimer::singleShot(200, m_manager, &ThemeManager::checkThemeChange);
            }
        }
    }
    return false;
}
#endif

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
{
    // 初始化当前主题状态
    m_currentIsDarkTheme = isDarkTheme();
}

ThemeManager::~ThemeManager()
{
    stopMonitoring();
}

void ThemeManager::startMonitoring()
{
    if (m_isMonitoring) {
        return;
    }
    
    setupPlatformMonitoring();
    m_isMonitoring = true;
    
    qDebug() << "ThemeManager: Started theme monitoring";
}

void ThemeManager::stopMonitoring()
{
    if (!m_isMonitoring) {
        return;
    }
    
    cleanupPlatformMonitoring();
    m_isMonitoring = false;
    
    qDebug() << "ThemeManager: Stopped theme monitoring";
}

bool ThemeManager::isDarkTheme() const
{
#ifdef Q_OS_WIN
    // Windows 主题检测
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
    return settings.value("AppsUseLightTheme", 1).toInt() == 0;
    
#elif defined(Q_OS_MACOS)
    // macOS: 使用 QSettings 读取系统偏好设置
    QSettings settings(QSettings::UserScope, "Apple", "Global Preferences");
    QString interfaceStyle = settings.value("AppleInterfaceStyle", "").toString();
    return interfaceStyle.toLower() == "dark";
    
#else
    // Linux 主题检测
    if (m_settingsInterface && m_settingsInterface->isValid()) {
        QDBusReply<QVariant> reply = m_settingsInterface->call("Read", "org.gnome.desktop.interface", "gtk-theme");
        if (reply.isValid()) {
            QString theme = reply.value().toString();
            return theme.contains("dark", Qt::CaseInsensitive);
        }
    }
    
    // 备用检测方法
    QString gtkTheme = qgetenv("GTK_THEME");
    QString desktopSession = qgetenv("DESKTOP_SESSION");
    QString xdgCurrentDesktop = qgetenv("XDG_CURRENT_DESKTOP");
    
    return gtkTheme.contains("dark", Qt::CaseInsensitive) || 
           desktopSession.contains("dark", Qt::CaseInsensitive) ||
           xdgCurrentDesktop.contains("dark", Qt::CaseInsensitive);
#endif
}

void ThemeManager::checkThemeChange()
{
    bool isDark = isDarkTheme();
    
    // 如果主题发生变化
    if (isDark != m_currentIsDarkTheme) {
        qDebug() << "ThemeManager: Theme changed from" << (m_currentIsDarkTheme ? "dark" : "light") 
                 << "to" << (isDark ? "dark" : "light");
        
        m_currentIsDarkTheme = isDark;
        emit themeChanged(isDark);
    }
}

void ThemeManager::onThemeCheckTimeout()
{
    checkThemeChange();
}

#ifdef Q_OS_LINUX
void ThemeManager::onDBusThemeChanged()
{
    // 延迟检查，避免频繁触发
    QTimer::singleShot(100, this, &ThemeManager::checkThemeChange);
}
#endif

void ThemeManager::setupPlatformMonitoring()
{
#ifdef Q_OS_WIN
    // Windows: 监听 WM_SETTINGCHANGE 消息
    m_themeFilter = new ThemeEventFilter(this);
    QApplication::instance()->installNativeEventFilter(m_themeFilter);
    qDebug() << "ThemeManager: Windows theme monitoring enabled";
    
#elif defined(Q_OS_MACOS)
    // macOS: 使用定时器检查（更兼容的方式）
    m_themeTimer = new QTimer(this);
    connect(m_themeTimer, &QTimer::timeout, this, &ThemeManager::onThemeCheckTimeout);
    m_themeTimer->start(2000);  // 每2秒检查一次
    qDebug() << "ThemeManager: macOS theme monitoring enabled (polling)";
    
#elif defined(Q_OS_LINUX)
    // Linux: 监听 GNOME/KDE 设置变化
    m_settingsInterface = new QDBusInterface(
        "org.freedesktop.portal.Desktop",
        "/org/freedesktop/portal/desktop",
        "org.freedesktop.portal.Settings",
        QDBusConnection::sessionBus(),
        this
    );
    
    if (m_settingsInterface->isValid()) {
        connect(m_settingsInterface, SIGNAL(SettingChanged(QString,QString,QVariant)),
                this, SLOT(onDBusThemeChanged()));
        qDebug() << "ThemeManager: Linux theme monitoring enabled (Portal)";
    } else {
        qDebug() << "ThemeManager: Failed to connect to Linux theme portal";
    }
#endif
}

void ThemeManager::cleanupPlatformMonitoring()
{
#ifdef Q_OS_WIN
    if (m_themeFilter) {
        QApplication::instance()->removeNativeEventFilter(m_themeFilter);
        delete m_themeFilter;
        m_themeFilter = nullptr;
    }
    
#elif defined(Q_OS_MACOS)
    if (m_themeTimer) {
        m_themeTimer->stop();
        delete m_themeTimer;
        m_themeTimer = nullptr;
    }
    
#elif defined(Q_OS_LINUX)
    if (m_settingsInterface) {
        delete m_settingsInterface;
        m_settingsInterface = nullptr;
    }
#endif
}

#include "thememanager.moc"