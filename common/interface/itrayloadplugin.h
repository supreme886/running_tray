// itrayloadplugin.h
#ifndef ITRAYLOADPLUGIN_H
#define ITRAYLOADPLUGIN_H

#include <QObject>
#include <QIcon>
#include <QSystemTrayIcon>

// 导出/导入宏定义
#ifdef COMMON_LIBRARY
#define ITRAYLOADPLUGIN_EXPORT Q_DECL_EXPORT
#else
#define ITRAYLOADPLUGIN_EXPORT Q_DECL_IMPORT
#endif

class ITRAYLOADPLUGIN_EXPORT ITrayLoadPlugin : public QObject {
    Q_OBJECT
public:
    // 必须在cpp文件中实现析构函数
    virtual ~ITrayLoadPlugin() override;

    virtual QString name() const = 0;
    virtual QSystemTrayIcon* init() = 0;
    virtual void stop() = 0;
    virtual void setStatusCallback(std::function<void(int)> callback) = 0;

signals:
    // 导出信号必须在接口类中声明
    void iconUpdated(const QIcon &icon);
};

#define ITrayLoadPlugin_iid "com.yourorg.TrayLoadPlugin"
Q_DECLARE_INTERFACE(ITrayLoadPlugin, ITrayLoadPlugin_iid)

#endif
