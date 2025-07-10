#include "weatherplugin.h"
#include "sharedmenumanager.h"

#include <QIcon>
#include <QDebug>
#include <QTimer>
#include <QMenu>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QStringList>
#include <QAction>
#include <QApplication>
#include <QScreen>
#include <QActionGroup>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QFile>

// 添加平台特定的包含
#ifdef Q_OS_WIN
#include <QApplication>
#elif defined(Q_OS_MACOS)

#elif defined(Q_OS_LINUX)
#include <QDBusConnection>
#include <QDBusInterface>
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
                // 增加延迟时间到200ms，确保系统主题切换完成
                QTimer::singleShot(200, m_plugin, &WeatherPlugin::onThemeChanged);
            }
        }
    }
    return false;
}
#endif

// 在类初始化时创建定时器
QSystemTrayIcon* WeatherPlugin::init() {
    trayIcon = new QSystemTrayIcon(this);
    trayMenu = new QMenu;
    
    // 添加插件特有菜单项
    QAction* aboutAction = new QAction("About Weather", this);
    connect(aboutAction, &QAction::triggered, [this](){
        QMessageBox::information(qApp->activeWindow(), "About", "Weather Plugin v1.0");
        QMessageBox::information(qApp->activeWindow(), "About", "Weathert Plugin v1.0");
    });
    trayMenu->addAction(aboutAction);
    
    trayMenu->addSeparator();
    
    for (QAction* action : SharedMenuManager::instance().getSharedActions()) {
        trayMenu->addAction(action);
    }
    
    trayIcon->setContextMenu(trayMenu);

    setupThemeMonitoring();
    
    iconUpdateTimer = new QTimer(this);
    connect(iconUpdateTimer, &QTimer::timeout, this, [this]() {
        if (m_animation && m_animation->totalFrame() > 0) {
            trayIcon->setIcon(updateIcon());
        }
    });

    reloadAnimation();

    trayIcon->setIcon(updateIcon());
    trayIcon->show();

    return trayIcon;
}

void WeatherPlugin::stop() {
    qDebug() << Q_FUNC_INFO <<__LINE__;

    m_animation.reset();
    // 停止定时器
    if (iconUpdateTimer) {
        iconUpdateTimer->stop();
        delete iconUpdateTimer;
        iconUpdateTimer = nullptr;
        delete iconUpdateTimer;
        iconUpdateTimer = nullptr;
    }
    
    cleanupThemeMonitoring();

    if (trayIcon) {
        trayIcon->hide();
        // 移除手动删除contextMenu的代码，由Qt对象树自动管理
        delete trayIcon;
        trayIcon = nullptr;

        delete trayMenu;
        trayMenu = nullptr; // 确保菜单指针也置空
    }
    
}

void WeatherPlugin::setStatusCallback(std::function<void (int)> callback)
{
    // 实现状态回调功能（如果需要）
}

void WeatherPlugin::setIconSize(int size) {
    if (size > 0 && size <= 128) {
        iconSize = size;
        qDebug() << "Icon size set to:" << iconSize;

        if (trayIcon) {
            trayIcon->setIcon(updateIcon());
        }
    } else {
        qDebug() << "Invalid icon size:" << size << ". Size must be between 1 and 128.";
    }
}

int WeatherPlugin::getIconSize() const {
    return iconSize;
}

void WeatherPlugin::setAutoScaleIcon(bool enabled) {
    autoScaleIcon = enabled;
    qDebug() << "Auto scale icon:" << (enabled ? "enabled" : "disabled");
    
    if (trayIcon) {
        trayIcon->setIcon(updateIcon());
    }
}

bool WeatherPlugin::isAutoScaleEnabled() const {
    return autoScaleIcon;
}

// 添加设置界面支持
bool WeatherPlugin::hasSettings() {
    return true;
}

QWidget* WeatherPlugin::createSettingsWidget() {
    QWidget* settingsWidget = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(settingsWidget);
    
    // 添加图标尺寸设置
    QGroupBox* sizeGroupBox = new QGroupBox("Icon Size");
    QVBoxLayout* sizeLayout = new QVBoxLayout(sizeGroupBox);
    
    QRadioButton* size16Btn = new QRadioButton("16x16 (Small)");
    QRadioButton* size24Btn = new QRadioButton("24x24 (Medium)");
    QRadioButton* size32Btn = new QRadioButton("32x32 (Large)");
    
    // 设置当前选中状态
    if (iconSize == 16) size16Btn->setChecked(true);
    else if (iconSize == 24) size24Btn->setChecked(true);
    else if (iconSize == 32) size32Btn->setChecked(true);
    
    // 连接信号
    connect(size16Btn, &QRadioButton::toggled, [this](bool checked) {
        if (checked) setIconSize(16);
    });
    connect(size24Btn, &QRadioButton::toggled, [this](bool checked) {
        if (checked) setIconSize(24);
    });
    connect(size32Btn, &QRadioButton::toggled, [this](bool checked) {
        if (checked) setIconSize(32);
    });
    
    sizeLayout->addWidget(size16Btn);
    sizeLayout->addWidget(size24Btn);
    sizeLayout->addWidget(size32Btn);
    
    QCheckBox* autoScaleBox = new QCheckBox("Auto Scale (DPI Aware)");
    autoScaleBox->setChecked(autoScaleIcon);
    connect(autoScaleBox, &QCheckBox::toggled, this, &WeatherPlugin::setAutoScaleIcon);
    sizeLayout->addWidget(autoScaleBox);
    
    mainLayout->addWidget(sizeGroupBox);
    mainLayout->addStretch();
    
    return settingsWidget;
}

