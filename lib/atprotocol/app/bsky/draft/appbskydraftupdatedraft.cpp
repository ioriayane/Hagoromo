#include "appbskydraftupdatedraft.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyDraftUpdateDraft::AppBskyDraftUpdateDraft(QObject *parent) : AccessAtProtocol { parent } { }

void AppBskyDraftUpdateDraft::updateDraft(const QJsonObject &draft)
{
    QJsonObject json_obj;
    if (!draft.isEmpty()) {
        json_obj.insert(QStringLiteral("draft"), draft);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/app.bsky.draft.updateDraft"),
         json_doc.toJson(QJsonDocument::Compact));
}

bool AppBskyDraftUpdateDraft::parseJson(bool success, const QString reply_json)
{
    Q_UNUSED(reply_json)
    return success;
}

}
