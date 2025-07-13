#include "weatherplugin.h"
#include "sharedmenumanager.h"
#include "networkmanager.h"
#include "thememanager.h"  // 包含新的主题管理器

#include <QIcon>
#include <QDebug>
#include <QTimer>
#include <QMenu>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QStringList>
#include <QAction>
#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QFile>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QActionGroup>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QFile>

const QString apiKey = "fc94017ebd0c8ac93460f39f334e9637";

QSystemTrayIcon* WeatherPlugin::init() {
    trayIcon = new QSystemTrayIcon(this);
    trayMenu = new QMenu;
    
    // 添加插件特有菜单项
    QAction* aboutAction = new QAction("About Weather", this);
    connect(aboutAction, &QAction::triggered, [this](){
        QMessageBox::information(qApp->activeWindow(), "About", "Weather Plugin v1.0");
    });
    trayMenu->addAction(aboutAction);
    
    trayMenu->addSeparator();
    
    for (QAction* action : SharedMenuManager::instance().getSharedActions()) {
        trayMenu->addAction(action);
    }
    
    trayIcon->setContextMenu(trayMenu);

    loadWeatherConfig();
    
    // 使用新的主题管理器
    m_themeManager = new ThemeManager(this);
    connect(m_themeManager, &ThemeManager::themeChanged, this, &WeatherPlugin::onThemeChanged);
    m_themeManager->startMonitoring();
    
    iconUpdateTimer = new QTimer(this);
    connect(iconUpdateTimer, &QTimer::timeout, this, [this]() {
        if (m_animation && m_animation->totalFrame() > 0) {
            trayIcon->setIcon(updateIcon());
        }
    });

    reloadAnimation();

    weatherUpdateTimer = new QTimer(this);
    weatherUpdateTimer->setInterval(3600000); // 每小时更新一次
    connect(weatherUpdateTimer, &QTimer::timeout, this, &WeatherPlugin::fetchPublicIP);
    
    fetchPublicIP();
    
    trayIcon->show();

    return trayIcon;
}

void WeatherPlugin::stop() {
    qDebug() << Q_FUNC_INFO <<__LINE__;

    m_animation.reset();
    
    // 停止定时器
    if (iconUpdateTimer) {
        iconUpdateTimer->stop();
        delete iconUpdateTimer;
        iconUpdateTimer = nullptr;
    }
    
    // 停止天气更新定时器
    if (weatherUpdateTimer) {
        weatherUpdateTimer->stop();
        delete weatherUpdateTimer;
        weatherUpdateTimer = nullptr;
    }
    
    // 停止主题监控
    if (m_themeManager) {
        m_themeManager->stopMonitoring();
        delete m_themeManager;
        m_themeManager = nullptr;
    }

    if (trayIcon) {
        trayIcon->hide();
        delete trayIcon;
        trayIcon = nullptr;
        delete trayMenu;
        trayMenu = nullptr;
    }
}

void WeatherPlugin::setStatusCallback(std::function<void (int)> callback)
{

}

void WeatherPlugin::setIconSize(int size) {
    if (size > 0 && size <= 128) {
        iconSize = size;
        qDebug() << "Icon size set to:" << iconSize;

        if (trayIcon) {
            trayIcon->setIcon(updateIcon());
        }
    } else {
        qDebug() << "Invalid icon size:" << size << ". Size must be between 1 and 128.";
    }
}

int WeatherPlugin::getIconSize() const {
    return iconSize;
}

void WeatherPlugin::setAutoScaleIcon(bool enabled) {
    autoScaleIcon = enabled;
    qDebug() << "Auto scale icon:" << (enabled ? "enabled" : "disabled");
    
    if (trayIcon) {
        trayIcon->setIcon(updateIcon());
    }
}

bool WeatherPlugin::isAutoScaleEnabled() const {
    return autoScaleIcon;
}

bool WeatherPlugin::hasSettings() {
    return true;
}

