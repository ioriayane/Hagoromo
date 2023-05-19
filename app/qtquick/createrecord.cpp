#include "createrecord.h"
#include "atprotocol/com/atproto/repo/comatprotorepocreaterecord.h"
#include "atprotocol/com/atproto/repo/comatprotorepouploadblob.h"

#include <QTimer>

using AtProtocolInterface::ComAtprotoRepoCreateRecord;
using AtProtocolInterface::ComAtprotoRepoUploadBlob;
using namespace AtProtocolType;

CreateRecord::CreateRecord(QObject *parent) : QObject { parent } { }

void CreateRecord::setAccount(const QString &service, const QString &did, const QString &handle,
                              const QString &email, const QString &accessJwt,
                              const QString &refreshJwt)
{
    m_account.service = service;
    m_account.did = did;
    m_account.handle = handle;
    m_account.email = email;
    m_account.accessJwt = accessJwt;
    m_account.refreshJwt = refreshJwt;
}

void CreateRecord::setText(const QString &text)
{
    m_text = text;
}

void CreateRecord::setReply(const QString &parent_cid, const QString &parent_uri,
                            const QString &root_cid, const QString &root_uri)
{
    m_replyParent.cid = parent_cid;
    m_replyParent.uri = parent_uri;
    m_replyRoot.cid = root_cid;
    m_replyRoot.uri = root_uri;
}

void CreateRecord::setQuote(const QString &cid, const QString &uri)
{
    m_embedQuote.cid = cid;
    m_embedQuote.uri = uri;
}

void CreateRecord::setImages(const QStringList &images)
{
    m_embedImages = images;
}

void CreateRecord::clear()
{
    m_text.clear();
    m_replyParent = AtProtocolType::ComAtprotoRepoStrongRef::Main();
    m_replyRoot = AtProtocolType::ComAtprotoRepoStrongRef::Main();
    m_embedQuote = AtProtocolType::ComAtprotoRepoStrongRef::Main();
    m_embedImages.clear();
    m_embedImageBlogs.clear();
}

void CreateRecord::post()
{
    if (m_text.isEmpty())
        return;

    ComAtprotoRepoCreateRecord *create_record = new ComAtprotoRepoCreateRecord();
    connect(create_record, &ComAtprotoRepoCreateRecord::finished, [=](bool success) {
        emit finished(success);
        create_record->deleteLater();
    });
    create_record->setAccount(m_account);
    create_record->setReply(m_replyParent.cid, m_replyParent.uri, m_replyRoot.cid, m_replyRoot.uri);
    create_record->setQuote(m_embedQuote.cid, m_embedQuote.uri);
    create_record->setImageBlobs(m_embedImageBlogs);
    create_record->post(m_text);
}

void CreateRecord::postWithImages()
{
    if (m_embedImages.isEmpty())
        return;
    QString path = QUrl(m_embedImages.first()).toLocalFile();
    m_embedImages.removeFirst();

    ComAtprotoRepoUploadBlob *upload_blob = new ComAtprotoRepoUploadBlob();
    connect(upload_blob, &ComAtprotoRepoUploadBlob::finished, [=](bool success) {
        if (success) {
            qDebug() << "Uploaded blob" << upload_blob->cid() << upload_blob->mimeType()
                     << upload_blob->size();

            LexiconsTypeUnknown::Blob blob;
            blob.cid = upload_blob->cid();
            blob.mimeType = upload_blob->mimeType();
            blob.size = upload_blob->size();
            m_embedImageBlogs.append(blob);

            if (m_embedImages.isEmpty()) {
                post();
            } else {
                postWithImages();
            }
        }
        emit finished(success);
        upload_blob->deleteLater();
    });
    upload_blob->setAccount(m_account);
    upload_blob->uploadBlob(path);
}

void CreateRecord::repost(const QString &cid, const QString &uri)
{
    ComAtprotoRepoCreateRecord *create_record = new ComAtprotoRepoCreateRecord();
    connect(create_record, &ComAtprotoRepoCreateRecord::finished, [=](bool success) {
        emit finished(success);

        // 成功なら、受け取ったデータでTLデータの更新をしないと値が大きくならない

        create_record->deleteLater();
    });
    create_record->setAccount(m_account);
    create_record->repost(cid, uri);
}

void CreateRecord::like(const QString &cid, const QString &uri)
{
    ComAtprotoRepoCreateRecord *create_record = new ComAtprotoRepoCreateRecord();
    connect(create_record, &ComAtprotoRepoCreateRecord::finished, [=](bool success) {
        emit finished(success);

        // 成功なら、受け取ったデータでTLデータの更新をしないと値が大きくならない

        create_record->deleteLater();
    });
    create_record->setAccount(m_account);
    create_record->like(cid, uri);
}
