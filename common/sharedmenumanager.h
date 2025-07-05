#pragma once

#include <QObject>
#include <QMenu>
#include <QAction>

// 添加导出/导入宏定义
#ifdef COMMON_LIBRARY
#define COMMON_LIBRARY_EXPORT Q_DECL_EXPORT
#else
#define COMMON_LIBRARY_EXPORT Q_DECL_IMPORT
#endif

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
    
private:
    SharedMenuManager(QObject* parent = nullptr);
    ~SharedMenuManager() override;
    
    QList<QAction*> sharedActions;
    std::function<void()> showMainWindowCallback;
};