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
    m_adultContentPref.enabled = true;
}

bool AppBskyActorGetPreferences::getPreferences()
{
    QUrlQuery query;

    return get(QStringLiteral("xrpc/app.bsky.actor.getPreferences"), query);
}

const QList<AtProtocolType::AppBskyActorDefs::SavedFeedsPref> *
AppBskyActorGetPreferences::savedFeedsPrefList() const
{
    return &m_savedFeedsPrefList;
}

const QList<AtProtocolType::AppBskyActorDefs::ContentLabelPref> *
AppBskyActorGetPreferences::contentLabelPrefList() const
{
    return &m_contentLabelPrefList;
}

const AtProtocolType::AppBskyActorDefs::AdultContentPref
AppBskyActorGetPreferences::adultContentPref() const
{
    return m_adultContentPref;
}

bool AppBskyActorGetPreferences::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("preferences")) {
        success = false;
    } else {
        for (const auto &value : json_doc.object().value("preferences").toArray()) {
            QString type = value.toObject().value("$type").toString();
            if (type == "app.bsky.actor.defs#savedFeedsPref") {
                AtProtocolType::AppBskyActorDefs::SavedFeedsPref saved_feeds;
                AtProtocolType::AppBskyActorDefs::copySavedFeedsPref(value.toObject(), saved_feeds);
                m_savedFeedsPrefList.append(saved_feeds);
            } else if (type == "app.bsky.actor.defs#adultContentPref") {
                AtProtocolType::AppBskyActorDefs::copyAdultContentPref(value.toObject(),
                                                                       m_adultContentPref);
            } else if (type == "app.bsky.actor.defs#contentLabelPref") {
                AtProtocolType::AppBskyActorDefs::ContentLabelPref content_label;
                AtProtocolType::AppBskyActorDefs::copyContentLabelPref(value.toObject(),
                                                                       content_label);
                m_contentLabelPrefList.append(content_label);
            }
        }
    }

    return success;
}

}