void WeatherPlugin::reloadAnimation() {
    m_iconCurIndex = 0;
    QFile file(":/res/clear-day.json");
    // QFile file(":/res/extreme-rain.json");
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray jsonData = file.readAll();
        m_animation = rlottie::Animation::loadFromData(jsonData.constData(), "weather_anim");
    }
    iconUpdateTimer->start(10);
}

// 修改updateIcon方法接收帧号参数
QIcon WeatherPlugin::updateIcon() {
    if (!m_animation) {
        reloadAnimation();
        if (!m_animation) {
            qWarning() << "Failed to load animation";
            return QIcon();
        }
    }

    size_t w = 0, h = 0;
    m_animation->size(w, h);
    if (w <= 0 || h <= 0) {
        qWarning() << "Invalid animation size";
        return QIcon();
    }

    const int targetSize = iconSize;
    const QSize renderSize(targetSize, targetSize);

    const int totalFrames = m_animation->totalFrame();
    if (totalFrames <= 0) {
        qWarning() << "Animation has no frames";
        return QIcon();
    }

    m_iconCurIndex = (m_iconCurIndex + 1) % totalFrames;

    QImage image(renderSize, QImage::Format_ARGB32);
    Surface surface {
        reinterpret_cast<uint32_t*>(image.bits()),
        static_cast<size_t>(image.width()),
        static_cast<size_t>(image.height()),
        static_cast<size_t>(image.bytesPerLine())
    };

    m_animation->renderSync(m_iconCurIndex, surface, true);

    QIcon resultIcon(QPixmap::fromImage(image));
    emit iconUpdated(resultIcon);
    return resultIcon;
}

void WeatherPlugin::setupThemeMonitoring() {
#ifdef Q_OS_WIN
    // Windows: 监听 WM_SETTINGCHANGE 消息
    m_themeFilter = new ThemeEventFilter(this);
    QApplication::instance()->installNativeEventFilter(m_themeFilter);
    qDebug() << "Windows theme monitoring enabled";
    
#elif defined(Q_OS_MACOS)
    // macOS: 使用简单的定时器检查（更兼容的方式）
    QTimer* themeTimer = new QTimer(this);
    connect(themeTimer, &QTimer::timeout, this, &WeatherPlugin::onThemeChanged);
    themeTimer->start(2000);  // 每2秒检查一次
    qDebug() << "macOS theme monitoring enabled (polling)";
    
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
        qDebug() << "Linux theme monitoring enabled (Portal)";
    }
#endif
}

void WeatherPlugin::cleanupThemeMonitoring() {
#ifdef Q_OS_WIN
    if (m_themeFilter) {
        QApplication::instance()->removeNativeEventFilter(m_themeFilter);
        delete m_themeFilter;
        m_themeFilter = nullptr;
    }
    
#elif defined(Q_OS_LINUX)
    if (m_settingsInterface) {
        delete m_settingsInterface;
        m_settingsInterface = nullptr;
    }
#endif
    // macOS 和 Linux 的定时器会在对象销毁时自动清理
}

bool WeatherPlugin::isDarkTheme() const {
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

void WeatherPlugin::onThemeChanged() {
    bool isDark = isDarkTheme();
    
    // 如果主题发生变化
    if (isDark != currentIsDarkTheme) {
        qDebug() << "Theme changed from" << (currentIsDarkTheme ? "dark" : "light") 
                 << "to" << (isDark ? "dark" : "light");
        
        updateIconPathsForTheme();
        
        // 立即更新托盘图标
        if (trayIcon) {
            trayIcon->setIcon(updateIcon());
        }
    }
    m_animation.reset();  // 强制下次更新时重新加载
}

void WeatherPlugin::updateIconPathsForTheme() {
    bool isDark = isDarkTheme();
    
    currentIsDarkTheme = isDark;
}
