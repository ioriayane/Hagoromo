#include "chatbskygroupgetgrouppublicinfo.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ChatBskyGroupGetGroupPublicInfo::ChatBskyGroupGetGroupPublicInfo(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ChatBskyGroupGetGroupPublicInfo::getGroupPublicInfo(const QString &code)
{
    QUrlQuery url_query;
    if (!code.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("code"), code);
    }

    get(QStringLiteral("xrpc/chat.bsky.group.getGroupPublicInfo"), url_query);
}

const AtProtocolType::ChatBskyGroupDefs::GroupPublicView &
ChatBskyGroupGetGroupPublicInfo::group() const
{
    return m_group;
}

bool ChatBskyGroupGetGroupPublicInfo::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::ChatBskyGroupDefs::copyGroupPublicView(
                json_doc.object().value("group").toObject(), m_group);
    }

    return success;
}

}
