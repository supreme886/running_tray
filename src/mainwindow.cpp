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
#include <QStackedWidget>

#include "pluginmanager.h"
#include "flowlayout.h"
#include "interface/itrayloadplugin.h"
#include "sharedmenumanager.h"
#include "plugincardwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // 设置窗口图标
    setWindowIcon(QIcon(":/resources/app_icon.ico"));
    
    // 设置窗口标题和大小
    setWindowTitle("Running-Tray");
    resize(800, 600);

    // 注册显示主窗口的回调到共享菜单管理器
    SharedMenuManager::instance().setShowMainWindowCallback([this]() {
        showMainWindow();
    });

    // 创建中央部件和布局
    // 移除原来的centralWidget，改用QStackedWidget
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);
    
    // 创建插件容器（保存引用）
    pluginContainer = new QWidget(this);
    FlowLayout* flowLayout = new FlowLayout(pluginContainer, 15, 15, 15);
    flowLayout->setSpacing(30);

    // 将插件容器添加到堆叠窗口
    stackedWidget->addWidget(pluginContainer);

    // 获取插件目录路径
    QString pluginPath;
    QDir baseDir(QCoreApplication::applicationDirPath());

#if defined(Q_OS_MAC)
    // macOS：优先检查标准的PlugIns目录
    QDir pluginsDir = baseDir;
    pluginsDir.cdUp(); // 从MacOS到Contents
    if (pluginsDir.cd("PlugIns")) {
        pluginPath = pluginsDir.absolutePath();
    } else {
        baseDir.cdUp();
        baseDir.cdUp();
        baseDir.cdUp();
        if (baseDir.cd("plugins")) {
            pluginPath = baseDir.absolutePath();
        }
    }
#else
    // 其他平台的逻辑保持不变
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

            // 从嵌套的MetaData对象中获取描述信息
            if (metaData.contains("MetaData") && metaData["MetaData"].isObject()) {
                QJsonObject metaDataObj = metaData["MetaData"].toObject();
                if (metaDataObj.contains("Description")) {
                    card->setPluginDescription(metaDataObj["Description"].toString());
                }
            }

            // 添加到流式布局
            flowLayout->addWidget(card);
            
            // 连接按钮信号 - 控制插件启停
            connect(card, &PluginCardWidget::controlClicked, this, [pluginEntry](bool isRunning) mutable {
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
            
            // 设置是否显示设置按钮
            card->setHasSettings(plugin->hasSettings());
            
            // 连接设置按钮信号
            connect(card, &PluginCardWidget::settingsClicked, this, [this, plugin]() {
                showPluginSettings(plugin);
            });
        }
    }
    
    // 添加"敬请期待"空卡片
    EmptyCardWidget* emptyCard = new EmptyCardWidget();
    flowLayout->addWidget(emptyCard);
}

// 显示插件设置界面
void MainWindow::showPluginSettings(ITrayLoadPlugin* plugin) {
    if (!plugin || !plugin->hasSettings()) return;
    
    currentPlugin = plugin;
    
    // 创建新的插件设置窗口
    currentSettingsWidget = new PluginSettingsWidget(plugin, this);
    
    // 连接信号槽
    connect(currentSettingsWidget, &PluginSettingsWidget::saveRequested, 
            this, &MainWindow::onSettingsSaved);
    connect(currentSettingsWidget, &PluginSettingsWidget::cancelRequested, 
            this, &MainWindow::onSettingsCancelled);
    
    // 将设置容器添加到堆叠窗口并切换
    stackedWidget->addWidget(currentSettingsWidget);
    stackedWidget->setCurrentWidget(currentSettingsWidget);
}

// 保存设置并返回插件列表
void MainWindow::onSettingsSaved() {
    if (currentPlugin) {
        currentPlugin->saveSettings();
    }
    // 切换回插件容器页面
    stackedWidget->setCurrentWidget(pluginContainer);
    // 清理设置页面
    if (currentSettingsWidget) {
        stackedWidget->removeWidget(currentSettingsWidget);
        delete currentSettingsWidget;
        currentSettingsWidget = nullptr;
    }
    currentPlugin = nullptr;
}

void MainWindow::onSettingsCancelled() {
    if (currentPlugin) {
        currentPlugin->cancelSettings();
    }
    // 切换回插件容器页面
    stackedWidget->setCurrentWidget(pluginContainer);
    // 清理设置页面
    if (currentSettingsWidget) {
        stackedWidget->removeWidget(currentSettingsWidget);
        delete currentSettingsWidget;
        currentSettingsWidget = nullptr;
    }
    currentPlugin = nullptr;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    hide();
    event->ignore();
}

void MainWindow::showMainWindow()
{
    show();
    raise();
    activateWindow();
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);
}

void MainWindow::showEvent(QShowEvent *event)
{
    if (currentSettingsWidget) {
        onSettingsCancelled();
    }
    QMainWindow::showEvent(event);
}
