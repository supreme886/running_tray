#include "autostartmanager.h"

#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>
#include <QDebug>

#ifdef Q_OS_WIN
#include <QSettings>
#elif defined(Q_OS_MACOS)
#include <QProcess>
#elif defined(Q_OS_LINUX)
#include <QFile>
#include <QTextStream>
#endif

AutoStartManager::AutoStartManager(QObject* parent) : QObject(parent) {
}

AutoStartManager::~AutoStartManager() {
}

AutoStartManager& AutoStartManager::instance() {
    static AutoStartManager instance;
    return instance;
}

bool AutoStartManager::setAutoStart(bool enabled) {
#ifdef Q_OS_WIN
    return setAutoStartWindows(enabled);
#elif defined(Q_OS_MACOS)
    return setAutoStartMacOS(enabled);
#elif defined(Q_OS_LINUX)
    return setAutoStartLinux(enabled);
#else
    qWarning() << "Auto start not supported on this platform";
    return false;
#endif
}

bool AutoStartManager::isAutoStartEnabled() const {
#ifdef Q_OS_WIN
    return isAutoStartEnabledWindows();
#elif defined(Q_OS_MACOS)
    return isAutoStartEnabledMacOS();
#elif defined(Q_OS_LINUX)
    return isAutoStartEnabledLinux();
#else
    return false;
#endif
}

QString AutoStartManager::getApplicationName() const {
    return QApplication::applicationName().isEmpty() ? "Running-Tray" : QApplication::applicationName();
}

QString AutoStartManager::getApplicationPath() const {
    return QApplication::applicationFilePath();
}

#ifdef Q_OS_WIN
bool AutoStartManager::setAutoStartWindows(bool enabled) {
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    QString appName = getApplicationName();
    
    if (enabled) {
        QString appPath = getApplicationPath();
        settings.setValue(appName, QDir::toNativeSeparators(appPath));
        qDebug() << "Windows auto start enabled for:" << appName << "at" << appPath;
    } else {
        settings.remove(appName);
        qDebug() << "Windows auto start disabled for:" << appName;
    }
    
    settings.sync();
    return settings.status() == QSettings::NoError;
}

bool AutoStartManager::isAutoStartEnabledWindows() const {
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    QString appName = getApplicationName();
    return settings.contains(appName);
}
#endif

#ifdef Q_OS_MACOS
bool AutoStartManager::setAutoStartMacOS(bool enabled) {
    QString appName = getApplicationName();
    QString plistPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + 
                       "/Library/LaunchAgents/com." + appName.toLower() + ".plist";
    
    if (enabled) {
        QString appPath = getApplicationPath();
        
        // 创建 plist 文件内容
        QString plistContent = QString(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
            "<plist version=\"1.0\">\n"
            "<dict>\n"
            "    <key>Label</key>\n"
            "    <string>com.%1</string>\n"
            "    <key>ProgramArguments</key>\n"
            "    <array>\n"
            "        <string>%2</string>\n"
            "    </array>\n"
            "    <key>RunAtLoad</key>\n"
            "    <true/>\n"
            "    <key>KeepAlive</key>\n"
            "    <false/>\n"
            "</dict>\n"
            "</plist>\n"
        ).arg(appName.toLower(), appPath);
        
        QFile file(plistPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << plistContent;
            file.close();
            qDebug() << "macOS auto start enabled, plist created at:" << plistPath;
            return true;
        } else {
            qWarning() << "Failed to create plist file:" << plistPath;
            return false;
        }
    } else {
        QFile file(plistPath);
        if (file.exists()) {
            bool removed = file.remove();
            qDebug() << "macOS auto start disabled, plist removed:" << removed;
            return removed;
        }
        return true;
    }
}

bool AutoStartManager::isAutoStartEnabledMacOS() const {
    QString appName = getApplicationName();
    QString plistPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + 
                       "/Library/LaunchAgents/com." + appName.toLower() + ".plist";
    return QFile::exists(plistPath);
}
#endif

#ifdef Q_OS_LINUX
bool AutoStartManager::setAutoStartLinux(bool enabled) {
    QString appName = getApplicationName();
    QString autostartDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/autostart";
    QString desktopFilePath = autostartDir + "/" + appName.toLower() + ".desktop";
    
    // 确保 autostart 目录存在
    QDir dir;
    if (!dir.exists(autostartDir)) {
        dir.mkpath(autostartDir);
    }
    
    if (enabled) {
        QString appPath = getApplicationPath();
        
        // 创建 .desktop 文件内容
        QString desktopContent = QString(
            "[Desktop Entry]\n"
            "Type=Application\n"
            "Name=%1\n"
            "Exec=%2\n"
            "Hidden=false\n"
            "NoDisplay=false\n"
            "X-GNOME-Autostart-enabled=true\n"
            "Comment=Running Tray Application\n"
        ).arg(appName, appPath);
        
        QFile file(desktopFilePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << desktopContent;
            file.close();
            qDebug() << "Linux auto start enabled, desktop file created at:" << desktopFilePath;
            return true;
        } else {
            qWarning() << "Failed to create desktop file:" << desktopFilePath;
            return false;
        }
    } else {
        QFile file(desktopFilePath);
        if (file.exists()) {
            bool removed = file.remove();
            qDebug() << "Linux auto start disabled, desktop file removed:" << removed;
            return removed;
        }
        return true;
    }
}

bool AutoStartManager::isAutoStartEnabledLinux() const {
    QString appName = getApplicationName();
    QString autostartDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/autostart";
    QString desktopFilePath = autostartDir + "/" + appName.toLower() + ".desktop";
    return QFile::exists(desktopFilePath);
}
#endif