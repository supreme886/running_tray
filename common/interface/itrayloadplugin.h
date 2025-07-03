// itrayloadplugin.h
#ifndef ITRAYLOADPLUGIN_H
#define ITRAYLOADPLUGIN_H

#include <QObject>
#include <QPixmap>

class ITrayLoadPlugin {
public:
    virtual ~ITrayLoadPlugin() = default;

    // 插件名称
    virtual QString name() const = 0;

    // 初始化（托盘icon注册）
    virtual void init() = 0;

    // 每隔固定时间调用此函数更新图标
    virtual QIcon updateIcon() = 0;
};

#define ITrayLoadPlugin_iid "com.yourorg.TrayLoadPlugin"
Q_DECLARE_INTERFACE(ITrayLoadPlugin, ITrayLoadPlugin_iid)

#endif