QWidget* WeatherPlugin::createSettingsWidget() {
    QWidget* settingsWidget = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(settingsWidget);
    
    QGroupBox* sizeGroupBox = new QGroupBox("Icon Size");
    QVBoxLayout* sizeLayout = new QVBoxLayout(sizeGroupBox);
    
    QRadioButton* size16Btn = new QRadioButton("16x16 (Small)");
    QRadioButton* size24Btn = new QRadioButton("24x24 (Medium)");
    QRadioButton* size32Btn = new QRadioButton("32x32 (Large)");
    
    if (iconSize == 16) size16Btn->setChecked(true);
    else if (iconSize == 24) size24Btn->setChecked(true);
    else if (iconSize == 32) size32Btn->setChecked(true);
    
    // 连接信号
    connect(size16Btn, &QRadioButton::toggled, [this](bool checked) {
        if (checked) setIconSize(16);
    });
    connect(size24Btn, &QRadioButton::toggled, [this](bool checked) {
        if (checked) setIconSize(24);
    });
    connect(size32Btn, &QRadioButton::toggled, [this](bool checked) {
        if (checked) setIconSize(32);
    });
    
    sizeLayout->addWidget(size16Btn);
    sizeLayout->addWidget(size24Btn);
    sizeLayout->addWidget(size32Btn);
    
    QCheckBox* autoScaleBox = new QCheckBox("Auto Scale (DPI Aware)");
    autoScaleBox->setChecked(autoScaleIcon);
    connect(autoScaleBox, &QCheckBox::toggled, this, &WeatherPlugin::setAutoScaleIcon);
    sizeLayout->addWidget(autoScaleBox);
    
    mainLayout->addWidget(sizeGroupBox);
    mainLayout->addStretch();
    
    return settingsWidget;
}

void WeatherPlugin::reloadAnimation(const QString& animationFileName) {
    if (iconUpdateTimer) {
        iconUpdateTimer->stop();
    }
    
    {
        QMutexLocker locker(&m_animationMutex);
        m_iconCurIndex = 0;
        
        QString fileName = animationFileName.isEmpty() ? "clear-day.json" : animationFileName;
        QString animationPath = QString(":/res/%1").arg(fileName);
        
        QFile file(animationPath);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray jsonData = file.readAll();
            
            auto newAnimation = rlottie::Animation::loadFromData(jsonData.constData(), "weather_anim");
            if (newAnimation) {
                m_animation.reset();
                m_animation = std::move(newAnimation);
                qDebug() << "Loaded weather animation:" << animationPath;
            }
        } else {
            qDebug() << "Failed to load animation file:" << animationPath;
        }
    }
    
    if (trayIcon) {
        trayIcon->setIcon(updateIcon());
    }
    
    if (iconUpdateTimer) {
        iconUpdateTimer->start((int)m_animation->duration());
    }
}

QIcon WeatherPlugin::updateIcon() {
    QMutexLocker locker(&m_animationMutex);
    
    if (!m_animation) {
        qWarning() << "No animation loaded";
        return QIcon();
    }

    size_t w = 0, h = 0;
    m_animation->size(w, h);
    if (w <= 0 || h <= 0) {
        qWarning() << "Invalid animation size";
        return QIcon();
    }

    const int targetSize = iconSize;
    const QSize renderSize(targetSize, targetSize);

    const int totalFrames = m_animation->totalFrame();
    if (totalFrames <= 0) {
        qWarning() << "Animation has no frames";
        return QIcon();
    }

    m_iconCurIndex = (m_iconCurIndex + 1) % totalFrames;

    QImage image(renderSize, QImage::Format_ARGB32);
    Surface surface {
        reinterpret_cast<uint32_t*>(image.bits()),
        static_cast<size_t>(image.width()),
        static_cast<size_t>(image.height()),
        static_cast<size_t>(image.bytesPerLine())
    };
    qWarning() << m_iconCurIndex <<"No animation loaded";
    m_animation->renderSync(m_iconCurIndex, surface, true);

    QIcon resultIcon(QPixmap::fromImage(image));
    emit iconUpdated(resultIcon);
    return resultIcon;
}

