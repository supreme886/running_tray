#include <QApplication>
#include <QSystemTrayIcon>
#include <QDir>
#include <QDebug>
#include <QTimer>
#include <QPluginLoader>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>
#include <QStandardPaths>

#include "mainwindow.h"

#if 1
// #include <QTextCodec>
void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    static QMutex mutex;
    mutex.lock();

    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString logLevel;

    switch (type) {
    case QtDebugMsg:     logLevel = "[DEBUG]"; break;
    case QtInfoMsg:      logLevel = "[INFO]";  break;
    case QtWarningMsg:   logLevel = "[WARN]";  break;
    case QtCriticalMsg:  logLevel = "[ERROR]"; break;
    case QtFatalMsg:     logLevel = "[FATAL]"; break;
    default:             logLevel = "[UNKNOWN]";
    }

    QString logMessage = QString("%1 %2 %3 (%4:%5): %6\n")
                             .arg(time)
                             .arg(logLevel)
                             .arg(context.category)
                             .arg(context.file)
                             .arg(context.line)
                             .arg(msg);

    // Use a more reliable log directory path
    QString logDir;
#ifdef Q_OS_WIN
    logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs";
#elif defined(Q_OS_MAC)
    logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs";
#else
    logDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.running_tray/logs";
#endif
    
    // Ensure directory exists
    QDir dir;
    if (!dir.mkpath(logDir)) {
        // Fallback to temp directory if we can't create in preferred location
        logDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/running_tray_logs";
        dir.mkpath(logDir);
    }

    QString logFilePath = logDir + "/app.log";
    QFile file(logFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << logMessage;
        file.close();
    } else {
        // If we still can't write, output error to console
        qDebug() << "Failed to write to log file:" << logFilePath << "Error:" << file.errorString();
    }

    QTextStream console(stdout);
    // Set console output encoding to GBK to solve Chinese garbled characters on Windows
#ifdef Q_OS_WIN
    // console.setCodec("GBK");
#else
    // Use UTF-8 for non-Windows systems
    console.setCodec("UTF-8");
#endif
    console << logMessage;

    mutex.unlock();
}
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    // qInstallMessageHandler(customMessageHandler);
    
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

