#include "appbskycontactgetsyncstatus.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

AppBskyContactGetSyncStatus::AppBskyContactGetSyncStatus(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void AppBskyContactGetSyncStatus::getSyncStatus()
{
    QUrlQuery url_query;

    get(QStringLiteral("xrpc/app.bsky.contact.getSyncStatus"), url_query);
}

const AtProtocolType::AppBskyContactDefs::SyncStatus &
AppBskyContactGetSyncStatus::syncStatus() const
{
    return m_syncStatus;
}

bool AppBskyContactGetSyncStatus::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::AppBskyContactDefs::copySyncStatus(
                json_doc.object().value("syncStatus").toObject(), m_syncStatus);
    }

    return success;
}

}
