#include <QApplication>
#include <QSystemTrayIcon>
#include <QDir>
#include <QDebug>
#include <QTimer>
#include <QPluginLoader>
#include <QMessageBox>
#include <QFile>
#include <QTextCodec>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>

#include "mainwindow.h"

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

    QString logDir = QCoreApplication::applicationDirPath() + "/logs";
    QDir().mkpath(logDir);

    QFile file(logDir + "/app.log");
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << logMessage;
        file.close();
    }

    QTextStream console(stdout);
    // 设置控制台输出编码为GBK以解决中文乱码问题
#ifdef Q_OS_WIN
    console.setCodec("GBK");
#else
    // 非Windows系统使用UTF-8
    console.setCodec("UTF-8");
#endif
    console << logMessage;

    mutex.unlock();
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(customMessageHandler);

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

