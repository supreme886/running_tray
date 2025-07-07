// itrayloadplugin.h
#ifndef ITRAYLOADPLUGIN_H
#define ITRAYLOADPLUGIN_H

#include <QObject>
#include <QIcon>

#include "common_exports.h"

class QSystemTrayIcon;

class ITRAYLOADPLUGIN_EXPORT ITrayLoadPlugin : public QObject {
    Q_OBJECT
public:
    // 必须在cpp文件中实现析构函数
    virtual ~ITrayLoadPlugin() override;

    virtual QString name() const = 0;
    virtual QSystemTrayIcon* init() = 0;
    virtual void stop() = 0;
    virtual void setStatusCallback(std::function<void(int)> callback) = 0;


    // 设置相关接口
    virtual bool hasSettings() {return true;}          // 是否支持设置
    virtual QWidget* createSettingsWidget() {return nullptr;}   // 创建设置界面
    virtual void saveSettings(){}               // 保存设置
    virtual void cancelSettings(){}             // 取消设置

signals:
    // 导出信号必须在接口类中声明
    void iconUpdated(const QIcon &icon);
};

#define ITrayLoadPlugin_iid "com.runnigTray.TrayLoadPlugin"
Q_DECLARE_INTERFACE(ITrayLoadPlugin, ITrayLoadPlugin_iid)

#endif
