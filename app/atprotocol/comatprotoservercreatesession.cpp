#include "comatprotoservercreatesession.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <qDebug>

ComAtprotoServerCreateSession::ComAtprotoServerCreateSession(QObject *parent)
    : AccessAtProtocol { parent }
{
    //    qDebug() << "ComAtprotoServerCreateSession::ComAtprotoServerCreateSession()" << this;
}

ComAtprotoServerCreateSession::~ComAtprotoServerCreateSession()
{
    //    qDebug() << "ComAtprotoServerCreateSession::~ComAtprotoServerCreateSession" << this <<
    //    service()
    //             << did();
}

void ComAtprotoServerCreateSession::create(const QString &id, const QString &password)
{
    QJsonObject json_obj;
    json_obj.insert("identifier", id);
    json_obj.insert("password", password);
    QJsonDocument json_doc(json_obj);

    post(QStringLiteral("xrpc/com.atproto.server.createSession"),
         json_doc.toJson(QJsonDocument::Compact), false);
}

void ComAtprotoServerCreateSession::parseJson(const QString reply_json)
{
    bool success = false;

    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (!json_doc.isEmpty()) {
        setSession(json_doc.object().value("did").toString(),
                   json_doc.object().value("handle").toString(),
                   json_doc.object().value("email").toString(),
                   json_doc.object().value("accessJwt").toString(),
                   json_doc.object().value("refreshJwt").toString());

        if (!did().isEmpty() && !handle().isEmpty() && !email().isEmpty() && !accessJwt().isEmpty()
            && !refreshJwt().isEmpty()) {
            success = true;
        }
    }

    emit finished(success);
}
