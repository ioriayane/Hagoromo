#include "appbskyactorgetpreferences.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyActorGetPreferences::AppBskyActorGetPreferences(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyActorGetPreferences::getPreferences()
{
    QUrlQuery query;

    get(QStringLiteral("xrpc/app.bsky.actor.getPreferences"), query);
}

const QList<AtProtocolType::AppBskyActorDefs::SavedFeedsPref> *
AppBskyActorGetPreferences::savedFeedsPrefList() const
{
    return &m_savedFeedsPrefList;
}

void AppBskyActorGetPreferences::parseJson(const QString reply_json)
{
    bool success = false;

    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
    } else if (!json_doc.object().contains("preferences")) {
    } else {
        for (const auto &value : json_doc.object().value("preferences").toArray()) {
            if (value.toObject().value("$type").toString()
                == "app.bsky.actor.defs#savedFeedsPref") {
                AtProtocolType::AppBskyActorDefs::SavedFeedsPref saved_feeds;
                AtProtocolType::AppBskyActorDefs::copySavedFeedsPref(value.toObject(), saved_feeds);
                m_savedFeedsPrefList.append(saved_feeds);
            }
        }
        success = true;
    }

    emit finished(success);
}

}
