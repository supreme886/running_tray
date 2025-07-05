#include "sharedmenumanager.h"

#include <QApplication>
#include <functional>

SharedMenuManager::SharedMenuManager(QObject* parent) : QObject(parent) {
    // 添加显示主窗口菜单项
    QAction* showMainWindowAction = new QAction("显示主窗口", this);
    connect(showMainWindowAction, &QAction::triggered, [this]() {
        if (showMainWindowCallback) {
            showMainWindowCallback();
        }
    });
    sharedActions.append(showMainWindowAction);
    
    // 添加分隔线
    QAction* separatorAction = new QAction(this);
    separatorAction->setSeparator(true);
    sharedActions.append(separatorAction);
    
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
