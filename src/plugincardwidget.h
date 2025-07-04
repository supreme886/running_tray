#ifndef PLUGINCARDWIDGET_H
#define PLUGINCARDWIDGET_H

#include <QFrame>

class QVBoxLayout;
class QLabel;
class QPushButton;

class PluginCardWidget : public QFrame {
    Q_OBJECT

public:
    explicit PluginCardWidget(QWidget *parent = nullptr);
    
    // 设置插件名称
    void setPluginName(const QString &name);
    
    // 设置插件图标
    void setPluginIcon(const QIcon &icon);
    
    // 设置运行状态
    void setRunningState(bool running);

signals:
    // 控制按钮点击信号
    void controlClicked(bool isRunning);

private slots:
    // 控制按钮点击处理
    void onControlButtonClicked();

private:
    QLabel *iconLabel;       // 插件图标
    QLabel *nameLabel;       // 插件名称
    QPushButton *controlBtn; // 启动/停止按钮
    bool isRunning;          // 运行状态标记
};

#endif // PLUGINCARDWIDGET_H
