#include "appbskyvideogetuploadlimits.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyVideoGetUploadLimits::AppBskyVideoGetUploadLimits(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyVideoGetUploadLimits::getUploadLimits()
{
    QUrlQuery url_query;

    get(QStringLiteral("xrpc/app.bsky.video.getUploadLimits"), url_query);
}

const bool &AppBskyVideoGetUploadLimits::canUpload() const
{
    return m_canUpload;
}

const qint64 &AppBskyVideoGetUploadLimits::remainingDailyVideos() const
{
    return m_remainingDailyVideos;
}

const qint64 &AppBskyVideoGetUploadLimits::remainingDailyBytes() const
{
    return m_remainingDailyBytes;
}

const QString &AppBskyVideoGetUploadLimits::message() const
{
    return m_message;
}

const QString &AppBskyVideoGetUploadLimits::error() const
{
    return m_error;
}

bool AppBskyVideoGetUploadLimits::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::LexiconsTypeUnknown::copyBool(json_doc.object().value("canUpload"),
                                                      m_canUpload);
        AtProtocolType::LexiconsTypeUnknown::copyInt(
                json_doc.object().value("remainingDailyVideos"), m_remainingDailyVideos);
        AtProtocolType::LexiconsTypeUnknown::copyInt(json_doc.object().value("remainingDailyBytes"),
                                                     m_remainingDailyBytes);
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("message"),
                                                        m_message);
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("error"), m_error);
    }

    return success;
}

}
