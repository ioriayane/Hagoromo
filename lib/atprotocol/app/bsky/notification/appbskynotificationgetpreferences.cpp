#include "appbskynotificationgetpreferences.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyNotificationGetPreferences::AppBskyNotificationGetPreferences(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyNotificationGetPreferences::getPreferences()
{
    QUrlQuery url_query;

    get(QStringLiteral("xrpc/app.bsky.notification.getPreferences"), url_query);
}

const AtProtocolType::AppBskyNotificationDefs::Preferences &
AppBskyNotificationGetPreferences::preferences() const
{
    return m_preferences;
}

bool AppBskyNotificationGetPreferences::parseJson(bool success, const QString reply_json)
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
