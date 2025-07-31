#pragma once

#include <QObject>
#include <QtPlugin>
#include <QTimer>
#include <QTime>

#include "interface/itrayloadplugin.h"

class QIcon;
class QSystemTrayIcon;
class QMenu;
class QAction;
class QWidget;

class TimerPlugin : public ITrayLoadPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ITrayLoadPlugin_iid FILE "timerplugin.json")
    Q_INTERFACES(ITrayLoadPlugin)

public:
    QString name() const override { return "Timer Plugin"; }
    QSystemTrayIcon* init() override;
    void stop() override;
    void setStatusCallback(std::function<void(int)> callback) override;
    bool hasSettings() override;
    QWidget* createSettingsWidget() override;

    QIcon updateIcon();
    
    // 计时器控制接口
    void startTimer();
    void pauseTimer();
    void resetTimer();
    
    // 获取当前时间
    QTime getCurrentTime() const;
    
    // 设置计时器时间
    void setTimerDuration(int seconds);
    int getTimerDuration() const;

private slots:
    void onTimerTimeout();
    void updateTrayIcon();
    
private:
    QTimer* m_timer = nullptr;
    QSystemTrayIcon* trayIcon = nullptr;
    QMenu* trayMenu = nullptr;
    
    // 计时器相关
    QTime m_currentTime;
    QTime m_targetTime;
    int m_timerDuration = 60; // 默认60秒
    bool m_isRunning = false;
    
    // 图标相关
    QIcon m_icon;
};