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

    void setPluginDescription(const QString &description);

    void setHasSettings(bool hasSettings);

signals:
    // 控制按钮点击信号
    void controlClicked(bool isRunning);
    void settingsClicked(bool checked);

private slots:
    // 控制按钮点击处理
    void onControlButtonClicked();

protected:
    QPushButton* controlBtn;
    QPushButton* settingsBtn;
    
private:
    QLabel* iconLabel;
    QLabel* nameLabel;
    QLabel* descLabel;  // 新增描述标签
    bool isRunning;          // 运行状态标记
};

// 新增：空卡片部件类
class EmptyCardWidget : public PluginCardWidget {
    Q_OBJECT

public:
    explicit EmptyCardWidget(QWidget *parent = nullptr);
};

#endif // PLUGINCARDWIDGET_H
