#include "runningcatplugin.h"
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
            if (setting == "ImmersiveColorSet" || setting.contains("Theme")) {
                // 延迟一点执行，确保系统设置已更新
                QTimer::singleShot(100, m_plugin, &RunningCatPlugin::onThemeChanged);
            }
        }
    }
    return false;
}
#endif

// 在 init() 函数中保存菜单项引用
QSystemTrayIcon* RunningCatPlugin::init() {
    // 创建托盘实例
    trayIcon = new QSystemTrayIcon(this);
    
    // 创建托盘菜单
    trayMenu = new QMenu();
    
    // 添加插件特有菜单项
    QAction* aboutAction = new QAction("About Running Cat", this);
    connect(aboutAction, &QAction::triggered, [this](){
        QMessageBox::information(nullptr, "About", "Running Cat Plugin v1.0");
    });
    trayMenu->addAction(aboutAction);
    
    // 新增：图标尺寸配置菜单
    QMenu* iconSizeMenu = new QMenu("Icon Size", trayMenu);
    
    // 预设尺寸选项
    QActionGroup* sizeGroup = new QActionGroup(this);
    size16Action = new QAction("16x16 (Small)", sizeGroup);
    size24Action = new QAction("24x24 (Medium)", sizeGroup);
    size32Action = new QAction("32x32 (Large)", sizeGroup);
    
    size16Action->setCheckable(true);
    size24Action->setCheckable(true);
    size32Action->setCheckable(true);
    size16Action->setChecked(iconSize == 16);
    size24Action->setChecked(iconSize == 24);
    size32Action->setChecked(iconSize == 32);
    
    connect(size16Action, &QAction::triggered, [this]() { setIconSize(16); });
    connect(size24Action, &QAction::triggered, [this]() { setIconSize(24); });
    connect(size32Action, &QAction::triggered, [this]() { setIconSize(32); });
    
    iconSizeMenu->addAction(size16Action);
    iconSizeMenu->addAction(size24Action);
    iconSizeMenu->addAction(size32Action);
    iconSizeMenu->addSeparator();
    
    // 自动缩放选项
    autoScaleAction = new QAction("Auto Scale (DPI Aware)", this);
    autoScaleAction->setCheckable(true);
    autoScaleAction->setChecked(autoScaleIcon);
    connect(autoScaleAction, &QAction::triggered, [this](bool checked) {
        setAutoScaleIcon(checked);
    });
    iconSizeMenu->addAction(autoScaleAction);
    
    trayMenu->addMenu(iconSizeMenu);
    
    // 添加分隔线
    trayMenu->addSeparator();
    
    // 添加共享菜单项
    for (QAction* action : SharedMenuManager::instance().getSharedActions()) {
        trayMenu->addAction(action);
    }
    
    // 设置托盘菜单
    trayIcon->setContextMenu(trayMenu);
    
    // 初始化图标路径
    initializeIconPaths();

    setupThemeMonitoring();
    
    currentRefreshInterval = 1000;

    // 初始化CPU监控定时器
    cpuTimer = new QTimer(this);
    connect(cpuTimer, &QTimer::timeout, this, &RunningCatPlugin::updateCPUUsage);
    cpuTimer->start(2000);

    // 初始化图标刷新定时器
    iconUpdateTimer = new QTimer(this);
    connect(iconUpdateTimer, &QTimer::timeout, this, &RunningCatPlugin::onIconUpdateTimeout);
    iconUpdateTimer->start(currentRefreshInterval);

    // 获取初始系统时间
#ifdef Q_OS_WIN
    FILETIME idleTime, kernelTime, userTime;
    if (GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        lastIdleTime = fileTimeToULargeInteger(idleTime);
        lastKernelTime = fileTimeToULargeInteger(kernelTime);
        lastUserTime = fileTimeToULargeInteger(userTime);
    } else {
        qDebug() << "Failed to get initial system time";
    }
#elif defined(Q_OS_MACOS)
    // 初始化macOS CPU信息
    kern_return_t kr = host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO,
                                          &lastNumCpus, &lastCpuInfo, &lastNumCpuInfo);
    if (kr != KERN_SUCCESS) {
        qDebug() << "Failed to get initial CPU info on macOS";
        lastCpuInfo = nullptr;
        lastNumCpuInfo = 0;
        lastNumCpus = 0;
    }
#else
    qDebug() << "CPU monitoring not supported on this platform";
#endif
    
    // 设置初始图标并显示托盘
    trayIcon->setIcon(updateIcon());
    trayIcon->show();
    
    return trayIcon;  // 返回创建的托盘实例
}

