#include "appbskyfeedgetfeedgenerator.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyFeedGetFeedGenerator::AppBskyFeedGetFeedGenerator(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyFeedGetFeedGenerator::getFeedGenerator(const QString &feed)
{
    QUrlQuery url_query;
    if (!feed.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("feed"), feed);
    }

    get(QStringLiteral("xrpc/app.bsky.feed.getFeedGenerator"), url_query);
}

const AtProtocolType::AppBskyFeedDefs::GeneratorView &AppBskyFeedGetFeedGenerator::view() const
{
    return m_view;
}

const bool &AppBskyFeedGetFeedGenerator::isOnline() const
{
    return m_isOnline;
}

const bool &AppBskyFeedGetFeedGenerator::isValid() const
{
    return m_isValid;
}

bool AppBskyFeedGetFeedGenerator::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::AppBskyFeedDefs::copyGeneratorView(
                json_doc.object().value("view").toObject(), m_view);
        AtProtocolType::LexiconsTypeUnknown::copyBool(json_doc.object().value("isOnline"),
                                                      m_isOnline);
        AtProtocolType::LexiconsTypeUnknown::copyBool(json_doc.object().value("isValid"),
                                                      m_isValid);
    }

    return success;
}

}
