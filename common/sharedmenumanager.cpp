#include "sharedmenumanager.h"
#include "autostartmanager.h"

#include <QApplication>
#include <QMessageBox>
#include <qDebug>
#include <functional>

SharedMenuManager::SharedMenuManager(QObject* parent) : QObject(parent), autoStartAction(nullptr) {
    // 添加显示主窗口菜单项
    QAction* showMainWindowAction = new QAction("显示主窗口", this);
    connect(showMainWindowAction, &QAction::triggered, [this]() {
        if (showMainWindowCallback) {
            showMainWindowCallback();
        }
    });
    sharedActions.append(showMainWindowAction);
    
    // 添加分隔线
    QAction* separatorAction1 = new QAction(this);
    separatorAction1->setSeparator(true);
    sharedActions.append(separatorAction1);
    
    // 添加开机自启动菜单项
    autoStartAction = new QAction("开机自启动", this);
    autoStartAction->setCheckable(true);
    autoStartAction->setChecked(AutoStartManager::instance().isAutoStartEnabled());
    connect(autoStartAction, &QAction::triggered, this, &SharedMenuManager::onAutoStartToggled);
    sharedActions.append(autoStartAction);
    
    // 添加分隔线
    QAction* separatorAction2 = new QAction(this);
    separatorAction2->setSeparator(true);
    sharedActions.append(separatorAction2);
    
    // 添加退出菜单项
    QAction* exitAction = new QAction("退出", this);
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);
    sharedActions.append(exitAction);
}

SharedMenuManager::~SharedMenuManager() {
    qDeleteAll(sharedActions);
    sharedActions.clear();
}

SharedMenuManager& SharedMenuManager::instance() {
    static SharedMenuManager instance;
    return instance;
}

QList<QAction*> SharedMenuManager::getSharedActions() {
    return sharedActions;
}

void SharedMenuManager::addSharedAction(QAction* action) {
    if (action && !sharedActions.contains(action)) {
        // 在退出菜单项之前插入新的动作
        int exitIndex = sharedActions.size() - 1; // 退出菜单项通常是最后一个
        sharedActions.insert(exitIndex, action);
    }
}

void SharedMenuManager::setShowMainWindowCallback(std::function<void()> callback) {
    showMainWindowCallback = callback;
}

void SharedMenuManager::onAutoStartToggled(bool checked) {
    bool success = AutoStartManager::instance().setAutoStart(checked);
    
    if (success) {
        QString message = checked ? "开机自启动已启用" : "开机自启动已禁用";
        qDebug() << message;
        
        // 可选：显示通知消息
        // QMessageBox::information(nullptr, "自启动设置", message);
    } else {
        QString errorMessage = checked ? "启用开机自启动失败" : "禁用开机自启动失败";
        qWarning() << errorMessage;
        
        // 恢复菜单项状态
        if (autoStartAction) {
            autoStartAction->setChecked(!checked);
        }
        
        QMessageBox::warning(nullptr, "错误", errorMessage);
    }
}
