#include "appbskyvideouploadvideoex.h"
#include "atprotocol/com/atproto/server/comatprotoservergetserviceauth.h"

#include <QFileInfo>

using AtProtocolInterface::ComAtprotoServerGetServiceAuth;

namespace AtProtocolInterface {

AppBskyVideoUploadVideoEx::AppBskyVideoUploadVideoEx(QObject *parent)
    : AppBskyVideoUploadVideo { parent }
{
}

void AppBskyVideoUploadVideoEx::uploadVideo(const QString &path)
{
    QFileInfo info(path);
    if (!info.exists()) {
        emit finished(false);
        return;
    }
    getServiceAuth([=](const QString &token) {
        if (token.isEmpty()) {
            qDebug().noquote() << "Can't get service token.";
            emit finished(false);
        } else {
            qDebug() << "Upload process and job status check";
            emit finished(false); // for debug
        }
    });
}

void AppBskyVideoUploadVideoEx::getServiceAuth(std::function<void(const QString &)> callback)
{
    ComAtprotoServerGetServiceAuth *auth = new ComAtprotoServerGetServiceAuth(this);
    connect(auth, &ComAtprotoServerGetServiceAuth::finished, [=](bool success) {
        QString token;
        if (success) {
            token = auth->token();
        }
        callback(token);
        auth->deleteLater();
    });
    auth->setAccount(account());
    auth->getServiceAuth("did:web:video.bsky.app", 0, "app.bsky.video.uploadVideo");
}
}
