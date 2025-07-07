#include <QApplication>
#include <QSystemTrayIcon>
#include <QDir>
#include <QDebug>
#include <QTimer>
#include <QPluginLoader>
#include <QMessageBox>

#include "mainwindow.h"
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("Running Tray");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Running Tray");
    
    // 设置应用图标
    app.setWindowIcon(QIcon(":/resources/app_icon.ico"));

    // 加载QSS样式表
    QFile styleFile(":/resources/styles.qss");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        app.setStyleSheet(styleSheet);
        styleFile.close();
    } else {
        QMessageBox::warning(nullptr, "样式加载失败", 
                            QString("无法加载样式文件: %1").arg(styleFile.errorString()));
    }

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, "Error", "System tray not available");
        return 1;
    }

    MainWindow w;
    w.show();

    return app.exec();
}