void RunningCatPlugin::stop() {
    // 停止定时器
    if (cpuTimer) {
        cpuTimer->stop();
    }
    if (iconUpdateTimer) {
        iconUpdateTimer->stop();
    }
    
    // 清理主题监听
    cleanupThemeMonitoring();
    
    // 隐藏托盘图标
    if (trayIcon) {
        trayIcon->hide();
    }
    
    // 清理macOS资源
#ifdef Q_OS_MACOS
    if (lastCpuInfo) {
        vm_deallocate(mach_task_self(), (vm_address_t)lastCpuInfo, lastNumCpuInfo * sizeof(integer_t));
        lastCpuInfo = nullptr;
    }
#endif
}

void RunningCatPlugin::setStatusCallback(std::function<void (int)> callback)
{
    // 实现状态回调功能（如果需要）
}

// 图标尺寸配置接口实现
void RunningCatPlugin::setIconSize(int size) {
    if (size > 0 && size <= 128) {  // 限制合理的尺寸范围
        iconSize = size;
        qDebug() << "Icon size set to:" << iconSize;
        
        // 更新菜单项选中状态
        if (size16Action) size16Action->setChecked(size == 16);
        if (size24Action) size24Action->setChecked(size == 24);
        if (size32Action) size32Action->setChecked(size == 32);
        
        // 立即更新托盘图标
        if (trayIcon) {
            trayIcon->setIcon(updateIcon());
        }
    } else {
        qDebug() << "Invalid icon size:" << size << ". Size must be between 1 and 128.";
    }
}

int RunningCatPlugin::getIconSize() const {
    return iconSize;
}

void RunningCatPlugin::setAutoScaleIcon(bool enabled) {
    autoScaleIcon = enabled;
    qDebug() << "Auto scale icon:" << (enabled ? "enabled" : "disabled");
    
    // 更新菜单项选中状态
    if (autoScaleAction) {
        autoScaleAction->setChecked(enabled);
    }
    
    // 立即更新托盘图标
    if (trayIcon) {
        trayIcon->setIcon(updateIcon());
    }
}

bool RunningCatPlugin::isAutoScaleEnabled() const {
    return autoScaleIcon;
}

// 创建缩放图标的方法
QIcon RunningCatPlugin::createScaledIcon(const QString& iconPath) {
    QIcon originalIcon(iconPath);
    if (originalIcon.isNull()) {
        return QIcon();
    }
    
    int targetSize = iconSize;
    
    // 如果启用自动缩放，根据DPI调整尺寸
    if (autoScaleIcon) {
        QScreen* screen = QApplication::primaryScreen();
        if (screen) {
            qreal dpiRatio = screen->devicePixelRatio();
            targetSize = static_cast<int>(iconSize * dpiRatio);
        }
    }
    
    // 创建指定尺寸的图标
    QPixmap pixmap = originalIcon.pixmap(targetSize, targetSize);
    
    // 为不同尺寸添加图标变体，提供更好的显示效果
    QIcon scaledIcon;
    scaledIcon.addPixmap(pixmap);
    
    // 添加常见尺寸的变体
    if (targetSize != 16) {
        scaledIcon.addPixmap(originalIcon.pixmap(16, 16));
    }
    if (targetSize != 24) {
        scaledIcon.addPixmap(originalIcon.pixmap(24, 24));
    }
    if (targetSize != 32) {
        scaledIcon.addPixmap(originalIcon.pixmap(32, 32));
    }
    
    return scaledIcon;
}

// 更新图标方法以支持自定义尺寸
QIcon RunningCatPlugin::updateIcon() {
    if (iconPaths.isEmpty()) {
        qDebug() << "No icon paths available";
        return QIcon();
    }
    
    QString currentIconPath = iconPaths[currentIndex];
    qDebug() << "Loading icon from:" << currentIconPath << "with size:" << iconSize;
    
    // 使用新的缩放图标方法
    QIcon icon = createScaledIcon(currentIconPath);
    
    if (icon.isNull()) {
        qDebug() << "Failed to create scaled icon";
        // 回退到原始图标
        icon = QIcon(currentIconPath);
    }
    
    // 发送图标更新信号
    emit iconUpdated(QIcon(icon));
    
    currentIndex = (currentIndex + 1) % iconPaths.size();
    return icon;
}

