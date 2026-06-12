#include "appbskyembedgetembedexternalview.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyEmbedGetEmbedExternalView::AppBskyEmbedGetEmbedExternalView(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyEmbedGetEmbedExternalView::getEmbedExternalView(const QString &url,
                                                            const QList<QString> &uris)
{
    QUrlQuery url_query;
    if (!url.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("url"), url);
    }
    for (const auto &value : uris) {
        url_query.addQueryItem(QStringLiteral("uris"), value);
    }

    get(QStringLiteral("xrpc/app.bsky.embed.getEmbedExternalView"), url_query);
}

const AtProtocolType::AppBskyEmbedExternal::View &AppBskyEmbedGetEmbedExternalView::view() const
{
    return m_view;
}

const QList<AtProtocolType::ComAtprotoRepoStrongRef::Main> &
AppBskyEmbedGetEmbedExternalView::associatedRefsList() const
{
    return m_associatedRefsList;
}

bool AppBskyEmbedGetEmbedExternalView::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("associatedRefs")) {
        success = false;
    } else {
        AtProtocolType::AppBskyEmbedExternal::copyView(json_doc.object().value("view").toObject(),
                                                       m_view);
        for (const auto &value : json_doc.object().value("associatedRefs").toArray()) {
            AtProtocolType::ComAtprotoRepoStrongRef::Main data;
            AtProtocolType::ComAtprotoRepoStrongRef::copyMain(value.toObject(), data);
            m_associatedRefsList.append(data);
        }
    }

    return success;
}

}
