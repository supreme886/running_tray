#include "runningcatplugin.h"
#include "sharedmenumanager.h"
#include "thememanager.h"  // 包含新的主题管理器

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

// 添加平台特定的包含
#ifdef Q_OS_WIN
#include <windows.h>
#include <QSettings>
#elif defined(Q_OS_MACOS)
#include <mach/mach.h>
#include <mach/processor_info.h>
#include <mach/mach_host.h>
#include <QSettings>
#elif defined(Q_OS_LINUX)
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#endif

QSystemTrayIcon* RunningCatPlugin::init() {
    // 创建托盘实例
    trayIcon = new QSystemTrayIcon(this);
    trayMenu = new QMenu;
    
    // 添加插件特有菜单项
    QAction* aboutAction = new QAction("About Running Cat", this);
    connect(aboutAction, &QAction::triggered, [this](){
        QMessageBox::information(qApp->activeWindow(), "About", "Running Cat Plugin v1.0");
    });
    trayMenu->addAction(aboutAction);
    
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

    // 使用新的主题管理器
    m_themeManager = new ThemeManager(this);
    connect(m_themeManager, &ThemeManager::themeChanged, this, &RunningCatPlugin::onThemeChanged);
    m_themeManager->startMonitoring();
    
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
    
    return trayIcon;
}

void RunningCatPlugin::stop() {
    qDebug() << Q_FUNC_INFO << __LINE__;
    
    // 停止定时器
    if (cpuTimer) {
        cpuTimer->stop();
        delete cpuTimer;
        cpuTimer = nullptr;
    }

    if (iconUpdateTimer) {
        iconUpdateTimer->stop();
        delete iconUpdateTimer;
        iconUpdateTimer = nullptr;
    }
    
    // 停止主题监控
    if (m_themeManager) {
        m_themeManager->stopMonitoring();
        delete m_themeManager;
        m_themeManager = nullptr;
    }

    if (trayIcon) {
        trayIcon->hide();
        delete trayIcon;
        trayIcon = nullptr;
    }
    
    if (trayMenu) {
        delete trayMenu;
        trayMenu = nullptr;
    }
    
    // 清理macOS资源
#ifdef Q_OS_MACOS
    if (lastCpuInfo) {
        vm_deallocate(mach_task_self(), (vm_address_t)lastCpuInfo, lastNumCpuInfo * sizeof(integer_t));
        lastCpuInfo = nullptr;
    }
#endif
}

void RunningCatPlugin::setStatusCallback(std::function<void (int)> callback) {
    // 实现状态回调功能（如果需要）
}

// 图标尺寸配置接口实现
void RunningCatPlugin::setIconSize(int size) {
    if (size > 0 && size <= 128) {
        iconSize = size;
        qDebug() << "Icon size set to:" << iconSize;

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
    
    // 立即更新托盘图标
    if (trayIcon) {
        trayIcon->setIcon(updateIcon());
    }
}

bool RunningCatPlugin::isAutoScaleEnabled() const {
    return autoScaleIcon;
}

bool RunningCatPlugin::hasSettings() {
    return true;
}

QWidget* RunningCatPlugin::createSettingsWidget() {
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
    
    // 添加自动缩放选项
    QCheckBox* autoScaleBox = new QCheckBox("Auto Scale (DPI Aware)");
    autoScaleBox->setChecked(autoScaleIcon);
    connect(autoScaleBox, &QCheckBox::toggled, this, &RunningCatPlugin::setAutoScaleIcon);
    sizeLayout->addWidget(autoScaleBox);
    
    mainLayout->addWidget(sizeGroupBox);
    mainLayout->addStretch();
    
    return settingsWidget;
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

    // 根据CPU使用率动态调整刷新间隔
    int newInterval;
    if (cpuUsage < 10.0) {
        newInterval = 200;
    } else if (cpuUsage < 20.0) {
        newInterval = 184;
    } else if (cpuUsage < 30.0) {
        newInterval = 168;
    } else if (cpuUsage < 40.0) {
        newInterval = 152;
    } else if (cpuUsage < 50.0) {
        newInterval = 136;
    } else if (cpuUsage < 60.0) {
        newInterval = 120;
    } else if (cpuUsage < 70.0) {
        newInterval = 104;
    } else if (cpuUsage < 80.0) {
        newInterval = 88;
    } else if (cpuUsage < 90.0) {
        newInterval = 72;
    } else {
        newInterval = 40;
    }

    trayIcon->setToolTip(QString("CPU:%1%").arg(QString::number(cpuUsage,'f',2)));

    // 如果间隔变化则更新
    if (newInterval != currentRefreshInterval) {
        currentRefreshInterval = newInterval;
        iconUpdateTimer->setInterval(newInterval);
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
    
    // 根据CPU使用率动态调整刷新间隔
    int newInterval;
    if (cpuUsage < 10.0) {
        newInterval = 200;
    } else if (cpuUsage < 20.0) {
        newInterval = 184;
    } else if (cpuUsage < 30.0) {
        newInterval = 168;
    } else if (cpuUsage < 40.0) {
        newInterval = 152;
    } else if (cpuUsage < 50.0) {
        newInterval = 136;
    } else if (cpuUsage < 60.0) {
        newInterval = 120;
    } else if (cpuUsage < 70.0) {
        newInterval = 104;
    } else if (cpuUsage < 80.0) {
        newInterval = 88;
    } else if (cpuUsage < 90.0) {
        newInterval = 72;
    } else {
        newInterval = 40;
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
    qDebug() << "CPU monitoring not supported on this platform";
#endif
}

void RunningCatPlugin::onIconUpdateTimeout() {
    if (trayIcon) {
        QIcon icon = updateIcon();
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
    darkIconPaths.clear();
    darkIconPaths << ":/res/dark_cat_1.ico"
                  << ":/res/dark_cat_2.ico"
                  << ":/res/dark_cat_3.ico"
                  << ":/res/dark_cat_4.ico"
                  << ":/res/dark_cat_5.ico";
    
    // 初始化亮色主题图标路径
    lightIconPaths.clear();
    lightIconPaths << ":/res/light_cat_1.ico"
                   << ":/res/light_cat_2.ico"
                   << ":/res/light_cat_3.ico"
                   << ":/res/light_cat_4.ico"
                   << ":/res/light_cat_5.ico";
    
    // 根据当前主题设置图标路径
    updateIconPathsForTheme();
}

void RunningCatPlugin::onThemeChanged() {
    qDebug() << "Theme changed, updating icon paths";
    updateIconPathsForTheme();
    
    // 立即更新托盘图标
    if (trayIcon) {
        trayIcon->setIcon(updateIcon());
    }
}

void RunningCatPlugin::updateIconPathsForTheme() {
    bool isDark = m_themeManager ? m_themeManager->isDarkTheme() : false;
    
    if (isDark) {
        iconPaths = darkIconPaths;
        qDebug() << "Using dark theme icons";
    } else {
        iconPaths = lightIconPaths;
        qDebug() << "Using light theme icons";
    }
    
    // 重置图标索引
    currentIndex = 0;
}
