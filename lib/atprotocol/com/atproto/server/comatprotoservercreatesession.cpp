#include "comatprotoservercreatesession.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ComAtprotoServerCreateSession::ComAtprotoServerCreateSession(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ComAtprotoServerCreateSession::createSession(const QString &identifier,
                                                  const QString &password,
                                                  const QString &authFactorToken)
{
    QJsonObject json_obj;
    if (!identifier.isEmpty()) {
        json_obj.insert(QStringLiteral("identifier"), identifier);
    }
    if (!password.isEmpty()) {
        json_obj.insert(QStringLiteral("password"), password);
    }
    if (!authFactorToken.isEmpty()) {
        json_obj.insert(QStringLiteral("authFactorToken"), authFactorToken);
    }

    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/com.atproto.server.createSession"),
         json_doc.toJson(QJsonDocument::Compact), false);
}

const QString &ComAtprotoServerCreateSession::accessJwt() const
{
    return m_accessJwt;
}

const QString &ComAtprotoServerCreateSession::refreshJwt() const
{
    return m_refreshJwt;
}

const QString &ComAtprotoServerCreateSession::handle() const
{
    return m_handle;
}

const QString &ComAtprotoServerCreateSession::did() const
{
    return m_did;
}

const QVariant &ComAtprotoServerCreateSession::didDoc() const
{
    return m_didDoc;
}

const QString &ComAtprotoServerCreateSession::email() const
{
    return m_email;
}

const bool &ComAtprotoServerCreateSession::emailConfirmed() const
{
    return m_emailConfirmed;
}

const bool &ComAtprotoServerCreateSession::emailAuthFactor() const
{
    return m_emailAuthFactor;
}

bool ComAtprotoServerCreateSession::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("accessJwt"),
                                                        m_accessJwt);
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("refreshJwt"),
                                                        m_refreshJwt);
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("handle"),
                                                        m_handle);
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("did"), m_did);
        AtProtocolType::LexiconsTypeUnknown::copyUnknown(
                json_doc.object().value("didDoc").toObject(), m_didDoc);
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("email"), m_email);
        AtProtocolType::LexiconsTypeUnknown::copyBool(json_doc.object().value("emailConfirmed"),
                                                      m_emailConfirmed);
        AtProtocolType::LexiconsTypeUnknown::copyBool(json_doc.object().value("emailAuthFactor"),
                                                      m_emailAuthFactor);
    }

    return success;
}

}