void RunningCatPlugin::updateCPUUsage() {
#ifdef Q_OS_WIN
    FILETIME idleTime, kernelTime, userTime;
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        qDebug() << "Failed to get system time";
        return;
    }

    // 转换为ULARGE_INTEGER进行计算
    ULARGE_INTEGER idle = fileTimeToULargeInteger(idleTime);
    ULARGE_INTEGER kernel = fileTimeToULargeInteger(kernelTime);
    ULARGE_INTEGER user = fileTimeToULargeInteger(userTime);

    // 计算时间差
    ULONGLONG idleDiff = idle.QuadPart - lastIdleTime.QuadPart;
    ULONGLONG kernelDiff = kernel.QuadPart - lastKernelTime.QuadPart;
    ULONGLONG userDiff = user.QuadPart - lastUserTime.QuadPart;

    // 计算CPU使用率
    ULONGLONG totalTime = kernelDiff + userDiff;
    ULONGLONG workTime = totalTime - idleDiff;
    double cpuUsage = (totalTime > 0) ? (double)workTime / totalTime * 100.0 : 0.0;
    qDebug() << QString("CPU Usage: %1%").arg(cpuUsage, 0, 'f', 1);

    // 根据CPU使用率动态调整刷新间隔 (10个等级, 200ms-40ms范围)
    int newInterval;
    if (cpuUsage < 10.0) {
        newInterval = 200;    // 0-10%: 200ms
    } else if (cpuUsage < 20.0) {
        newInterval = 184;    // 10-20%: 184ms
    } else if (cpuUsage < 30.0) {
        newInterval = 168;    // 20-30%: 168ms
    } else if (cpuUsage < 40.0) {
        newInterval = 152;    // 30-40%: 152ms
    } else if (cpuUsage < 50.0) {
        newInterval = 136;    // 40-50%: 136ms
    } else if (cpuUsage < 60.0) {
        newInterval = 120;    // 50-60%: 120ms
    } else if (cpuUsage < 70.0) {
        newInterval = 104;    // 60-70%: 104ms
    } else if (cpuUsage < 80.0) {
        newInterval = 88;     // 70-80%: 88ms
    } else if (cpuUsage < 90.0) {
        newInterval = 72;     // 80-90%: 72ms
    } else {
        newInterval = 40;     // 90-100%: 40ms
    }

    trayIcon->setToolTip(QString("CPU:%1%").arg(QString::number(cpuUsage,'f',2)));

    // 如果间隔变化则更新
    if (newInterval != currentRefreshInterval) {
        currentRefreshInterval = newInterval;
        iconUpdateTimer->setInterval(newInterval);  // 更新图标刷新定时器间隔
        qDebug() << QString("Plugin refresh interval adjusted to: %1ms").arg(currentRefreshInterval);
    }

    // 更新上次时间戳
    lastIdleTime = idle;
    lastKernelTime = kernel;
    lastUserTime = user;
#elif defined(Q_OS_MACOS)
    processor_info_array_t cpuInfo;
    mach_msg_type_number_t numCpuInfo;
    natural_t numCpus;
    
    kern_return_t kr = host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO,
                                          &numCpus, &cpuInfo, &numCpuInfo);
    
    if (kr != KERN_SUCCESS || lastCpuInfo == nullptr) {
        qDebug() << "Failed to get CPU info on macOS";
        return;
    }
    
    double totalUsage = 0.0;
    
    for (natural_t i = 0; i < numCpus; i++) {
        processor_cpu_load_info_t cpuLoadInfo = (processor_cpu_load_info_t)&cpuInfo[i * CPU_STATE_MAX];
        processor_cpu_load_info_t lastCpuLoadInfo = (processor_cpu_load_info_t)&lastCpuInfo[i * CPU_STATE_MAX];
        
        // 计算各个状态的时间差
        uint64_t userDiff = cpuLoadInfo->cpu_ticks[CPU_STATE_USER] - lastCpuLoadInfo->cpu_ticks[CPU_STATE_USER];
        uint64_t systemDiff = cpuLoadInfo->cpu_ticks[CPU_STATE_SYSTEM] - lastCpuLoadInfo->cpu_ticks[CPU_STATE_SYSTEM];
        uint64_t idleDiff = cpuLoadInfo->cpu_ticks[CPU_STATE_IDLE] - lastCpuLoadInfo->cpu_ticks[CPU_STATE_IDLE];
        uint64_t niceDiff = cpuLoadInfo->cpu_ticks[CPU_STATE_NICE] - lastCpuLoadInfo->cpu_ticks[CPU_STATE_NICE];
        
        uint64_t totalTicks = userDiff + systemDiff + idleDiff + niceDiff;
        uint64_t usedTicks = userDiff + systemDiff + niceDiff;
        
        if (totalTicks > 0) {
            totalUsage += (double)usedTicks / totalTicks * 100.0;
        }
    }
    
    double cpuUsage = totalUsage / numCpus;
    qDebug() << QString("CPU Usage: %1%").arg(cpuUsage, 0, 'f', 1);
    
    // 根据CPU使用率动态调整刷新间隔 (与Windows版本相同的逻辑)
    int newInterval;
    if (cpuUsage < 10.0) {
        newInterval = 200;    // 0-10%: 200ms
    } else if (cpuUsage < 20.0) {
        newInterval = 184;    // 10-20%: 184ms
    } else if (cpuUsage < 30.0) {
        newInterval = 168;    // 20-30%: 168ms
    } else if (cpuUsage < 40.0) {
        newInterval = 152;    // 30-40%: 152ms
    } else if (cpuUsage < 50.0) {
        newInterval = 136;    // 40-50%: 136ms
    } else if (cpuUsage < 60.0) {
        newInterval = 120;    // 50-60%: 120ms
    } else if (cpuUsage < 70.0) {
        newInterval = 104;    // 60-70%: 104ms
    } else if (cpuUsage < 80.0) {
        newInterval = 88;     // 70-80%: 88ms
    } else if (cpuUsage < 90.0) {
        newInterval = 72;     // 80-90%: 72ms
    } else {
        newInterval = 40;     // 90-100%: 40ms
    }
    
    trayIcon->setToolTip(QString("CPU:%1%").arg(QString::number(cpuUsage,'f',2)));
    
    // 如果间隔变化则更新
    if (newInterval != currentRefreshInterval) {
        currentRefreshInterval = newInterval;
        iconUpdateTimer->setInterval(newInterval);
        qDebug() << QString("Plugin refresh interval adjusted to: %1ms").arg(currentRefreshInterval);
    }
    
    // 释放旧的CPU信息并保存新的
    if (lastCpuInfo) {
        vm_deallocate(mach_task_self(), (vm_address_t)lastCpuInfo, lastNumCpuInfo * sizeof(integer_t));
    }
    lastCpuInfo = cpuInfo;
    lastNumCpuInfo = numCpuInfo;
    lastNumCpus = numCpus;
    
