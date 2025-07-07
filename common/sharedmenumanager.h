#pragma once

#include <QObject>
#include <QMenu>
#include <QAction>
#include <functional>
#include "common_exports.h"

class COMMON_LIBRARY_EXPORT SharedMenuManager : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(SharedMenuManager)

public:
    static SharedMenuManager& instance();
    
    // 获取共享菜单项
    QList<QAction*> getSharedActions();
    
    // 添加共享菜单项
    void addSharedAction(QAction* action);
    
    // 设置显示主窗口的回调函数
    void setShowMainWindowCallback(std::function<void()> callback);
    
private slots:
    void onAutoStartToggled(bool checked);
    
private:
    SharedMenuManager(QObject* parent = nullptr);
    ~SharedMenuManager() override;
    
    QList<QAction*> sharedActions;
    std::function<void()> showMainWindowCallback;
    QAction* autoStartAction;
};