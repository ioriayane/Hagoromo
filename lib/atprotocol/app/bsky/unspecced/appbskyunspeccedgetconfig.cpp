#include "appbskyunspeccedgetconfig.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyUnspeccedGetConfig::AppBskyUnspeccedGetConfig(QObject *parent) : AccessAtProtocol { parent }
{
}

void AppBskyUnspeccedGetConfig::getConfig()
{
    QUrlQuery url_query;

    get(QStringLiteral("xrpc/app.bsky.unspecced.getConfig"), url_query);
}

const bool &AppBskyUnspeccedGetConfig::checkEmailConfirmed() const
{
    return m_checkEmailConfirmed;
}

bool AppBskyUnspeccedGetConfig::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::LexiconsTypeUnknown::copyBool(
                json_doc.object().value("checkEmailConfirmed"), m_checkEmailConfirmed);
    }

    return success;
}

}
