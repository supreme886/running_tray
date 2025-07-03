#include "runningcatplugin.h"
#include <QIcon>
#include <QDebug>
#include <QTimer>

void RunningCatPlugin::init() {
    iconPaths = {
        ":/dark_cat_1.ico",
        ":/dark_cat_2.ico",
        ":/dark_cat_3.ico",
        ":/dark_cat_4.ico",
        ":/dark_cat_5.ico"
    };
    currentIndex = 0;

    currentRefreshInterval = 1000; // 默认1秒刷新间隔

    // 初始化CPU监测定时器(每2秒检查一次CPU使用率)
    cpuTimer = new QTimer(this);
    connect(cpuTimer, &QTimer::timeout, this, &RunningCatPlugin::updateCPUUsage);
    cpuTimer->start(2000);

    // 获取初始系统时间用于后续CPU使用率计算
    FILETIME idleTime, kernelTime, userTime;
    if (GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        lastIdleTime = fileTimeToULargeInteger(idleTime);
        lastKernelTime = fileTimeToULargeInteger(kernelTime);
        lastUserTime = fileTimeToULargeInteger(userTime);
    } else {
        qDebug() << "获取初始系统时间失败";
    }
}

QIcon RunningCatPlugin::updateIcon() {
    if (iconPaths.isEmpty()) {
        qDebug() << "No icon paths available";
        return QIcon();
    }
    qDebug() << "Loading icon from:" << iconPaths[currentIndex];
    QIcon icon(iconPaths[currentIndex]);
    qDebug() << "Icon is null:" << icon.isNull();
    currentIndex = (currentIndex + 1) % iconPaths.size();
    return icon;
}

ULARGE_INTEGER RunningCatPlugin::fileTimeToULargeInteger(const FILETIME& ft) {
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return uli;
}

void RunningCatPlugin::updateCPUUsage() {
    FILETIME idleTime, kernelTime, userTime;
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        qDebug() << "获取系统时间失败";
        return;
    }

    // 转换为ULARGE_INTEGER以便计算
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
    qDebug() << QString("CPU使用率: %1%").arg(cpuUsage, 0, 'f', 1);

    // 根据CPU使用率动态调整刷新间隔
    int newInterval;
    if (cpuUsage < 30.0) {
        newInterval = 200;   // 低负载
    } else if (cpuUsage < 70.0) {
        newInterval = 100;  // 中等负载
    } else {
        newInterval = 50;  // 高负载
    }

    // 如果间隔有变化则更新
    if (newInterval != currentRefreshInterval) {
        currentRefreshInterval = newInterval;
        qDebug() << QString("插件刷新间隔已调整为: %1ms").arg(currentRefreshInterval);
    }

    // 更新上次时间戳
    lastIdleTime = idle;
    lastKernelTime = kernel;
    lastUserTime = user;
}

