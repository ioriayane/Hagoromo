#include "appbskyvideogetuploadlimitsex.h"

#include "atprotocol/app/bsky/video/appbskyvideogetuploadlimits.h"
#include "atprotocol/com/atproto/server/comatprotoservergetserviceauth.h"

#include <QFileInfo>

using AtProtocolInterface::AppBskyVideoGetUploadLimits;
using AtProtocolInterface::ComAtprotoServerGetServiceAuth;

namespace AtProtocolInterface {

AppBskyVideoGetUploadLimitsEx::AppBskyVideoGetUploadLimitsEx(QObject *parent)
    : AppBskyVideoGetUploadLimits { parent }, m_endpoint("https://video.bsky.app")
{
}

void AppBskyVideoGetUploadLimitsEx::canUpload(const QString &path)
{
    QFileInfo info(path);
    if (!info.exists()) {
        emit finished(false);
        return;
    }

    getServiceAuth([=](const QString &token) {
        if (token.isEmpty()) {
            emit finished(false);
        } else {
            AppBskyVideoGetUploadLimits *limit = new AppBskyVideoGetUploadLimits(this);
            connect(limit, &AppBskyVideoGetUploadLimits::finished, [=](bool success) {
                bool can = false;
                if (success) {
                    can = (limit->canUpload() && (limit->remainingDailyBytes() - info.size()) >= 0
                           && limit->remainingDailyVideos() > 0);
                    qDebug().noquote()
                            << "Video upload limit:\n  canUpload=" << limit->canUpload()
                            << "\n  remainingDailyBytes=" << limit->remainingDailyBytes()
                            << "\n  remainingDailyVideos=" << limit->remainingDailyVideos()
                            << "\n  message=" << limit->message() << "\n--\n  can=" << can
                            << "\n  remain bytes:" << (limit->remainingDailyBytes() - info.size());
                } else {
                }
                emit finished(can);
                limit->deleteLater();
            });
            AtProtocolInterface::AccountData a = account();
            a.accessJwt = token;
            limit->setAccount(a);
            limit->getUploadLimits();
        }
    });
}

void AppBskyVideoGetUploadLimitsEx::getServiceAuth(std::function<void(const QString &)> callback)
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
    auth->getServiceAuth("did:web:video.bsky.app", 0, "app.bsky.video.getUploadLimits");
}

void AppBskyVideoGetUploadLimitsEx::setEndpoint(const QString &newEndpoint)
{
    if (newEndpoint.isEmpty())
        return;
    m_endpoint = newEndpoint;
}
}
