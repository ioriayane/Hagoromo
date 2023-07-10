#include "recordoperator.h"
#include "atprotocol/com/atproto/repo/comatprotorepocreaterecord.h"
#include "atprotocol/com/atproto/repo/comatprotorepouploadblob.h"
#include "atprotocol/com/atproto/repo/comatprotorepodeleterecord.h"
#include "atprotocol/app/bsky/actor/appbskyactorgetprofiles.h"

#include <QPointer>
#include <QTimer>

using AtProtocolInterface::AppBskyActorGetProfiles;
using AtProtocolInterface::ComAtprotoRepoCreateRecord;
using AtProtocolInterface::ComAtprotoRepoDeleteRecord;
using AtProtocolInterface::ComAtprotoRepoUploadBlob;
using namespace AtProtocolType;

struct MentionData
{
    int start = -1;
    int end = -1;
};

RecordOperator::RecordOperator(QObject *parent) : QObject { parent }, m_running(false)
{
    m_rxFacet = QRegularExpression(
            QString("(?:%1)|(?:%2)")
                    .arg("http[s]?://"
                         "(?:[a-zA-Z]|[0-9]|[$-_@.&+]|[!*\\(\\),]|(?:%[0-9a-fA-F][0-9a-fA-F]))+",
                         "@(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?\\.)+(?:[a-zA-Z0-9](?:["
                         "a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)"));
}

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

void RecordOperator::setPostLanguages(const QStringList &langs)
{
    m_postLanguages = langs;
}

void RecordOperator::setOpenGraphData(const QString &url)
{
    //
}

void RecordOperator::clear()
{
    m_text.clear();
    m_replyParent = AtProtocolType::ComAtprotoRepoStrongRef::Main();
    m_replyRoot = AtProtocolType::ComAtprotoRepoStrongRef::Main();
    m_embedQuote = AtProtocolType::ComAtprotoRepoStrongRef::Main();
    m_embedImages.clear();
    m_embedImageBlogs.clear();
    m_facets.clear();
}

void RecordOperator::post()
{
    if (m_text.isEmpty())
        return;

    setRunning(true);

    makeFacets(m_text, [=]() {
        QPointer<RecordOperator> aliving(this);

        ComAtprotoRepoCreateRecord *create_record = new ComAtprotoRepoCreateRecord();
        connect(create_record, &ComAtprotoRepoCreateRecord::finished, [=](bool success) {
            if (aliving) {
                emit finished(success, QString(), QString());
                if (!success) {
                    emit errorOccured(create_record->errorMessage());
                }
                setRunning(false);
            }
            create_record->deleteLater();
        });
        create_record->setAccount(m_account);
        create_record->setReply(m_replyParent.cid, m_replyParent.uri, m_replyRoot.cid,
                                m_replyRoot.uri);
        create_record->setQuote(m_embedQuote.cid, m_embedQuote.uri);
        create_record->setImageBlobs(m_embedImageBlogs);
        create_record->setFacets(m_facets);
        create_record->setPostLanguages(m_postLanguages);
        create_record->post(m_text);
    });
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
                emit errorOccured(upload_blob->errorMessage());
                emit finished(success, QString(), QString());
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
            if (!success) {
                emit errorOccured(create_record->errorMessage());
            }
            emit finished(success, create_record->replyUri(), create_record->replyCid());
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
            if (!success) {
                emit errorOccured(create_record->errorMessage());
            }
            emit finished(success, create_record->replyUri(), create_record->replyCid());
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
            if (!success) {
                emit errorOccured(create_record->errorMessage());
            }
            emit finished(success, QString(), QString());
            setRunning(false);
        }
        create_record->deleteLater();
    });
    create_record->setAccount(m_account);
    create_record->follow(did);
}

void RecordOperator::deletePost(const QString &uri)
{
    if (running() || !uri.startsWith("at://"))
        return;
    setRunning(true);

    QString r_key = uri.split("/").last();

    QPointer<RecordOperator> aliving(this);

    ComAtprotoRepoDeleteRecord *delete_record = new ComAtprotoRepoDeleteRecord();
    connect(delete_record, &ComAtprotoRepoDeleteRecord::finished, [=](bool success) {
        if (aliving) {
            if (!success) {
                emit errorOccured(delete_record->errorMessage());
            }
            emit finished(success, QString(), QString());
            setRunning(false);
        }
        delete_record->deleteLater();
    });
    delete_record->setAccount(m_account);
    delete_record->deletePost(r_key);
}

