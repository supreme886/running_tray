#include "sharedmenumanager.h"

#include <QApplication>

SharedMenuManager::SharedMenuManager(QObject* parent) : QObject(parent) {
    // 添加默认共享菜单项
    QAction* exitAction = new QAction("Exit", this);
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
        sharedActions.append(action);
    }
}
