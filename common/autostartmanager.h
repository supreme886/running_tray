#pragma once

#include <QObject>
#include <QString>

// 添加导出/导入宏定义
#ifdef COMMON_LIBRARY
#define COMMON_LIBRARY_EXPORT Q_DECL_EXPORT
#else
#define COMMON_LIBRARY_EXPORT Q_DECL_IMPORT
#endif

class COMMON_LIBRARY_EXPORT AutoStartManager : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(AutoStartManager)

public:
    static AutoStartManager& instance();
    
    // 设置开机自启动
    bool setAutoStart(bool enabled);
    
    // 检查是否已设置开机自启动
    bool isAutoStartEnabled() const;
    
    // 获取应用程序名称
    QString getApplicationName() const;
    
    // 获取应用程序路径
    QString getApplicationPath() const;
    
private:
    AutoStartManager(QObject* parent = nullptr);
    ~AutoStartManager() override;
    
    // 平台特定的实现
#ifdef Q_OS_WIN
    bool setAutoStartWindows(bool enabled);
    bool isAutoStartEnabledWindows() const;
#elif defined(Q_OS_MACOS)
    bool setAutoStartMacOS(bool enabled);
    bool isAutoStartEnabledMacOS() const;
#elif defined(Q_OS_LINUX)
    bool setAutoStartLinux(bool enabled);
    bool isAutoStartEnabledLinux() const;
#endif
};