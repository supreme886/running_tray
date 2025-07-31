#include "timerplugin.h"
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
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QTime>

QSystemTrayIcon* TimerPlugin::init() {
    // 创建托盘实例
    trayIcon = new QSystemTrayIcon(this);
    trayMenu = new QMenu;
    
    // 添加插件特有菜单项
    QAction* startAction = new QAction("Start Timer", this);
    connect(startAction, &QAction::triggered, this, &TimerPlugin::startTimer);
    trayMenu->addAction(startAction);
    
    QAction* pauseAction = new QAction("Pause Timer", this);
    connect(pauseAction, &QAction::triggered, this, &TimerPlugin::pauseTimer);
    trayMenu->addAction(pauseAction);
    
    QAction* resetAction = new QAction("Reset Timer", this);
    connect(resetAction, &QAction::triggered, this, &TimerPlugin::resetTimer);
    trayMenu->addAction(resetAction);
    
    trayMenu->addSeparator();
    
    QAction* aboutAction = new QAction("About Timer", this);
    connect(aboutAction, &QAction::triggered, [this](){
        QMessageBox::information(qApp->activeWindow(), "About", "Timer Plugin v1.0");
    });
    trayMenu->addAction(aboutAction);
    
    trayMenu->addSeparator();
    
    // 添加共享菜单项
    for (QAction* action : SharedMenuManager::instance().getSharedActions()) {
        trayMenu->addAction(action);
    }
    
    // 设置托盘菜单
    trayIcon->setContextMenu(trayMenu);
    
    // 初始化计时器
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &TimerPlugin::onTimerTimeout);
    
    // 初始化时间
    m_currentTime = QTime(0, 0, 0);
    
    // 设置初始图标并显示托盘
    trayIcon->setIcon(updateIcon());
    trayIcon->show();
    
    return trayIcon;
}

void TimerPlugin::stop() {
    qDebug() << Q_FUNC_INFO << __LINE__;
    
    // 停止计时器
    if (m_timer) {
        m_timer->stop();
        delete m_timer;
        m_timer = nullptr;
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
}

void TimerPlugin::setStatusCallback(std::function<void (int)> callback) {
    // 实现状态回调功能（如果需要）
}

bool TimerPlugin::hasSettings() {
    return true;
}

QWidget* TimerPlugin::createSettingsWidget() {
    QWidget* settingsWidget = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(settingsWidget);
    
    // 创建控制按钮
    QGroupBox* controlGroupBox = new QGroupBox("Timer Control");
    QVBoxLayout* controlLayout = new QVBoxLayout(controlGroupBox);
    
    QPushButton* startButton = new QPushButton("Start");
    QPushButton* pauseButton = new QPushButton("Pause");
    QPushButton* resetButton = new QPushButton("Reset");
    
    connect(startButton, &QPushButton::clicked, this, &TimerPlugin::startTimer);
    connect(pauseButton, &QPushButton::clicked, this, &TimerPlugin::pauseTimer);
    connect(resetButton, &QPushButton::clicked, this, &TimerPlugin::resetTimer);
    
    controlLayout->addWidget(startButton);
    controlLayout->addWidget(pauseButton);
    controlLayout->addWidget(resetButton);
    
    mainLayout->addWidget(controlGroupBox);
    
    // 显示当前时间
    QGroupBox* timeGroupBox = new QGroupBox("Current Time");
    QVBoxLayout* timeLayout = new QVBoxLayout(timeGroupBox);
    
    QLabel* timeLabel = new QLabel(m_currentTime.toString("mm:ss"));
    timeLabel->setAlignment(Qt::AlignCenter);
    timeLayout->addWidget(timeLabel);
    
    mainLayout->addWidget(timeGroupBox);
    mainLayout->addStretch();
    
    return settingsWidget;
}

QIcon TimerPlugin::updateIcon() {
    // 简单实现：创建一个带时间文本的图标
    // 实际项目中可能需要创建更美观的图标
    return QIcon();
}

void TimerPlugin::startTimer() {
    if (!m_isRunning) {
        m_timer->start(1000); // 每秒更新一次
        m_isRunning = true;
        updateTrayIcon();
    }
}

void TimerPlugin::pauseTimer() {
    if (m_isRunning) {
        m_timer->stop();
        m_isRunning = false;
        updateTrayIcon();
    }
}

void TimerPlugin::resetTimer() {
    m_timer->stop();
    m_isRunning = false;
    m_currentTime = QTime(0, 0, 0);
    updateTrayIcon();
}

QTime TimerPlugin::getCurrentTime() const {
    return m_currentTime;
}

void TimerPlugin::setTimerDuration(int seconds) {
    if (seconds > 0) {
        m_timerDuration = seconds;
    }
}

int TimerPlugin::getTimerDuration() const {
    return m_timerDuration;
}

void TimerPlugin::onTimerTimeout() {
    if (m_isRunning) {
        m_currentTime = m_currentTime.addSecs(1);
        updateTrayIcon();
        
        // 检查是否到达目标时间
        if (m_currentTime >= m_targetTime) {
            // 计时结束，停止计时器
            m_timer->stop();
            m_isRunning = false;
            
            // 显示通知
            if (trayIcon) {
                trayIcon->showMessage("Timer Finished", "Time's up!", QSystemTrayIcon::Information, 5000);
            }
        }
    }
}

void TimerPlugin::updateTrayIcon() {
    if (trayIcon) {
        trayIcon->setIcon(updateIcon());
        trayIcon->setToolTip(m_currentTime.toString("mm:ss"));
    }
}