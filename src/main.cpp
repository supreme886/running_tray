#include <QApplication>
#include <QSystemTrayIcon>
#include <QDir>
#include <QDebug>
#include <QTimer>
#include <QPluginLoader>
#include <QMessageBox>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("Running Tray");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Running Tray");
    
    // 设置应用图标
    app.setWindowIcon(QIcon(":/resources/app_icon.ico"));

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, "Error", "System tray not available");
        return 1;
    }

    MainWindow w;
    w.show();

    return app.exec();
}

