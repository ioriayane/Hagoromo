#include "appbskyunspeccedgetpostthreadhiddenv2.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyUnspeccedGetPostThreadHiddenV2::AppBskyUnspeccedGetPostThreadHiddenV2(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyUnspeccedGetPostThreadHiddenV2::getPostThreadHiddenV2(const QString &anchor)
{
    QUrlQuery url_query;
    if (!anchor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("anchor"), anchor);
    }

    get(QStringLiteral("xrpc/app.bsky.unspecced.getPostThreadHiddenV2"), url_query);
}

const QList<AtProtocolType::AppBskyUnspeccedGetPostThreadHiddenV2::ThreadHiddenItem> &
AppBskyUnspeccedGetPostThreadHiddenV2::threadList() const
{
    return m_threadList;
}

bool AppBskyUnspeccedGetPostThreadHiddenV2::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("thread")) {
        success = false;
    } else {
        for (const auto &value : json_doc.object().value("thread").toArray()) {
            AtProtocolType::AppBskyUnspeccedGetPostThreadHiddenV2::ThreadHiddenItem data;
            AtProtocolType::AppBskyUnspeccedGetPostThreadHiddenV2::copyThreadHiddenItem(
                    value.toObject(), data);
            m_threadList.append(data);
        }
    }

    return success;
}

}
