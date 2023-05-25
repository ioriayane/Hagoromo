#include "recordoperator.h"
#include "atprotocol/com/atproto/repo/comatprotorepocreaterecord.h"
#include "atprotocol/com/atproto/repo/comatprotorepouploadblob.h"

#include <QPointer>
#include <QTimer>

using AtProtocolInterface::ComAtprotoRepoCreateRecord;
using AtProtocolInterface::ComAtprotoRepoUploadBlob;
using namespace AtProtocolType;

RecordOperator::RecordOperator(QObject *parent) : QObject { parent }, m_running(false) { }

void RecordOperator::setAccount(const QString &service, const QString &did, const QString &handle,
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

void RecordOperator::setText(const QString &text)
{
    m_text = text;
}

void RecordOperator::setReply(const QString &parent_cid, const QString &parent_uri,
                            const QString &root_cid, const QString &root_uri)
{
    m_replyParent.cid = parent_cid;
    m_replyParent.uri = parent_uri;
    m_replyRoot.cid = root_cid;
    m_replyRoot.uri = root_uri;
}

void RecordOperator::setQuote(const QString &cid, const QString &uri)
{
    m_embedQuote.cid = cid;
    m_embedQuote.uri = uri;
}

void RecordOperator::setImages(const QStringList &images)
{
    m_embedImages = images;
}

void RecordOperator::clear()
{
    m_text.clear();
    m_replyParent = AtProtocolType::ComAtprotoRepoStrongRef::Main();
    m_replyRoot = AtProtocolType::ComAtprotoRepoStrongRef::Main();
    m_embedQuote = AtProtocolType::ComAtprotoRepoStrongRef::Main();
    m_embedImages.clear();
    m_embedImageBlogs.clear();
}

void RecordOperator::post()
{
    if (m_text.isEmpty())
        return;

    setRunning(true);

    QPointer<RecordOperator> aliving(this);

    ComAtprotoRepoCreateRecord *create_record = new ComAtprotoRepoCreateRecord();
    connect(create_record, &ComAtprotoRepoCreateRecord::finished, [=](bool success) {
        if (aliving) {
            emit finished(success);
            setRunning(false);
        }
        create_record->deleteLater();
    });
    create_record->setAccount(m_account);
    create_record->setReply(m_replyParent.cid, m_replyParent.uri, m_replyRoot.cid, m_replyRoot.uri);
    create_record->setQuote(m_embedQuote.cid, m_embedQuote.uri);
    create_record->setImageBlobs(m_embedImageBlogs);
    create_record->post(m_text);
}

void RecordOperator::postWithImages()
{
    if (m_embedImages.isEmpty())
        return;

    setRunning(true);

    QString path = QUrl(m_embedImages.first()).toLocalFile();
    m_embedImages.removeFirst();

    QPointer<RecordOperator> aliving(this);

    ComAtprotoRepoUploadBlob *upload_blob = new ComAtprotoRepoUploadBlob();
    connect(upload_blob, &ComAtprotoRepoUploadBlob::finished, [=](bool success) {
        if (aliving) {
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
            } else {
                emit finished(success);
                setRunning(false);
            }
        }
        upload_blob->deleteLater();
    });
    upload_blob->setAccount(m_account);
    upload_blob->uploadBlob(path);
}

void RecordOperator::repost(const QString &cid, const QString &uri)
{
    if (running())
        return;
    setRunning(true);

    QPointer<RecordOperator> aliving(this);

    ComAtprotoRepoCreateRecord *create_record = new ComAtprotoRepoCreateRecord();
    connect(create_record, &ComAtprotoRepoCreateRecord::finished, [=](bool success) {
        if (aliving) {
            emit finished(success);
            setRunning(false);

            // 成功なら、受け取ったデータでTLデータの更新をしないと値が大きくならない
        }
        create_record->deleteLater();
    });
    create_record->setAccount(m_account);
    create_record->repost(cid, uri);
}

void RecordOperator::like(const QString &cid, const QString &uri)
{
    if (running())
        return;
    setRunning(true);

    QPointer<RecordOperator> aliving(this);

    ComAtprotoRepoCreateRecord *create_record = new ComAtprotoRepoCreateRecord();
    connect(create_record, &ComAtprotoRepoCreateRecord::finished, [=](bool success) {
        if (aliving) {
            emit finished(success);
            setRunning(false);

            // 成功なら、受け取ったデータでTLデータの更新をしないと値が大きくならない
        }
        create_record->deleteLater();
    });
    create_record->setAccount(m_account);
    create_record->like(cid, uri);
}

void RecordOperator::follow(const QString &did)
{
    if (running())
        return;
    setRunning(true);

    QPointer<RecordOperator> aliving(this);

    ComAtprotoRepoCreateRecord *create_record = new ComAtprotoRepoCreateRecord();
    connect(create_record, &ComAtprotoRepoCreateRecord::finished, [=](bool success) {
        if (aliving) {
            emit finished(success);
            setRunning(false);
        }
        create_record->deleteLater();
    });
    create_record->setAccount(m_account);
    create_record->follow(did);
}

bool RecordOperator::running() const
{
    return m_running;
}

void RecordOperator::setRunning(bool newRunning)
{
    if (m_running == newRunning)
        return;
    m_running = newRunning;
    emit runningChanged();
}
