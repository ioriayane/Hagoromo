#include "appbskycontactremovedata.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyContactRemoveData::AppBskyContactRemoveData(QObject *parent)
    : AccessAtProtocol { parent } { }

void AppBskyContactRemoveData::removeData()
{
    post(QStringLiteral("xrpc/app.bsky.contact.removeData"), QByteArray());
}

bool AppBskyContactRemoveData::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)
    return success;
}

}
