#ifndef APPBSKYNOTIFICATIONPUTPREFERENCESV2_H
#define APPBSKYNOTIFICATIONPUTPREFERENCESV2_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyNotificationPutPreferencesV2 : public AccessAtProtocol
{
public:
    explicit AppBskyNotificationPutPreferencesV2(QObject *parent = nullptr);

    void putPreferencesV2(const QJsonObject &chat, const QJsonObject &follow,
                          const QJsonObject &like, const QJsonObject &likeViaRepost,
                          const QJsonObject &mention, const QJsonObject &quote,
                          const QJsonObject &reply, const QJsonObject &repost,
                          const QJsonObject &repostViaRepost, const QJsonObject &starterpackJoined,
                          const QJsonObject &subscribedPost, const QJsonObject &unverified,
                          const QJsonObject &verified);

    const AtProtocolType::AppBskyNotificationDefs::Preferences &preferences() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::AppBskyNotificationDefs::Preferences m_preferences;
};

}

#endif // APPBSKYNOTIFICATIONPUTPREFERENCESV2_H
