#include "appbskyactorgetpreferences.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyActorGetPreferences::AppBskyActorGetPreferences(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyActorGetPreferences::getPreferences()
{
    QUrlQuery url_query;

    get(QStringLiteral("xrpc/app.bsky.actor.getPreferences"), url_query);
}

const AtProtocolType::AppBskyActorDefs::Preferences &AppBskyActorGetPreferences::preferences() const
{
    return m_preferences;
}

bool AppBskyActorGetPreferences::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::AppBskyActorDefs::copyPreferences(
                json_doc.object().value("preferences").toArray(), m_preferences);
    }

    return success;
}

}