#else
    // 非Windows/macOS平台不实现CPU监控
    qDebug() << "CPU monitoring not supported on this platform";
#endif
}

// 修改图标更新槽函数实现
void RunningCatPlugin::onIconUpdateTimeout() {
    if (trayIcon) {
        QIcon icon = updateIcon();
        // 直接使用 updateIcon() 返回的图标，它已经包含了正确的尺寸
        trayIcon->setIcon(icon);
    }
}

#ifdef Q_OS_WIN
ULARGE_INTEGER RunningCatPlugin::fileTimeToULargeInteger(const FILETIME& ft) {
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return uli;
}
#endif

void RunningCatPlugin::initializeIconPaths() {
    // 初始化暗色主题图标路径
    lightIconPaths.clear();
    lightIconPaths << ":/res/dark_cat_1.ico"
                  << ":/res/dark_cat_2.ico"
                  << ":/res/dark_cat_3.ico"
                  << ":/res/dark_cat_4.ico"
                  << ":/res/dark_cat_5.ico";
    
    // 初始化亮色主题图标路径
    darkIconPaths.clear();
    darkIconPaths << ":/res/light_cat_1.ico"
                   << ":/res/light_cat_2.ico"
                   << ":/res/light_cat_3.ico"
                   << ":/res/light_cat_4.ico"
                   << ":/res/light_cat_5.ico";
    
    // 根据当前主题设置图标路径
    updateIconPathsForTheme();
}

void RunningCatPlugin::setupThemeMonitoring() {
#ifdef Q_OS_WIN
    // Windows: 监听 WM_SETTINGCHANGE 消息
    m_themeFilter = new ThemeEventFilter(this);
    QApplication::instance()->installNativeEventFilter(m_themeFilter);
    qDebug() << "Windows theme monitoring enabled";
    
#elif defined(Q_OS_MACOS)
    // macOS: 使用简单的定时器检查（更兼容的方式）
    QTimer* themeTimer = new QTimer(this);
    connect(themeTimer, &QTimer::timeout, this, &RunningCatPlugin::onThemeChanged);
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

void RunningCatPlugin::cleanupThemeMonitoring() {
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

bool RunningCatPlugin::isDarkTheme() const {
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

void RunningCatPlugin::onThemeChanged() {
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
}

void RunningCatPlugin::updateIconPathsForTheme() {
    bool isDark = isDarkTheme();
    
    if (isDark) {
        iconPaths = darkIconPaths;
        qDebug() << "Switched to dark theme icons";
    } else {
        iconPaths = lightIconPaths;
        qDebug() << "Switched to light theme icons";
    }
    
    currentIsDarkTheme = isDark;
    currentIndex = 0;  // 重置图标索引
}
