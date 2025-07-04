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

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, "Error", "System tray not available");
        return 1;
    }

    MainWindow w;
    w.show();

    return app.exec();
}

