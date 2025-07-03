#include "runningcatplugin.h"
#include <QIcon>
#include <QDebug>

void RunningCatPlugin::init() {
    iconPaths = {
        ":/dark_cat_1.ico",  // 修改为.ico后缀
        ":/dark_cat_2.ico",
        ":/dark_cat_3.ico",
        ":/dark_cat_4.ico",
        ":/dark_cat_5.ico"
    };
    currentIndex = 0;
}

QIcon RunningCatPlugin::updateIcon() {
    if (iconPaths.isEmpty()) {
        qDebug() << "No icon paths available";
        return QIcon();
    }
    qDebug() << "Loading icon from:" << iconPaths[currentIndex];
    QIcon icon(iconPaths[currentIndex]);
    qDebug() << "Icon is null:" << icon.isNull();
    currentIndex = (currentIndex + 1) % iconPaths.size();
    return icon;
}

