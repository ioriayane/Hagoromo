#include "appbskynotificationputpreferencesv2.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyNotificationPutPreferencesV2::AppBskyNotificationPutPreferencesV2(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyNotificationPutPreferencesV2::putPreferencesV2(
        const QJsonObject &chat, const QJsonObject &follow, const QJsonObject &like,
        const QJsonObject &likeViaRepost, const QJsonObject &mention, const QJsonObject &quote,
        const QJsonObject &reply, const QJsonObject &repost, const QJsonObject &repostViaRepost,
        const QJsonObject &starterpackJoined, const QJsonObject &subscribedPost,
        const QJsonObject &unverified, const QJsonObject &verified)
{
    QJsonObject json_obj;
    if (!chat.isEmpty()) {
        json_obj.insert(QStringLiteral("chat"), chat);
    }
    if (!follow.isEmpty()) {
        json_obj.insert(QStringLiteral("follow"), follow);
    }
    if (!like.isEmpty()) {
        json_obj.insert(QStringLiteral("like"), like);
    }
    if (!likeViaRepost.isEmpty()) {
        json_obj.insert(QStringLiteral("likeViaRepost"), likeViaRepost);
    }
    if (!mention.isEmpty()) {
        json_obj.insert(QStringLiteral("mention"), mention);
    }
    if (!quote.isEmpty()) {
        json_obj.insert(QStringLiteral("quote"), quote);
    }
    if (!reply.isEmpty()) {
        json_obj.insert(QStringLiteral("reply"), reply);
    }
    if (!repost.isEmpty()) {
        json_obj.insert(QStringLiteral("repost"), repost);
    }
    if (!repostViaRepost.isEmpty()) {
        json_obj.insert(QStringLiteral("repostViaRepost"), repostViaRepost);
    }
    if (!starterpackJoined.isEmpty()) {
        json_obj.insert(QStringLiteral("starterpackJoined"), starterpackJoined);
    }
    if (!subscribedPost.isEmpty()) {
        json_obj.insert(QStringLiteral("subscribedPost"), subscribedPost);
    }
    if (!unverified.isEmpty()) {
        json_obj.insert(QStringLiteral("unverified"), unverified);
    }
    if (!verified.isEmpty()) {
        json_obj.insert(QStringLiteral("verified"), verified);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/app.bsky.notification.putPreferencesV2"),
         json_doc.toJson(QJsonDocument::Compact));
}

const AtProtocolType::AppBskyNotificationDefs::Preferences &
AppBskyNotificationPutPreferencesV2::preferences() const
{
    return m_preferences;
}

bool AppBskyNotificationPutPreferencesV2::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::AppBskyNotificationDefs::copyPreferences(
                json_doc.object().value("preferences").toObject(), m_preferences);
    }

    return success;
}

}
