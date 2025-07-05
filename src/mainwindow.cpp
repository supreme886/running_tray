#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QSystemTrayIcon>
#include <QDir>
#include <QMessageBox>
#include <QTimer>
#include <QStyle>
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QFrame>
#include <QCloseEvent>
#include <QApplication>

#include "pluginmanager.h"
#include "interface/itrayloadplugin.h"
#include "sharedmenumanager.h"
#include "plugincardwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 设置窗口标题和大小
    setWindowTitle("Running-Tray");
    resize(800, 600);

    // 注册显示主窗口的回调到共享菜单管理器
    SharedMenuManager::instance().setShowMainWindowCallback([this]() {
        showMainWindow();
    });

    // 创建中央部件和布局
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // 创建插件状态表格
    QWidget* pluginContainer = new QWidget(this);
    QGridLayout* gridLayout = new QGridLayout(pluginContainer);
    gridLayout->setSpacing(20);
    gridLayout->setContentsMargins(20, 20, 20, 20);

    mainLayout->addWidget(pluginContainer);

    // 获取插件目录路径
    QString pluginPath;
    QDir baseDir(QCoreApplication::applicationDirPath());

#if defined(Q_OS_MAC)
    // 在macOS上，应用程序通常打包在.app目录中
    // 插件位于 .app/Contents/PlugIns/
    baseDir.cdUp();
    baseDir.cdUp();
    baseDir.cdUp();
    baseDir.cd("plugins");
    pluginPath = baseDir.absolutePath();
#else
    // 对于其他平台（Windows, Linux），插件通常在可执行文件旁边的plugins目录中
    if (QDir(baseDir.absoluteFilePath("plugins")).exists()) {
        pluginPath = baseDir.absoluteFilePath("plugins");
    } else if (QDir(baseDir.absoluteFilePath("../plugins")).exists()) {
        pluginPath = baseDir.absoluteFilePath("../plugins");
    }
#endif
    qDebug() << Q_FUNC_INFO << baseDir;
    if (pluginPath.isEmpty() || !QDir(pluginPath).exists()) {
        QMessageBox::warning(nullptr, "Plugin Not Found", "Plugin directory not found: " + pluginPath);
        return;
    }

    // 加载插件
    auto plugins = PluginManager::instance().loadPlugins(pluginPath);
    if (plugins.isEmpty()) {
        QMessageBox::information(nullptr, "No Plugins", "No plugins loaded");
        return;
    }

    // 初始化共享菜单管理器
    SharedMenuManager::instance();

    // 多宫格布局参数
    const int maxCols = 3;  // 每行显示3个插件
    int row = 0;
    int col = 0;

    // 为每个插件创建卡片
    for (auto& pluginEntry : plugins) {
        ITrayLoadPlugin* plugin = pluginEntry.plugin;
        if (plugin) {
            // 打印插件元数据
            QJsonObject metaData = pluginEntry.loader->metaData();
            qDebug() << "Plugin MetaData:" << metaData;

            plugin->init();

            // 创建插件卡片（使用新控件）
            PluginCardWidget* card = new PluginCardWidget();
            card->setPluginName(plugin->name());
            card->setRunningState(pluginEntry.is_loaded);

            // 添加到网格布局
            gridLayout->addWidget(card, row, col);

            // 连接按钮信号 - 控制插件启停
            connect(card, &PluginCardWidget::controlClicked, this, [&pluginEntry](bool isRunning) {
                if (isRunning) {
                    PluginManager::instance().startPlugin(pluginEntry);
                } else {
                    PluginManager::instance().stopPlugin(pluginEntry);
                }
            });

            // 连接图标更新信号 - 实时显示动态图标
            connect(plugin, &ITrayLoadPlugin::iconUpdated, card, &PluginCardWidget::setPluginIcon);

            // 更新网格位置
            col++;
            if (col >= maxCols) {
                col = 0;
                row++;
            }
        }
    }

    qDebug() << "Plugin path:" << pluginPath;
    qDebug() << "Loaded plugins count:" << plugins.count();

    // 显示主窗口
    show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 关闭窗口时隐藏而不是退出程序
    hide();
    event->ignore();
}

void MainWindow::showMainWindow()
{
    // 显示并激活主窗口
    show();
    raise();
    activateWindow();
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);
}
