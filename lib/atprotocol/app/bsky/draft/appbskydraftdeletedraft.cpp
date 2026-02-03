#include "appbskydraftdeletedraft.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyDraftDeleteDraft::AppBskyDraftDeleteDraft(QObject *parent) : AccessAtProtocol { parent } { }

void AppBskyDraftDeleteDraft::deleteDraft(const QString &id)
{
    QJsonObject json_obj;
    if (!id.isEmpty()) {
        json_obj.insert(QStringLiteral("id"), id);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/app.bsky.draft.deleteDraft"),
         json_doc.toJson(QJsonDocument::Compact));
}

bool AppBskyDraftDeleteDraft::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)
    return success;
}

}
