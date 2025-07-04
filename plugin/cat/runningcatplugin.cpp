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
    
    // 添加分隔线
    trayMenu->addSeparator();
    
    // 添加共享菜单项
    for (QAction* action : SharedMenuManager::instance().getSharedActions()) {
        trayMenu->addAction(action);
    }
    
    // 设置托盘菜单
    trayIcon->setContextMenu(trayMenu);
    
    // 初始化图标路径
    iconPaths = {
        ":/dark_cat_1.ico",
        ":/dark_cat_2.ico",
        ":/dark_cat_3.ico",
        ":/dark_cat_4.ico",
        ":/dark_cat_5.ico"
    };
    currentIndex = 0;

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
    
    // 隐藏托盘图标
    if (trayIcon) {
        trayIcon->hide();
    }
}

void RunningCatPlugin::setStatusCallback(std::function<void (int)> callback)
{

}

QIcon RunningCatPlugin::updateIcon() {
    if (iconPaths.isEmpty()) {
        qDebug() << "No icon paths available";
        return QIcon();
    }
    qDebug() << "Loading icon from:" << iconPaths[currentIndex];
    QIcon icon(iconPaths[currentIndex]);
    qDebug() << "Icon is null:" << icon.isNull();
    
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
#else
    // 非Windows平台不实现CPU监控
    qDebug() << "CPU monitoring not supported on this platform";
#endif
}

// 添加图标更新槽函数实现
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

