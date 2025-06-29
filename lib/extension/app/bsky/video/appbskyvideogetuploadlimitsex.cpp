#include "appbskyvideogetuploadlimitsex.h"

#include "atprotocol/app/bsky/video/appbskyvideogetuploadlimits.h"

#include <QFileInfo>

using AtProtocolInterface::AppBskyVideoGetUploadLimits;

namespace AtProtocolInterface {

AppBskyVideoGetUploadLimitsEx::AppBskyVideoGetUploadLimitsEx(QObject *parent)
    : AppBskyVideoGetUploadLimits { parent }
{
}

void AppBskyVideoGetUploadLimitsEx::canUpload(const QString &path)
{
    QFileInfo info(path);
    if (!info.exists()) {
        emit finished(false);
        return;
    }

    AppBskyVideoGetUploadLimits *limit = new AppBskyVideoGetUploadLimits(this);
    connect(limit, &AppBskyVideoGetUploadLimits::finished, [=](bool success) {
        bool can = false;
        if (success) {
            can = (limit->canUpload() && (limit->remainingDailyBytes() - info.size()) >= 0);
            qDebug().noquote() << "Video upload limit:\n  canUpload=" << limit->canUpload()
                               << "\n  remainingDailyBytes=" << limit->remainingDailyBytes()
                               << "\n  remainingDailyVideos=" << limit->remainingDailyVideos()
                               << "\n  message=" << limit->message() << "\n--\n  can=" << can
                               << "\n  remain bytes:"
                               << (limit->remainingDailyBytes() - info.size());
        } else {
        }
        emit finished(can);
        limit->deleteLater();
    });
    limit->setAccount(account());
    limit->getUploadLimits();
}
}
