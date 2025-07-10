#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ITrayLoadPlugin;
class QStackedWidget;
class QSplitter;
class QListWidget;

#include "pluginsettingswidget.h"  // 添加新头文件

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;

public slots:
    void showMainWindow();
    void showPluginSettings(ITrayLoadPlugin* plugin);
    void onSettingsSaved();
    void onSettingsCancelled();

private:
    Ui::MainWindow *ui;
    QWidget* pluginContainer;  // 保存插件容器引用
    QStackedWidget* stackedWidget;  // 添加堆叠窗口成员
    PluginSettingsWidget* currentSettingsWidget = nullptr;  // 更新类型
    ITrayLoadPlugin* currentPlugin = nullptr;  // 当前设置的插件
    QSplitter* splitter;          // 新增：左右分隔器
    QListWidget* leftSidebar;     // 新增：左侧选项栏
};

#endif // MAINWINDOW_H
