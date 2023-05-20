#include "comatprotorepouploadblob.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ComAtprotoRepoUploadBlob::ComAtprotoRepoUploadBlob(QObject *parent)
    : AccessAtProtocol { parent } { }

void ComAtprotoRepoUploadBlob::uploadBlob(const QString &path)
{
    postWithImage(QStringLiteral("xrpc/com.atproto.repo.uploadBlob"), path);
}

void ComAtprotoRepoUploadBlob::parseJson(const QString reply_json)
{
    bool success = false;

    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
    } else {
        qDebug() << reply_json;
        QString cid = json_doc.object()
                              .value("blob")
                              .toObject()
                              .value("ref")
                              .toObject()
                              .value("$link")
                              .toString();
        QString mimetype = json_doc.object().value("blob").toObject().value("mimeType").toString();
        int size = json_doc.object().value("blob").toObject().value("size").toInt();
        if (!cid.isEmpty() && !mimetype.isEmpty()) {
            m_cid = cid;
            m_mimeType = mimetype;
            m_size = size;
            success = true;
        }
    }

    emit finished(success);
}

int ComAtprotoRepoUploadBlob::size() const
{
    return m_size;
}

QString ComAtprotoRepoUploadBlob::mimeType() const
{
    return m_mimeType;
}

QString ComAtprotoRepoUploadBlob::cid() const
{
    return m_cid;
}

}