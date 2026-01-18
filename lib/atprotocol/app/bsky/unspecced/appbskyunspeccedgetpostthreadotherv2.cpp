#include "appbskyunspeccedgetpostthreadotherv2.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyUnspeccedGetPostThreadOtherV2::AppBskyUnspeccedGetPostThreadOtherV2(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyUnspeccedGetPostThreadOtherV2::getPostThreadOtherV2(const QString &anchor)
{
    QUrlQuery url_query;
    if (!anchor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("anchor"), anchor);
    }

    get(QStringLiteral("xrpc/app.bsky.unspecced.getPostThreadOtherV2"), url_query);
}

const QList<AtProtocolType::AppBskyUnspeccedGetPostThreadOtherV2::ThreadItem> &
AppBskyUnspeccedGetPostThreadOtherV2::threadList() const
{
    return m_threadList;
}

bool AppBskyUnspeccedGetPostThreadOtherV2::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("thread")) {
        success = false;
    } else {
        for (const auto &value : json_doc.object().value("thread").toArray()) {
            AtProtocolType::AppBskyUnspeccedGetPostThreadOtherV2::ThreadItem data;
            AtProtocolType::AppBskyUnspeccedGetPostThreadOtherV2::copyThreadItem(value.toObject(),
                                                                                 data);
            m_threadList.append(data);
        }
    }

    return success;
}

}
