#include "appbskycontactverifyphone.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

AppBskyContactVerifyPhone::AppBskyContactVerifyPhone(QObject *parent) : AccessAtProtocol { parent }
{
}

void AppBskyContactVerifyPhone::verifyPhone(const QString &phone, const QString &code)
{
    QJsonObject json_obj;
    if (!phone.isEmpty()) {
        json_obj.insert(QStringLiteral("phone"), phone);
    }
    if (!code.isEmpty()) {
        json_obj.insert(QStringLiteral("code"), code);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/app.bsky.contact.verifyPhone"),
         json_doc.toJson(QJsonDocument::Compact));
}

const QString &AppBskyContactVerifyPhone::token() const
{
    return m_token;
}

bool AppBskyContactVerifyPhone::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("token"), m_token);
    }

    return success;
}

}
