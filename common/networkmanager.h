#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <functional>
#include <QUrl>

#include "common_exports.h"

class COMMON_LIBRARY_EXPORT CommonNetworkManager : public QObject
{
    Q_OBJECT
public:
    static CommonNetworkManager* instance();
    ~CommonNetworkManager() override;

    // 异步GET请求
    void getAsync(const QUrl& url,
                 std::function<void(const QByteArray&)> onSuccess,
                 std::function<void(QNetworkReply::NetworkError)> onError = nullptr);

    // 异步POST请求
    void postAsync(const QUrl& url, const QByteArray& data,
                  std::function<void(const QByteArray&)> onSuccess,
                  std::function<void(QNetworkReply::NetworkError)> onError = nullptr);

private:
    explicit CommonNetworkManager(QObject* parent = nullptr);
    QNetworkAccessManager* m_networkManager;

    // 禁止拷贝
    CommonNetworkManager(const CommonNetworkManager&) = delete;
    CommonNetworkManager& operator=(const CommonNetworkManager&) = delete;
};

#endif // NETWORKMANAGER_H