void WeatherPlugin::onThemeChanged(bool isDarkTheme) {
    if (isDarkTheme != currentIsDarkTheme) {
        qDebug() << "Weather Plugin: Theme changed from" << (currentIsDarkTheme ? "dark" : "light") 
                 << "to" << (isDarkTheme ? "dark" : "light");
        
        currentIsDarkTheme = isDarkTheme;
        updateIconPathsForTheme();
        
        if (trayIcon) {
            trayIcon->setIcon(updateIcon());
        }
    }
}

void WeatherPlugin::updateIconPathsForTheme() {    
    qDebug() << "Weather Plugin: Updated for" << (currentIsDarkTheme ? "dark" : "light") << "theme";
}

void WeatherPlugin::fetchPublicIP()
{
    // //传递对象实例而非函数指针
    // CommonNetworkManager::instance()->getAsync(QUrl("https://api.ipify.org?format=json"), [this](const QByteArray& array){
    //     QJsonDocument doc = QJsonDocument::fromJson(array);
    //     QJsonObject ip = doc.object();
    //     qDebug() << Q_FUNC_INFO << __LINE__;
    //     if (!ip.isEmpty()) {
    //         publicIp = ip[QLatin1String("ip")].toString();
    //         if (!publicIp.isEmpty()) {
    //             qDebug() << Q_FUNC_INFO << publicIp;
    //             fetchLocationByIP("45.135.228.108");
    //         }
    //     }
    // });
    fetchLocationByIP("114.247.50.2");
}

void WeatherPlugin::fetchLocationByIP(const QString& ip) {
    CommonNetworkManager::instance()->getAsync(
        QUrl(QString("https://restapi.amap.com/v3/ip?ip=%1&key=%2").arg(ip).arg(apiKey)),
        [this](const QByteArray& response) {
            QJsonObject obj = QJsonDocument::fromJson(response).object();
            qDebug() << Q_FUNC_INFO << obj << __LINE__;
            if (!obj.isEmpty()) {
                QString city = obj["adcode"].toString();
                if (!city.isEmpty()) {
                    qDebug() << Q_FUNC_INFO << city << __LINE__;
                    fetchWeatherData(city);
                }
            }
        }
    );
}

void WeatherPlugin::loadWeatherConfig() {
    QFile configFile(":/weather_config.json");
    if (configFile.open(QIODevice::ReadOnly)) {
        QByteArray configData = configFile.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(configData);
        weatherConfig = doc.object();
    }
}

QString WeatherPlugin::getAnimationFileForWeather(const QString& weatherDesc) {
    QJsonObject mapping = weatherConfig["weatherMapping"].toObject();
    QJsonObject weatherInfo = mapping.value(weatherDesc).toObject();
    return weatherInfo.contains("lottieFile") ? weatherInfo.value("lottieFile").toString() : "clear-day.json";
}

void WeatherPlugin::fetchWeatherData(const QString& cityCode) {
    QString url = QString("https://restapi.amap.com/v3/weather/weatherInfo?city=%1&key=%2")
                  .arg(cityCode).arg(apiKey);
    
    CommonNetworkManager::instance()->getAsync(
        QUrl(url),
        [this](const QByteArray& response) {
            QJsonObject obj = QJsonDocument::fromJson(response).object();
            qDebug() << Q_FUNC_INFO << obj << __LINE__;
            
            if (obj["status"].toString() == "1") {
                QJsonArray lives = obj["lives"].toArray();
                if (!lives.isEmpty()) {
                    QJsonObject weather = lives[0].toObject();
                    QString weatherDesc = weather["weather"].toString();
                    QString temperature = weather["temperature"].toString();
                    
                    qDebug() << "Weather:" << weatherDesc << "Temperature:" << temperature;
                    
                    // 根据天气描述选择对应的动画文件
                    QString animationFile = getAnimationFileForWeather(weatherDesc);
                    reloadAnimation("cloudy.json");
                    
                    // 更新托盘图标提示
                    if (trayIcon) {
                        QString tooltip = QString("%1 %2°C").arg(weatherDesc).arg(temperature);
                        trayIcon->setToolTip(tooltip);
                    }
                }
            }
        }
    );
}
