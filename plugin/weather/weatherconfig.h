#ifndef WEATHERCONFIG_H
#define WEATHERCONFIG_H

#include <QObject>
#include <QJsonObject>
#include <QString>

class WeatherConfig : public QObject
{
    Q_OBJECT
public:
    explicit WeatherConfig(QObject *parent = nullptr);

    QString apiKey() const;
    void setApiKey(const QString &newApiKey);

    bool load();
    bool save();

private:
    QString m_configPath;
    QJsonObject m_configData;
};

#endif // WEATHERCONFIG_H