void RecordOperator::deleteLike(const QString &uri)
{
    if (running() || !uri.startsWith("at://"))
        return;
    setRunning(true);

    QString r_key = uri.split("/").last();

    QPointer<RecordOperator> aliving(this);

    ComAtprotoRepoDeleteRecord *delete_record = new ComAtprotoRepoDeleteRecord();
    connect(delete_record, &ComAtprotoRepoDeleteRecord::finished, [=](bool success) {
        if (aliving) {
            if (!success) {
                emit errorOccured(delete_record->errorMessage());
            }
            emit finished(success, QString(), QString());
            setRunning(false);
        }
        delete_record->deleteLater();
    });
    delete_record->setAccount(m_account);
    delete_record->deleteLike(r_key);
}

void RecordOperator::deleteRepost(const QString &uri)
{
    if (running() || !uri.startsWith("at://"))
        return;
    setRunning(true);

    QString r_key = uri.split("/").last();

    QPointer<RecordOperator> aliving(this);

    ComAtprotoRepoDeleteRecord *delete_record = new ComAtprotoRepoDeleteRecord();
    connect(delete_record, &ComAtprotoRepoDeleteRecord::finished, [=](bool success) {
        if (aliving) {
            if (!success) {
                emit errorOccured(delete_record->errorMessage());
            }
            emit finished(success, QString(), QString());
            setRunning(false);
        }
        delete_record->deleteLater();
    });
    delete_record->setAccount(m_account);
    delete_record->deleteRepost(r_key);
}

void RecordOperator::deleteFollow(const QString &uri)
{
    if (running() || !uri.startsWith("at://"))
        return;
    setRunning(true);

    QString r_key = uri.split("/").last();

    QPointer<RecordOperator> aliving(this);

    ComAtprotoRepoDeleteRecord *delete_record = new ComAtprotoRepoDeleteRecord();
    connect(delete_record, &ComAtprotoRepoDeleteRecord::finished, [=](bool success) {
        if (aliving) {
            if (!success) {
                emit errorOccured(delete_record->errorMessage());
            }
            emit finished(success, QString(), QString());
            setRunning(false);
        }
        delete_record->deleteLater();
    });
    delete_record->setAccount(m_account);
    delete_record->unfollow(r_key);
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

template<typename F>
void RecordOperator::makeFacets(const QString &text, F callback)
{
    QMultiMap<QString, MentionData> mention;

    QRegularExpressionMatch match = m_rxFacet.match(text);
    if (!match.capturedTexts().isEmpty()) {
        QString temp;
        int pos;
        int byte_start = 0;
        int byte_end = 0;
        while ((pos = match.capturedStart()) != -1) {
            byte_start = text.left(pos).toUtf8().length();
            temp = match.captured();
            byte_end = byte_start + temp.toUtf8().length();

            if (temp.startsWith("@")) {
                temp.remove("@");
                MentionData position;
                position.start = byte_start;
                position.end = byte_end;
                mention.insert(temp, position);
            } else {
                AppBskyRichtextFacet::Main facet;
                facet.index.byteStart = byte_start;
                facet.index.byteEnd = byte_end;
                AppBskyRichtextFacet::Link link;
                link.uri = temp;
                facet.features_type = AppBskyRichtextFacet::MainFeaturesType::features_Link;
                facet.features_Link.append(link);
                m_facets.append(facet);
            }

            match = m_rxFacet.match(text, pos + match.capturedLength());
        }

        if (!mention.isEmpty()) {
            QStringList ids;
            QMapIterator<QString, MentionData> i(mention);
            while (i.hasNext()) {
                i.next();
                if (!ids.contains(i.key())) {
                    ids.append(i.key());
                }
            }

            QPointer<RecordOperator> aliving(this);
            AppBskyActorGetProfiles *profiles = new AppBskyActorGetProfiles();
            connect(profiles, &AppBskyActorGetProfiles::finished, [=](bool success) {
                if (success && aliving) {
                    for (const auto &item : qAsConst(*profiles->profileViewDetaileds())) {
                        QString handle = item.handle;
                        handle.remove("@");
                        if (mention.contains(handle)) {
                            const QList<MentionData> positions = mention.values(handle);
                            for (const auto &position : positions) {
                                AppBskyRichtextFacet::Main facet;
                                facet.index.byteStart = position.start;
                                facet.index.byteEnd = position.end;
                                AppBskyRichtextFacet::Mention mention;
                                mention.did = item.did;
                                facet.features_type =
                                        AppBskyRichtextFacet::MainFeaturesType::features_Mention;
                                facet.features_Mention.append(mention);
                                m_facets.append(facet);
                            }
                        }
                    }
                    callback();
                }
                profiles->deleteLater();
            });
            profiles->setAccount(m_account);
            profiles->getProfiles(ids);
        } else {
            // mentionがないときは直接戻る
            callback();
        }
    } else {
        // uriもmentionがないときは直接戻る
        callback();
    }
}
