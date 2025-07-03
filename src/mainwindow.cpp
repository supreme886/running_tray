#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QSystemTrayIcon>
#include <QDir>
#include <QMessageBox>
#include <QTimer>
#include <QStyle>
#include <QLabel>

#include "pluginmanager.h"
#include "interface/itrayloadplugin.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 创建托盘图标对象
    QSystemTrayIcon *trayIcon = new QSystemTrayIcon(this);
    trayIcon->setToolTip("插件托盘演示");

    // 获取插件目录路径（优先 ./plugins，其次 ../plugins）
    QString pluginPath;
    QDir baseDir(QCoreApplication::applicationDirPath());

    if (QDir(baseDir.absoluteFilePath("plugins")).exists()) {
        pluginPath = baseDir.absoluteFilePath("plugins");
    } else if (QDir(baseDir.absoluteFilePath("../plugins")).exists()) {
        pluginPath = baseDir.absoluteFilePath("../plugins");
    } else {
        QMessageBox::warning(nullptr, "未找到插件", "未找到插件目录");
        return;
    }

    // 加载插件
    auto plugins = PluginManager::instance().loadPlugins(pluginPath);
    if (plugins.isEmpty()) {
        QMessageBox::information(nullptr, "无插件", "未加载任何插件");
        return;
    }

    // 使用第一个插件进行展示（可扩展为菜单选择）
    ITrayLoadPlugin *selectedPlugin = plugins.first().plugin;
    selectedPlugin->init();

    // 初始图标
    trayIcon->setIcon(selectedPlugin->updateIcon());

    QTimer *timer = new QTimer(this);
    QObject::connect(timer, &QTimer::timeout, [=]() {
        QIcon icon = selectedPlugin->updateIcon();
        trayIcon->setIcon(icon);
    });
    timer->start(100); // 1 秒更新一次
    trayIcon->show();

    // 从插件获取初始刷新间隔
    int initialInterval = selectedPlugin->refreshInterval();
    timer->start(initialInterval);
    qDebug() << "初始刷新间隔:" << initialInterval << "ms";

    // 添加定时器定期检查刷新间隔是否变化
    QTimer* intervalCheckTimer = new QTimer(this);
    connect(intervalCheckTimer, &QTimer::timeout, [=]() {
        int newInterval = selectedPlugin->refreshInterval();
        if (newInterval != timer->interval()) {
            timer->setInterval(newInterval);
            qDebug() << "刷新间隔已调整为:" << newInterval << "ms";
        }
    });
    intervalCheckTimer->start(2000); // 每2秒检查一次

    trayIcon->show();
    qDebug() << "Plugin path:" << pluginPath;
    qDebug() << "Loaded plugins count:" << plugins.count();
    qDebug() << "First plugin valid:" << (selectedPlugin != nullptr);
}

MainWindow::~MainWindow()
{
    delete ui;
}
