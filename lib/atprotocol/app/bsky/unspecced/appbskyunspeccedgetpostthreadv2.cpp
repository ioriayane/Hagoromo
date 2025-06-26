#include "appbskyunspeccedgetpostthreadv2.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyUnspeccedGetPostThreadV2::AppBskyUnspeccedGetPostThreadV2(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyUnspeccedGetPostThreadV2::getPostThreadV2(const QString &anchor, const bool above,
                                                      const int below, const int branchingFactor,
                                                      const bool prioritizeFollowedUsers,
                                                      const QString &sort)
{
    QUrlQuery url_query;
    if (!anchor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("anchor"), anchor);
    }
    if (above) {
        url_query.addQueryItem(QStringLiteral("above"), "true");
    }
    if (below > 0) {
        url_query.addQueryItem(QStringLiteral("below"), QString::number(below));
    }
    if (branchingFactor > 0) {
        url_query.addQueryItem(QStringLiteral("branchingFactor"), QString::number(branchingFactor));
    }
    if (prioritizeFollowedUsers) {
        url_query.addQueryItem(QStringLiteral("prioritizeFollowedUsers"), "true");
    }
    if (!sort.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("sort"), sort);
    }

    get(QStringLiteral("xrpc/app.bsky.unspecced.getPostThreadV2"), url_query);
}

const QList<AtProtocolType::AppBskyUnspeccedGetPostThreadV2::ThreadItem> &
AppBskyUnspeccedGetPostThreadV2::threadList() const
{
    return m_threadList;
}

const AtProtocolType::AppBskyFeedDefs::ThreadgateView &
AppBskyUnspeccedGetPostThreadV2::threadgate() const
{
    return m_threadgate;
}

const bool &AppBskyUnspeccedGetPostThreadV2::hasOtherReplies() const
{
    return m_hasOtherReplies;
}

bool AppBskyUnspeccedGetPostThreadV2::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("thread")) {
        success = false;
    } else {
        for (const auto &value : json_doc.object().value("thread").toArray()) {
            AtProtocolType::AppBskyUnspeccedGetPostThreadV2::ThreadItem data;
            AtProtocolType::AppBskyUnspeccedGetPostThreadV2::copyThreadItem(value.toObject(), data);
            m_threadList.append(data);
        }
        AtProtocolType::AppBskyFeedDefs::copyThreadgateView(
                json_doc.object().value("threadgate").toObject(), m_threadgate);
        AtProtocolType::LexiconsTypeUnknown::copyBool(json_doc.object().value("hasOtherReplies"),
                                                      m_hasOtherReplies);
    }

    return success;
}

}
