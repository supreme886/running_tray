#include "networkmanager.h"
#include <QNetworkRequest>
#include <QDebug>

CommonNetworkManager* CommonNetworkManager::instance()
{
    static CommonNetworkManager instance;
    return &instance;
}

CommonNetworkManager::CommonNetworkManager(QObject* parent)
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this))
{
}

CommonNetworkManager::~CommonNetworkManager()
{
    delete m_networkManager;
}

void CommonNetworkManager::getAsync(const QUrl& url,
                                   std::function<void(const QByteArray&)> onSuccess,
                                   std::function<void(QNetworkReply::NetworkError)> onError)
{
    if (!url.isValid()) {
        qWarning() << "Invalid URL:" << url.toString();
        if (onError) onError(QNetworkReply::ProtocolInvalidOperationError);
        return;
    }

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [reply, onSuccess, onError]() {
        if (reply->error() == QNetworkReply::NoError) {
            if (onSuccess) onSuccess(reply->readAll());
        } else {
            qWarning() << "Network error:" << reply->errorString();
            if (onError) onError(reply->error());
        }
        reply->deleteLater();
    });
}

void CommonNetworkManager::postAsync(const QUrl& url, const QByteArray& data,
                                    std::function<void(const QByteArray&)> onSuccess,
                                    std::function<void(QNetworkReply::NetworkError)> onError)
{
    if (!url.isValid()) {
        qWarning() << "Invalid URL:" << url.toString();
        if (onError) onError(QNetworkReply::ProtocolInvalidOperationError);
        return;
    }

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = m_networkManager->post(request, data);

    connect(reply, &QNetworkReply::finished, this, [reply, onSuccess, onError]() {
        if (reply->error() == QNetworkReply::NoError) {
            if (onSuccess) onSuccess(reply->readAll());
        } else {
            qWarning() << "Network error:" << reply->errorString();
            if (onError) onError(reply->error());
        }
        reply->deleteLater();
    });
}