#include "appbskydraftcreatedraft.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyDraftCreateDraft::AppBskyDraftCreateDraft(QObject *parent) : AccessAtProtocol { parent } { }

void AppBskyDraftCreateDraft::createDraft(const QJsonObject &draft)
{
    QJsonObject json_obj;
    if (!draft.isEmpty()) {
        json_obj.insert(QStringLiteral("draft"), draft);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/app.bsky.draft.createDraft"),
         json_doc.toJson(QJsonDocument::Compact));
}

const QString &AppBskyDraftCreateDraft::id() const
{
    return m_id;
}

bool AppBskyDraftCreateDraft::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("id"), m_id);
    }

    return success;
}

}
