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

#include "pluginmanager.h"
#include "interface/itrayloadplugin.h"
#include "sharedmenumanager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 设置窗口标题和大小
    setWindowTitle("Plugin Tray Manager");
    resize(800, 600);

    // 创建中央部件和布局
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // 创建插件多宫格容器
    QWidget* pluginContainer = new QWidget(this);
    QGridLayout* gridLayout = new QGridLayout(pluginContainer);
    gridLayout->setSpacing(20);
    gridLayout->setContentsMargins(20, 20, 20, 20);
    
    mainLayout->addWidget(pluginContainer);

    // 获取插件目录路径
    QString pluginPath;
    QDir baseDir(QCoreApplication::applicationDirPath());

    if (QDir(baseDir.absoluteFilePath("plugins")).exists()) {
        pluginPath = baseDir.absoluteFilePath("plugins");
    } else if (QDir(baseDir.absoluteFilePath("../plugins")).exists()) {
        pluginPath = baseDir.absoluteFilePath("../plugins");
    } else {
        QMessageBox::warning(nullptr, "Plugin Not Found", "Plugin directory not found");
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
            // 创建插件卡片
            QFrame* card = new QFrame();
            card->setFrameShape(QFrame::StyledPanel);
            card->setStyleSheet("QFrame { border: 1px solid #ccc; border-radius: 8px; padding: 15px; background-color: #f5f5f5; }");
            card->setFixedSize(200, 250);
            
            QVBoxLayout* cardLayout = new QVBoxLayout(card);
            cardLayout->setAlignment(Qt::AlignCenter);
            cardLayout->setSpacing(15);

            // 插件图标
            QLabel* iconLabel = new QLabel();
            iconLabel->setAlignment(Qt::AlignCenter);
            iconLabel->setFixedSize(100, 100);
            iconLabel->setStyleSheet("background-color: white; border-radius: 50px; padding: 5px;");
            cardLayout->addWidget(iconLabel);

            // 插件名称
            QLabel* nameLabel = new QLabel(plugin->name());
            nameLabel->setAlignment(Qt::AlignCenter);
            nameLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
            cardLayout->addWidget(nameLabel);

            // 启动/停止按钮
            QPushButton* controlButton = new QPushButton("Start");
            controlButton->setStyleSheet("padding: 8px; font-size: 14px;");
            cardLayout->addWidget(controlButton);

            // 添加到网格布局
            gridLayout->addWidget(card, row, col);

            // 连接按钮信号 - 控制插件启停
            connect(controlButton, &QPushButton::clicked, this, [&pluginEntry, controlButton]() {
                if (pluginEntry.is_loaded) {
                    PluginManager::instance().stopPlugin(pluginEntry);
                    controlButton->setText("Start");
                } else {
                    PluginManager::instance().startPlugin(pluginEntry);
                    controlButton->setText("Stop");
                }
            });

            // 连接图标更新信号 - 实时显示动态图标
            connect(plugin, &ITrayLoadPlugin::iconUpdated, this, [iconLabel](const QIcon& icon) {
                iconLabel->setPixmap(icon.pixmap(90, 90));
            });

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
