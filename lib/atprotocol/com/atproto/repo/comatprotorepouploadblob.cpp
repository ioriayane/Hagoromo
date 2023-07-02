#include "comatprotorepouploadblob.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace AtProtocolInterface {

ComAtprotoRepoUploadBlob::ComAtprotoRepoUploadBlob(QObject *parent) : AccessAtProtocol { parent }
{

    m_compressor.moveToThread(&m_thread);

    connect(this, &ComAtprotoRepoUploadBlob::compress, &m_compressor, &ImageCompressor::compress);
    connect(&m_compressor, &ImageCompressor::compressed, this,
            &ComAtprotoRepoUploadBlob::compressed);

    m_thread.start();
}

ComAtprotoRepoUploadBlob::~ComAtprotoRepoUploadBlob()
{
    m_thread.exit();
    m_thread.wait();
}

void ComAtprotoRepoUploadBlob::uploadBlob(const QString &path)
{
    emit compress(path);
}

void ComAtprotoRepoUploadBlob::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
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
        } else {
            success = false;
        }
    }

    emit finished(success);
}

int ComAtprotoRepoUploadBlob::size() const
{
    return m_size;
}

void ComAtprotoRepoUploadBlob::compressed(const QString &path)
{
    postWithImage(QStringLiteral("xrpc/com.atproto.repo.uploadBlob"), path);
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
