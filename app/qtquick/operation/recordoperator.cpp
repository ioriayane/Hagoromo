#include "operation/recordoperator.h"
#include "atprotocol/com/atproto/repo/comatprotorepouploadblob.h"
#include "atprotocol/app/bsky/actor/appbskyactorgetprofiles.h"
#include "atprotocol/app/bsky/bookmark/appbskybookmarkcreatebookmark.h"
#include "atprotocol/app/bsky/bookmark/appbskybookmarkdeletebookmark.h"
#include "atprotocol/app/bsky/graph/appbskygraphmuteactor.h"
#include "atprotocol/app/bsky/graph/appbskygraphunmuteactor.h"
#include "atprotocol/app/bsky/notification/appbskynotificationputactivitysubscription.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "extension/com/atproto/repo/comatprotorepodeleterecordex.h"
#include "extension/com/atproto/repo/comatprotorepogetrecordex.h"
#include "extension/com/atproto/repo/comatprotorepolistrecordsex.h"
#include "extension/com/atproto/repo/comatprotorepoputrecordex.h"
#include "extension/app/bsky/video/appbskyvideogetuploadlimitsex.h"
#include "extension/app/bsky/video/appbskyvideouploadvideoex.h"
#include "tools/accountmanager.h"

#include <QFileInfo>
#include <QTimer>

using AtProtocolInterface::AppBskyActorGetProfiles;
using AtProtocolInterface::AppBskyBookmarkCreateBookmark;
using AtProtocolInterface::AppBskyBookmarkDeleteBookmark;
using AtProtocolInterface::AppBskyGraphMuteActor;
using AtProtocolInterface::AppBskyGraphUnmuteActor;
using AtProtocolInterface::AppBskyNotificationPutActivitySubscription;
using AtProtocolInterface::AppBskyVideoGetUploadLimitsEx;
using AtProtocolInterface::AppBskyVideoUploadVideoEx;
using AtProtocolInterface::ComAtprotoRepoCreateRecordEx;
using AtProtocolInterface::ComAtprotoRepoDeleteRecordEx;
using AtProtocolInterface::ComAtprotoRepoGetRecordEx;
using AtProtocolInterface::ComAtprotoRepoListRecordsEx;
using AtProtocolInterface::ComAtprotoRepoPutRecordEx;
using AtProtocolInterface::ComAtprotoRepoUploadBlob;
using namespace AtProtocolType;

RecordOperator::RecordOperator(QObject *parent)
    : QObject { parent },
      m_sequentialPostsTotal(0),
      m_sequentialPostsCurrent(0),
      m_embedImagesTotal(0),
      m_threadGateType("everybody"),
      m_running(false)
{
}

AtProtocolInterface::AccountData RecordOperator::account() const
{
    return AccountManager::getInstance()->getAccount(m_account.uuid);
}

QString RecordOperator::accountUuid() const
{
    return m_account.uuid;
}

void RecordOperator::setAccount(const QString &uuid)
{
    m_account.uuid = uuid;
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

void RecordOperator::setImages(const QStringList &images, const QStringList &alts)
{
    for (int i = 0; i < images.length(); i++) {
        EmbedImage e;
        e.path = images.at(i);
        if (i < alts.length()) {
            e.alt = alts.at(i);
        }
        m_embedImages.append(e);
    }
}

void RecordOperator::setVideo(const QString &video)
{
    QUrl url(video);
    if (url.isValid() && url.isLocalFile()) {
        m_embedVideo = url.toLocalFile();
    } else {
        m_embedVideo = video;
    }
}

void RecordOperator::setPostLanguages(const QStringList &langs)
{
    m_postLanguages = langs;
}

void RecordOperator::setExternalLink(const QString &uri, const QString &title,
                                     const QString &description, const QString &image_path)
{
    m_externalLinkUri = uri;
    m_externalLinkTitle = title;
    m_externalLinkDescription = description;
    m_embedImages.clear();
    if (!image_path.isEmpty()) {
        EmbedImage e;
        e.path = image_path;
        m_embedImages.append(e);
    }
}

void RecordOperator::setFeedGeneratorLink(const QString &uri, const QString &cid)
{
    m_feedGeneratorLinkUri = uri;
    m_feedGeneratorLinkCid = cid;
}

void RecordOperator::setSelfLabels(const QStringList &labels)
{
    m_selfLabels = labels;
}

// type = everybody || nobody || choice
// rules = mentioned || followed || at://uri
void RecordOperator::setThreadGate(const QString &type, const QStringList &rules)
{
    m_threadGateType = type;
    m_threadGateRules = rules;
}

// rule = disableRule
// uris = at://uri
void RecordOperator::setPostGate(const bool quote_enabled, const QStringList &uris)
{
    if (quote_enabled) {
        m_postGateEmbeddingRule.clear();
    } else {
        m_postGateEmbeddingRule = "disableRule";
    }
    m_postGateDetachedEmbeddingUris = uris;
}

void RecordOperator::clear()
{
    m_text.clear();
    m_replyParent = AtProtocolType::ComAtprotoRepoStrongRef::Main();
    m_replyRoot = AtProtocolType::ComAtprotoRepoStrongRef::Main();
    m_embedQuote = AtProtocolType::ComAtprotoRepoStrongRef::Main();
    m_embedImages.clear();
    m_embedImageBlobs.clear();
    m_externalLinkUri.clear();
    m_externalLinkTitle.clear();
    m_externalLinkDescription.clear();
    m_feedGeneratorLinkUri.clear();
    m_feedGeneratorLinkCid.clear();
    m_selfLabels.clear();
    m_sequentialPostsTotal = 0;
    m_sequentialPostsCurrent = 0;
    m_embedImagesTotal = 0;

    m_threadGateType = "everybody";
    m_threadGateRules.clear();

    m_postGateEmbeddingRule.clear();
    m_postGateDetachedEmbeddingUris.clear();
}

void RecordOperator::post()
{
    if (m_text.isEmpty())
        return;

    setRunning(true);

    if (m_embedImageBlobs.count() > 4 && m_sequentialPostsTotal == 0) {
        m_sequentialPostsTotal = static_cast<int>(m_embedImageBlobs.count() / 4)
                + ((m_embedImageBlobs.count() % 4) > 0 ? 1 : 0);
        QString progress_msg =
                QString("(%1/%2)").arg(m_sequentialPostsCurrent + 1).arg(m_sequentialPostsTotal);
        m_text += QString("\n%1").arg(progress_msg);
        setProgressMessage(tr("Posting ... %1").arg(progress_msg));
    } else if (m_sequentialPostsCurrent > 0) {
        QString progress_msg =
                QString("(%1/%2)").arg(m_sequentialPostsCurrent + 1).arg(m_sequentialPostsTotal);
        m_text = progress_msg;
        setProgressMessage(tr("Posting ... %1").arg(progress_msg));
    } else {
        setProgressMessage(tr("Posting ..."));
    }
    QList<AtProtocolType::Blob> embed_imageBlobs;
    for (int i = 0; i < 4; i++) {
        if (m_embedImageBlobs.isEmpty()) {
            break;
        }
        embed_imageBlobs.append(m_embedImageBlobs.first());
        m_embedImageBlobs.pop_front();
    }

    LexiconsTypeUnknown::makeFacets(
            this, account(), m_text,
            [=](const QList<AtProtocolType::AppBskyRichtextFacet::Main> &facets) {
                ComAtprotoRepoCreateRecordEx *create_record =
                        new ComAtprotoRepoCreateRecordEx(this);
                connect(create_record, &ComAtprotoRepoCreateRecordEx::finished, [=](bool success) {
                    if (success) {
                        QString last_post_uri = create_record->uri();
                        QString last_post_cid = create_record->cid();
                        bool ret = threadGate(
                                last_post_uri,
                                [=](bool success2, const QString &uri, const QString &cid) {
                                    Q_UNUSED(uri)
                                    Q_UNUSED(cid)
                                    postGate(last_post_uri,
                                             [=](bool success3, const QString &uri3,
                                                 const QString &cid3) {
                                                 Q_UNUSED(success3)
                                                 m_sequentialPostsCurrent++;
                                                 if (m_sequentialPostsCurrent
                                                     >= m_sequentialPostsTotal) {
                                                     setProgressMessage(QString());
                                                     emit finished(success2, uri3, cid3);
                                                     setRunning(false);
                                                 } else {
                                                     setPostGate(true, QStringList());
                                                     m_threadGateType = "everybody";
                                                     if (m_sequentialPostsCurrent == 1
                                                         && m_replyRoot.uri.isEmpty()) {
                                                         m_replyRoot.uri = last_post_uri;
                                                         m_replyRoot.cid = last_post_cid;
                                                     }
                                                     m_replyParent.uri = last_post_uri;
                                                     m_replyParent.cid = last_post_cid;
                                                     post();
                                                 }
                                             });
                                });
                        if (!ret) {
                            setProgressMessage(QString());
                            emit errorOccured(
                                    "InvalidThreadGateSetting",
                                    QString("Invalid thread gate setting.\ntype:%1\nrules:%2")
                                            .arg(m_threadGateType, m_threadGateRules.join(", ")));
                            emit finished(ret, QString(), QString());
                            setRunning(false);
                        }
                    } else {
                        setProgressMessage(QString());
                        emit errorOccured(create_record->errorCode(),
                                          create_record->errorMessage());
                        emit finished(success, QString(), QString());
                        setRunning(false);
                    }
                    create_record->deleteLater();
                });
                create_record->setAccount(account());
                create_record->setReply(m_replyParent.cid, m_replyParent.uri, m_replyRoot.cid,
                                        m_replyRoot.uri);
                create_record->setQuote(m_embedQuote.cid, m_embedQuote.uri);
                create_record->setImageBlobs(embed_imageBlobs);
                create_record->setFacets(facets);
                create_record->setPostLanguages(m_postLanguages);
                create_record->setExternalLink(m_externalLinkUri, m_externalLinkTitle,
                                               m_externalLinkDescription);
                create_record->setFeedGeneratorLink(m_feedGeneratorLinkUri, m_feedGeneratorLinkCid);
                create_record->setSelfLabels(m_selfLabels);
                create_record->post(m_text);
            });
}

void RecordOperator::postWithImages()
{
    if (m_embedImages.isEmpty()) {
        post();
        return;
    }

    setRunning(true);

    uploadBlob([=](bool success) {
        if (success) {
            post();
        } else {
            setProgressMessage(QString());
            emit finished(success, QString(), QString());
            setRunning(false);
        }
    });
}

void RecordOperator::postWithVideo()
{
    setRunning(true);

    setProgressMessage(tr("Uploading video ..."));

    AppBskyVideoGetUploadLimitsEx *limit = new AppBskyVideoGetUploadLimitsEx(this);
    connect(limit, &AppBskyVideoGetUploadLimitsEx::finished, [=](bool success_of_limit) {
        if (success_of_limit) {
            uploadVideoBlob([=](bool success_of_upload) {
                if (success_of_upload) {
                    post();
                } else {
                    setProgressMessage(QString());
                    emit finished(success_of_upload, QString(), QString()); // for Debug
                    setRunning(false);
                }
            });
        } else {
            setProgressMessage(QString());
            emit finished(success_of_limit, QString(), QString());
            setRunning(false);
        }
        limit->deleteLater();
    });
#ifdef QT_DEBUG
    limit->setEndpoint(m_videoEndpoint);
#endif
    limit->setAccount(account());
    limit->canUpload(m_embedVideo);
}

void RecordOperator::repost(const QString &cid, const QString &uri, const QString &via_cid,
                            const QString &via_uri)
{
    if (running())
        return;
    setRunning(true);

    setProgressMessage(tr("Repost ..."));

    ComAtprotoRepoCreateRecordEx *create_record = new ComAtprotoRepoCreateRecordEx(this);
    connect(create_record, &ComAtprotoRepoCreateRecordEx::finished, [=](bool success) {
        if (!success) {
            emit errorOccured(create_record->errorCode(), create_record->errorMessage());
        }
        setProgressMessage(QString());
        emit finished(success, create_record->uri(), create_record->cid());
        setRunning(false);

        // 成功なら、受け取ったデータでTLデータの更新をしないと値が大きくならない
        create_record->deleteLater();
    });
    create_record->setAccount(account());
    create_record->repost(cid, uri, via_cid, via_uri);
}

void RecordOperator::like(const QString &cid, const QString &uri, const QString &via_cid,
                          const QString &via_uri)
{
    if (running())
        return;
    setRunning(true);

    setProgressMessage(tr("Like ..."));

    ComAtprotoRepoCreateRecordEx *create_record = new ComAtprotoRepoCreateRecordEx(this);
    connect(create_record, &ComAtprotoRepoCreateRecordEx::finished, [=](bool success) {
        if (!success) {
            emit errorOccured(create_record->errorCode(), create_record->errorMessage());
        }
        setProgressMessage(QString());
        emit finished(success, create_record->uri(), create_record->cid());
        setRunning(false);

        // 成功なら、受け取ったデータでTLデータの更新をしないと値が大きくならない
        create_record->deleteLater();
    });
    create_record->setAccount(account());
    create_record->like(cid, uri, via_cid, via_uri);
}

void RecordOperator::follow(const QString &did)
{
    if (running())
        return;
    setRunning(true);

    setProgressMessage(tr("Follow ..."));

    ComAtprotoRepoCreateRecordEx *create_record = new ComAtprotoRepoCreateRecordEx(this);
    connect(create_record, &ComAtprotoRepoCreateRecordEx::finished, [=](bool success) {
        if (!success) {
            emit errorOccured(create_record->errorCode(), create_record->errorMessage());
        }
        setProgressMessage(QString());
        emit finished(success, create_record->uri(), create_record->cid());
        setRunning(false);
        create_record->deleteLater();
    });
    create_record->setAccount(account());
    create_record->follow(did);
}

void RecordOperator::mute(const QString &did)
{
    if (running())
        return;
    setRunning(true);

    setProgressMessage(tr("Mute ..."));

    AppBskyGraphMuteActor *mute = new AppBskyGraphMuteActor(this);
    connect(mute, &AppBskyGraphMuteActor::finished, [=](bool success) {
        if (success) {
        } else {
            emit errorOccured(mute->errorCode(), mute->errorMessage());
        }
        setProgressMessage(QString());
        emit finished(success, QString(), QString());
        setRunning(false);
        mute->deleteLater();
    });
    mute->setAccount(account());
    mute->muteActor(did);
}

void RecordOperator::block(const QString &did)
{
    if (running())
        return;
    setRunning(true);

    setProgressMessage(tr("Block ..."));

    ComAtprotoRepoCreateRecordEx *create_record = new ComAtprotoRepoCreateRecordEx(this);
    connect(create_record, &ComAtprotoRepoCreateRecordEx::finished, [=](bool success) {
        if (!success) {
            emit errorOccured(create_record->errorCode(), create_record->errorMessage());
        }
        setProgressMessage(QString());
        emit finished(success, create_record->uri(), create_record->cid());
        setRunning(false);
        create_record->deleteLater();
    });
    create_record->setAccount(account());
    create_record->block(did);
}

void RecordOperator::blockList(const QString &uri)
{
    if (running() || !uri.startsWith("at://"))
        return;
    setRunning(true);

    setProgressMessage(tr("Block list ..."));

    ComAtprotoRepoCreateRecordEx *create_record = new ComAtprotoRepoCreateRecordEx(this);
    connect(create_record, &ComAtprotoRepoCreateRecordEx::finished, [=](bool success) {
        if (!success) {
            emit errorOccured(create_record->errorCode(), create_record->errorMessage());
        }
        setProgressMessage(QString());
        emit finished(success, create_record->uri(), create_record->cid());
        setRunning(false);
        create_record->deleteLater();
    });
    create_record->setAccount(account());
    create_record->blockList(uri);
}

bool RecordOperator::list(const QString &name, const RecordOperator::ListPurpose purpose,
                          const QString &description)
{
    if (running())
        return false;

    uploadBlob([=](bool success) {
        if (success) {
            setProgressMessage(tr("Create list ... (%1)").arg(name));

            ComAtprotoRepoCreateRecordEx *create_record = new ComAtprotoRepoCreateRecordEx(this);
            connect(create_record, &ComAtprotoRepoCreateRecordEx::finished, [=](bool success) {
                if (!success) {
                    emit errorOccured(create_record->errorCode(), create_record->errorMessage());
                }
                setProgressMessage(QString());
                emit finished(success, create_record->uri(), create_record->cid());
                setRunning(false);
                create_record->deleteLater();
            });
            if (!m_embedImageBlobs.isEmpty()) {
                create_record->setImageBlobs(m_embedImageBlobs);
            } else {
                create_record->setImageBlobs(QList<AtProtocolType::Blob>());
            }
            create_record->setAccount(account());
            create_record->list(
                    name,
                    static_cast<AtProtocolInterface::ComAtprotoRepoCreateRecordEx::ListPurpose>(
                            purpose),
                    description);
        } else {
            setProgressMessage(QString());
            emit finished(success, QString(), QString());
            setRunning(false);
        }
    });

    setRunning(true);
    return running();
}

bool RecordOperator::listItem(const QString &uri, const QString &did)
{
    if (running())
        return false;
    setRunning(true);

    setProgressMessage(tr("Add to list ..."));

    ComAtprotoRepoCreateRecordEx *create_record = new ComAtprotoRepoCreateRecordEx(this);
    connect(create_record, &ComAtprotoRepoCreateRecordEx::finished, [=](bool success) {
        if (!success) {
            emit errorOccured(create_record->errorCode(), create_record->errorMessage());
        }
        setProgressMessage(QString());
        emit finished(success, create_record->uri(), create_record->cid());
        setRunning(false);
        create_record->deleteLater();
    });
    create_record->setAccount(account());
    create_record->listItem(uri, did);
    return true;
}

bool RecordOperator::bookmark(const QString &cid, const QString &uri)
{
    if (running())
        return false;
    setRunning(true);

    setProgressMessage(tr("Saving post ..."));

    AppBskyBookmarkCreateBookmark *book_mark = new AppBskyBookmarkCreateBookmark(this);
    connect(book_mark, &AppBskyBookmarkCreateBookmark::finished, [=](bool success) {
        if (!success) {
            emit errorOccured(book_mark->errorCode(), book_mark->errorMessage());
        }
        setProgressMessage(QString());
        emit finished(success, QString(), QString());
        setRunning(false);
        book_mark->deleteLater();
    });
    book_mark->setAccount(account());
    book_mark->createBookmark(uri, cid);

    return true;
}

void RecordOperator::deletePost(const QString &uri)
{
    if (running() || !uri.startsWith("at://"))
        return;
    setRunning(true);

    QString r_key = uri.split("/").last();

    setProgressMessage(tr("Delete post ..."));

    ComAtprotoRepoDeleteRecordEx *delete_record = new ComAtprotoRepoDeleteRecordEx(this);
    connect(delete_record, &ComAtprotoRepoDeleteRecordEx::finished, [=](bool success) {
        if (!success) {
            emit errorOccured(delete_record->errorCode(), delete_record->errorMessage());
        }
        setProgressMessage(QString());
        emit finished(success, QString(), QString());
        setRunning(false);
        delete_record->deleteLater();
    });
    delete_record->setAccount(account());
    delete_record->deletePost(r_key);
}

void RecordOperator::deleteLike(const QString &uri)
{
    if (running() || !uri.startsWith("at://"))
        return;
    setRunning(true);

    QString r_key = uri.split("/").last();

    setProgressMessage(tr("Delete like ..."));

    ComAtprotoRepoDeleteRecordEx *delete_record = new ComAtprotoRepoDeleteRecordEx(this);
    connect(delete_record, &ComAtprotoRepoDeleteRecordEx::finished, [=](bool success) {
        if (!success) {
            emit errorOccured(delete_record->errorCode(), delete_record->errorMessage());
        }
        setProgressMessage(QString());
        emit finished(success, QString(), QString());
        setRunning(false);
        delete_record->deleteLater();
    });
    delete_record->setAccount(account());
    delete_record->deleteLike(r_key);
}

void RecordOperator::deleteRepost(const QString &uri)
{
    if (running() || !uri.startsWith("at://"))
        return;
    setRunning(true);

    QString r_key = uri.split("/").last();

    setProgressMessage(tr("Delete repost ..."));

    ComAtprotoRepoDeleteRecordEx *delete_record = new ComAtprotoRepoDeleteRecordEx(this);
    connect(delete_record, &ComAtprotoRepoDeleteRecordEx::finished, [=](bool success) {
        if (!success) {
            emit errorOccured(delete_record->errorCode(), delete_record->errorMessage());
        }
        setProgressMessage(QString());
        emit finished(success, QString(), QString());
        setRunning(false);
        delete_record->deleteLater();
    });
    delete_record->setAccount(account());
    delete_record->deleteRepost(r_key);
}

void RecordOperator::deleteFollow(const QString &uri)
{
    if (running() || !uri.startsWith("at://"))
        return;
    setRunning(true);

    QString r_key = uri.split("/").last();

    setProgressMessage(tr("Unfollow ..."));

    ComAtprotoRepoDeleteRecordEx *delete_record = new ComAtprotoRepoDeleteRecordEx(this);
    connect(delete_record, &ComAtprotoRepoDeleteRecordEx::finished, [=](bool success) {
        if (!success) {
            emit errorOccured(delete_record->errorCode(), delete_record->errorMessage());
        }
        setProgressMessage(QString());
        emit finished(success, QString(), QString());
        setRunning(false);
        delete_record->deleteLater();
    });
    delete_record->setAccount(account());
    delete_record->unfollow(r_key);
}

void RecordOperator::deleteMute(const QString &did)
{
    if (running())
        return;
    setRunning(true);

    setProgressMessage(tr("Unmute ..."));

    AppBskyGraphUnmuteActor *unmute = new AppBskyGraphUnmuteActor(this);
    connect(unmute, &AppBskyGraphMuteActor::finished, [=](bool success) {
        if (success) {
        } else {
            emit errorOccured(unmute->errorCode(), unmute->errorMessage());
        }
        setProgressMessage(QString());
        emit finished(success, QString(), QString());
        setRunning(false);
        unmute->deleteLater();
    });
    unmute->setAccount(account());
    unmute->unmuteActor(did);
}

void RecordOperator::deleteBlock(const QString &uri)
{
    if (running() || !uri.startsWith("at://"))
        return;
    setRunning(true);

    QString r_key = uri.split("/").last();

    setProgressMessage(tr("Unblock ..."));

    ComAtprotoRepoDeleteRecordEx *delete_record = new ComAtprotoRepoDeleteRecordEx(this);
    connect(delete_record, &ComAtprotoRepoDeleteRecordEx::finished, [=](bool success) {
        if (!success) {
            emit errorOccured(delete_record->errorCode(), delete_record->errorMessage());
        }
        setProgressMessage(QString());
        emit finished(success, QString(), QString());
        setRunning(false);
        delete_record->deleteLater();
    });
    delete_record->setAccount(account());
    delete_record->deleteBlock(r_key);
}

void RecordOperator::deleteBlockList(const QString &uri)
{
    if (running() || !uri.startsWith("at://"))
        return;
    setRunning(true);

    QString r_key = uri.split("/").last();

    setProgressMessage(tr("Unblock block list ..."));

    ComAtprotoRepoDeleteRecordEx *delete_record = new ComAtprotoRepoDeleteRecordEx(this);
    connect(delete_record, &ComAtprotoRepoDeleteRecordEx::finished, [=](bool success) {
        if (!success) {
            emit errorOccured(delete_record->errorCode(), delete_record->errorMessage());
        }
        setProgressMessage(QString());
        emit finished(success, QString(), QString());
        setRunning(false);
        delete_record->deleteLater();
    });
    delete_record->setAccount(account());
    delete_record->deleteBlockList(r_key);
}

bool RecordOperator::deleteList(const QString &uri)
{
    if (running() || !uri.startsWith("at://"))
        return false;

    setProgressMessage(tr("Delete list ..."));

    m_listItemCursor = "__start__";
    m_listItems.clear();
    bool res = getAllListItems(uri, [=](bool success3) {
        if (success3) {
            deleteAllListItems([=](bool success2) {
                if (success2) {
                    QString r_key = uri.split("/").last();

                    ComAtprotoRepoDeleteRecordEx *delete_record =
                            new ComAtprotoRepoDeleteRecordEx(this);
                    connect(delete_record, &ComAtprotoRepoDeleteRecordEx::finished,
                            [=](bool success) {
                                if (!success) {
                                    emit errorOccured(delete_record->errorCode(),
                                                      delete_record->errorMessage());
                                }
                                setProgressMessage(QString());
                                emit finished(success, QString(), QString());
                                setRunning(false);
                                delete_record->deleteLater();
                            });
                    delete_record->setAccount(account());
                    delete_record->deleteList(r_key);
                } else {
                    setProgressMessage(QString());
                    emit errorOccured(
                            QStringLiteral("FailDeleteList"),
                            QStringLiteral(
                                    "An error occurred while deleting a user in the list.(2)"));
                    emit finished(false, QString(), QString());
                    setRunning(false);
                }
            });
        } else {
            setProgressMessage(QString());
            emit errorOccured(QStringLiteral("FailDeleteList"),
                              QStringLiteral("An error occurred while retrieving user information "
                                             "registered in the list.(2)"));
            emit finished(false, QString(), QString());
            setRunning(false);
        }
    });
    if (!res) {
        setProgressMessage(QString());
        emit errorOccured(QStringLiteral("FailDeleteList"),
                          QStringLiteral("An error occurred while retrieving user information "
                                         "registered in the list.(1)"));
    }
    setRunning(res);
    return running();
}

bool RecordOperator::deleteListItem(const QString &uri)
{
    if (running() || !uri.startsWith("at://"))
        return false;
    setRunning(true);

    QString r_key = uri.split("/").last();

    setProgressMessage(tr("Delete list item ..."));

    ComAtprotoRepoDeleteRecordEx *delete_record = new ComAtprotoRepoDeleteRecordEx(this);
    connect(delete_record, &ComAtprotoRepoDeleteRecordEx::finished, [=](bool success) {
        if (!success) {
            emit errorOccured(delete_record->errorCode(), delete_record->errorMessage());
        }
        setProgressMessage(QString());
        emit finished(success, QString(), QString());
        setRunning(false);
        delete_record->deleteLater();
    });
    delete_record->setAccount(account());
    delete_record->deleteListItem(r_key);
    return true;
}

bool RecordOperator::deleteBookmark(const QString &uri)
{
    if (running())
        return false;
    setRunning(true);

    setProgressMessage(tr("Delete saved post ..."));

    AppBskyBookmarkDeleteBookmark *book_mark = new AppBskyBookmarkDeleteBookmark(this);
    connect(book_mark, &AppBskyBookmarkDeleteBookmark::finished, [=](bool success) {
        if (!success) {
            emit errorOccured(book_mark->errorCode(), book_mark->errorMessage());
        }
        setProgressMessage(QString());
        emit finished(success, QString(), QString());
        setRunning(false);
        book_mark->deleteLater();
    });
    book_mark->setAccount(account());
    book_mark->deleteBookmark(uri);

    return true;
}

void RecordOperator::updateProfile(const QString &avatar_url, const QString &banner_url,
                                   const QString &description, const QString &display_name,
                                   const QString &pronouns, const QString &website)
{
    if (running())
        return;
    setRunning(true);

    setProgressMessage(tr("Update profile ... (%1)").arg(account().handle));

    QStringList images;
    QStringList alts;
    if (!avatar_url.isEmpty() && QUrl(avatar_url).isLocalFile()) {
        images.append(avatar_url);
        alts.append(QStringLiteral("avatar"));
    }
    if (!banner_url.isEmpty() && QUrl(banner_url).isLocalFile()) {
        images.append(banner_url);
        alts.append(QStringLiteral("banner"));
    }
    setImages(images, alts);

    ComAtprotoRepoGetRecordEx *old_profile = new ComAtprotoRepoGetRecordEx(this);
    connect(old_profile, &ComAtprotoRepoGetRecordEx::finished, [=](bool success1) {
        if (success1) {
            AppBskyActorProfile::Main old_record =
                    LexiconsTypeUnknown::fromQVariant<AppBskyActorProfile::Main>(
                            old_profile->value());
            QString old_cid = old_profile->cid();
            uploadBlob([=](bool success2) {
                if (success2) {
                    AtProtocolType::Blob avatar = old_record.avatar;
                    AtProtocolType::Blob banner = old_record.banner;
                    for (const auto &blob : std::as_const(m_embedImageBlobs)) {
                        if (blob.alt == "avatar") {
                            avatar = blob;
                            avatar.alt.clear();
                        } else if (blob.alt == "banner") {
                            banner = blob;
                            banner.alt.clear();
                        }
                    }
                    ComAtprotoRepoPutRecordEx *new_profile = new ComAtprotoRepoPutRecordEx(this);
                    connect(new_profile, &ComAtprotoRepoPutRecordEx::finished, [=](bool success3) {
                        if (!success3) {
                            emit errorOccured(new_profile->errorCode(),
                                              new_profile->errorMessage());
                        }
                        setProgressMessage(QString());
                        emit finished(success3, QString(), QString());
                        setRunning(false);
                        new_profile->deleteLater();
                    });
                    new_profile->setAccount(account());
                    new_profile->profile(avatar, banner, description, display_name, pronouns,
                                         website, old_record.pinnedPost, old_cid);
                } else {
                    setProgressMessage(QString());
                    emit finished(false, QString(), QString());
                    setRunning(false);
                }
            });
        } else {
            setProgressMessage(QString());
            emit errorOccured(old_profile->errorCode(), old_profile->errorMessage());
            emit finished(false, QString(), QString());
            setRunning(false);
        }
        old_profile->deleteLater();
    });
    old_profile->setAccount(account());
    old_profile->profile(account().did);
}

void RecordOperator::updatePostPinning(const QString &post_uri, const QString &post_cid)
{
    if (running())
        return;
    setRunning(true);

    setProgressMessage(tr("Update post pinning ... (%1)").arg(account().handle));

    ComAtprotoRepoGetRecordEx *old_profile = new ComAtprotoRepoGetRecordEx(this);
    connect(old_profile, &ComAtprotoRepoGetRecordEx::finished, [=](bool success1) {
        if (success1) {
            AppBskyActorProfile::Main old_record =
                    LexiconsTypeUnknown::fromQVariant<AppBskyActorProfile::Main>(
                            old_profile->value());
            QString old_cid = old_profile->cid();
            ComAtprotoRepoPutRecordEx *new_profile = new ComAtprotoRepoPutRecordEx(this);
            connect(new_profile, &ComAtprotoRepoPutRecordEx::finished, [=](bool success3) {
                if (!success3) {
                    emit errorOccured(new_profile->errorCode(), new_profile->errorMessage());
                }
                setProgressMessage(QString());
                emit finished(success3, QString(), QString());
                setRunning(false);
                new_profile->deleteLater();
            });
            ComAtprotoRepoStrongRef::Main pinned_post;
            pinned_post.uri = post_uri;
            pinned_post.cid = post_cid;
            new_profile->setAccount(account());
            new_profile->profile(old_record.avatar, old_record.banner, old_record.description,
                                 old_record.displayName, old_record.pronouns, old_record.website,
                                 pinned_post, old_cid);
        } else {
            setProgressMessage(QString());
            emit errorOccured(old_profile->errorCode(), old_profile->errorMessage());
            emit finished(false, QString(), QString());
            setRunning(false);
        }
        old_profile->deleteLater();
    });
    old_profile->setAccount(account());
    old_profile->profile(account().did);
}

void RecordOperator::updateList(const QString &uri, const QString &avatar_url,
                                const QString &description, const QString &name)
{
    if (running() || !uri.startsWith("at://"))
        return;
    setRunning(true);

    QString r_key = uri.split("/").last();

    setProgressMessage(tr("Update list ... (%1)").arg(name));

    QStringList images;
    QStringList alts;
    if (!avatar_url.isEmpty() && QUrl(avatar_url).isLocalFile()) {
        images.append(avatar_url);
        alts.append(QStringLiteral("avatar"));
    }
    setImages(images, alts);

    ComAtprotoRepoGetRecordEx *old_list = new ComAtprotoRepoGetRecordEx(this);
    connect(old_list, &ComAtprotoRepoGetRecordEx::finished, [=](bool success1) {
        if (success1) {
            AppBskyGraphList::Main old_record =
                    LexiconsTypeUnknown::fromQVariant<AppBskyGraphList::Main>(old_list->value());
            uploadBlob([=](bool success2) {
                if (success2) {
                    AtProtocolType::Blob avatar = old_record.avatar;
                    for (const auto &blob : std::as_const(m_embedImageBlobs)) {
                        if (blob.alt == "avatar") {
                            avatar = blob;
                            avatar.alt.clear();
                        }
                    }
                    ComAtprotoRepoPutRecordEx *new_list = new ComAtprotoRepoPutRecordEx(this);
                    connect(new_list, &ComAtprotoRepoPutRecordEx::finished, [=](bool success3) {
                        if (!success3) {
                            emit errorOccured(new_list->errorCode(), new_list->errorMessage());
                        }
                        setProgressMessage(QString());
                        emit finished(success3, QString(), QString());
                        setRunning(false);
                        new_list->deleteLater();
                    });
                    new_list->setAccount(account());
                    new_list->list(avatar, old_record.purpose, description, name, r_key);
                } else {
                    setProgressMessage(QString());
                    emit finished(false, QString(), QString());
                    setRunning(false);
                }
            });
        } else {
            setProgressMessage(QString());
            emit errorOccured(old_list->errorCode(), old_list->errorMessage());
            emit finished(false, QString(), QString());
            setRunning(false);
        }
        old_list->deleteLater();
    });
    old_list->setAccount(account());
    old_list->list(account().did, r_key);
}

void RecordOperator::updateThreadGate(const QString &uri, const QString &threadgate_uri,
                                      const QString &type, const QStringList &rules)
{
    if (running() || !uri.startsWith("at://"))
        return;
    setRunning(true);

    QString r_key = AtProtocolType::LexiconsTypeUnknown::extractRkey(threadgate_uri);

    setProgressMessage(tr("Update who can reply ..."));

    ComAtprotoRepoDeleteRecordEx *delete_record = new ComAtprotoRepoDeleteRecordEx(this);
    connect(delete_record, &ComAtprotoRepoDeleteRecordEx::finished, [=](bool success) {
        // レコードがないときはエラーになるので継続

        if (type == "everybody") {
            // delete only
            setProgressMessage(QString());
            setRunning(false);
            emit finished(success, QString(), QString());
        } else {
            // update
            setThreadGate(type, rules);
            bool ret = threadGate(uri, [=](bool success, const QString &uri, const QString &cid) {
                setProgressMessage(QString());
                emit finished(success, uri, cid);
                setRunning(false);
            });
            if (!ret) {
                setProgressMessage(QString());
                emit errorOccured("InvalidThreadGateSetting",
                                  QString("Invalid thread gate setting.\ntype:%1\nrules:%2")
                                          .arg(m_threadGateType, m_threadGateRules.join(", ")));
                emit finished(ret, QString(), QString());
                setRunning(false);
            }
        }
        delete_record->deleteLater();
    });
    delete_record->setAccount(account());
    delete_record->deleteThreadGate(r_key);
}

void RecordOperator::updateQuoteEnabled(const QString &uri, bool enabled)
{
    if (uri.isEmpty() || !uri.startsWith("at://")) {
        emit finished(false, QString(), QString());
        return;
    }
    setProgressMessage(tr("Update quote status ..."));

    QString target_rkey = AtProtocolType::LexiconsTypeUnknown::extractRkey(uri);
    ComAtprotoRepoGetRecordEx *record = new ComAtprotoRepoGetRecordEx(this);
    connect(record, &ComAtprotoRepoGetRecordEx::finished, this, [=](bool success) {
        qDebug().noquote() << __func__ << "get post_gate" << success << record->value().isValid();
        // レコードがないときはエラーになるので継続
        AppBskyFeedPostgate::Main old_record =
                LexiconsTypeUnknown::fromQVariant<AppBskyFeedPostgate::Main>(record->value());
        AppBskyFeedPostgate::MainEmbeddingRulesType rule =
                AppBskyFeedPostgate::MainEmbeddingRulesType::none;
        if (!enabled) {
            rule = AppBskyFeedPostgate::MainEmbeddingRulesType::embeddingRules_DisableRule;
        }

        ComAtprotoRepoPutRecordEx *put = new ComAtprotoRepoPutRecordEx(this);
        connect(put, &ComAtprotoRepoPutRecordEx::finished, this, [=](bool success2) {
            qDebug().noquote() << __func__ << "put post gate" << success2 << "enabled:" << enabled;
            setProgressMessage(QString());
            if (!success2) {
                emit errorOccured(put->errorCode(), put->errorMessage());
            }
            emit finished(success2, put->uri(), put->cid());
            put->deleteLater();
        });
        put->setAccount(account());
        put->postGate(uri, rule, old_record.detachedEmbeddingUris);

        record->deleteLater();
    });
    record->setAccount(account());
    record->postGate(account().did, target_rkey);
}

void RecordOperator::updateDetachedStatusOfQuote(bool detached, QString target_uri,
                                                 QString detach_uri)
{
    // 引用のデタッチのみ更新
    if (detach_uri.isEmpty() || !detach_uri.startsWith("at://") || target_uri.isEmpty()
        || !target_uri.startsWith("at://")) {
        emit finished(false, QString(), QString());
        return;
    }

    setProgressMessage(tr("Update quote status ..."));

    QString target_rkey = AtProtocolType::LexiconsTypeUnknown::extractRkey(target_uri);
    ComAtprotoRepoGetRecordEx *record = new ComAtprotoRepoGetRecordEx(this);
    connect(record, &ComAtprotoRepoGetRecordEx::finished, this, [=](bool success) {
        qDebug().noquote() << __func__ << "get post_gate" << success << record->value().isValid();
        // レコードがないときはエラーになるので継続

        AppBskyFeedPostgate::Main old_record =
                LexiconsTypeUnknown::fromQVariant<AppBskyFeedPostgate::Main>(record->value());

        AppBskyFeedPostgate::MainEmbeddingRulesType rule =
                AppBskyFeedPostgate::MainEmbeddingRulesType::none;
        if (!old_record.embeddingRules_DisableRule.isEmpty()) {
            rule = AppBskyFeedPostgate::MainEmbeddingRulesType::embeddingRules_DisableRule;
        }
        if (detached) {
            // re-attach
            qDebug() << __func__ << "re-attach" << detach_uri;
            if (old_record.detachedEmbeddingUris.contains(detach_uri)) {
                old_record.detachedEmbeddingUris.removeAll(detach_uri);
            }
        } else {
            // detach
            qDebug() << __func__ << "detach" << detach_uri;
            if (!old_record.detachedEmbeddingUris.contains(detach_uri)) {
                old_record.detachedEmbeddingUris.append(detach_uri);
            }
        }

        ComAtprotoRepoPutRecordEx *put = new ComAtprotoRepoPutRecordEx(this);
        connect(put, &ComAtprotoRepoPutRecordEx::finished, this, [=](bool success2) {
            qDebug().noquote() << __func__ << "put post gate" << success2
                               << "quoted:" << detach_uri;
            setProgressMessage(QString());
            if (!success2) {
                emit errorOccured(put->errorCode(), put->errorMessage());
            }
            emit finished(success2, put->uri(), put->cid());
            put->deleteLater();
        });
        put->setAccount(account());
        put->postGate(target_uri, rule, old_record.detachedEmbeddingUris);

        record->deleteLater();
    });
    record->setAccount(account());
    record->postGate(account().did, target_rkey);
}

void RecordOperator::updateActivitySubscription(const QString &did, bool post, bool reply)
{
    if (running())
        return;
    setRunning(true);

    QJsonObject subscription_json;
    subscription_json.insert(QStringLiteral("post"), post);
    subscription_json.insert(QStringLiteral("reply"), reply);

    AppBskyNotificationPutActivitySubscription *subscription =
            new AppBskyNotificationPutActivitySubscription(this);
    connect(subscription, &AppBskyNotificationPutActivitySubscription::finished, [=](bool success) {
        if (success) {
            qDebug().noquote() << "Activity subscription updated for" << did << "post:" << post
                               << "reply:" << reply;
        } else {
            emit errorOccured(subscription->errorCode(), subscription->errorMessage());
        }
        emit finished(success, QString(), QString());
        setRunning(false);
        subscription->deleteLater();
    });
    subscription->setAccount(account());
    subscription->putActivitySubscription(did, subscription_json);
}

void RecordOperator::updateNotificationDeclaration(const QString &declaration)
{
    if (running())
        return;
    setRunning(true);

    ComAtprotoRepoPutRecordEx *create_record = new ComAtprotoRepoPutRecordEx(this);
    connect(create_record, &ComAtprotoRepoPutRecordEx::finished, [=](bool success) {
        if (!success) {
            emit errorOccured(create_record->errorCode(), create_record->errorMessage());
        }
        emit finished(success, QString(), QString());
        setRunning(false);
        create_record->deleteLater();
    });
    create_record->setAccount(account());
    create_record->notificationDeclaration(declaration);
}

void RecordOperator::requestNotificationDeclaration()
{
    if (running())
        return;
    setRunning(true);

    ComAtprotoRepoGetRecordEx *record = new ComAtprotoRepoGetRecordEx(this);
    connect(record, &ComAtprotoRepoGetRecordEx::finished, this, [=](bool success) {
        QString declaration;
        if (success) {
            qDebug().noquote() << "requestNotificationDeclaration:" << record->replyJson();
            AppBskyNotificationDeclaration::Main notification_declaration =
                    LexiconsTypeUnknown::fromQVariant<AppBskyNotificationDeclaration::Main>(
                            record->value());
            declaration = notification_declaration.allowSubscriptions;
        } else {
            emit errorOccured(record->errorCode(), record->errorMessage());
        }
        setRunning(false);
        emit finishedRequestNotificationDeclaration(success, declaration);
        record->deleteLater();
    });
    record->setAccount(account());
    record->notificationDeclaration(account().did);
}

void RecordOperator::requestPostGate(const QString &uri)
{
    QString rkey = AtProtocolType::LexiconsTypeUnknown::extractRkey(uri);
    if (rkey.isEmpty()) {
        emit finishedRequestPostGate(false, false, QStringList());
        return;
    }

    ComAtprotoRepoGetRecordEx *record = new ComAtprotoRepoGetRecordEx(this);
    connect(record, &ComAtprotoRepoGetRecordEx::finished, this, [=](bool success) {
        bool enabled = true;
        QStringList uris;
        if (success) {
            AppBskyFeedPostgate::Main postgate =
                    LexiconsTypeUnknown::fromQVariant<AppBskyFeedPostgate::Main>(record->value());
            enabled = postgate.embeddingRules_DisableRule.isEmpty();
            uris = postgate.detachedEmbeddingUris;
        } else {
            // 未設定状態
        }
        emit finishedRequestPostGate(success, enabled, uris);
        record->deleteLater();
    });
    record->setAccount(account());
    record->postGate(account().did, rkey);
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

void RecordOperator::uploadBlob(std::function<void(bool)> callback)
{
    if (m_embedImages.isEmpty()) {
        callback(true);
        return;
    }

    if (m_embedImagesTotal == 0) {
        m_embedImagesTotal = m_embedImages.count();
    }
    setProgressMessage(tr("Uploading images ... (%1/%2)")
                               .arg(m_embedImagesTotal - m_embedImages.count() + 1)
                               .arg(m_embedImagesTotal));
    QString path = QUrl(m_embedImages.first().path).toLocalFile();
    QString alt = m_embedImages.first().alt;
    m_embedImages.removeFirst();

    ComAtprotoRepoUploadBlob *upload_blob = new ComAtprotoRepoUploadBlob(this);
    connect(upload_blob, &ComAtprotoRepoUploadBlob::finished, [=](bool success) {
        if (success) {
            qDebug().noquote() << "Uploaded blob" << upload_blob->cid() << upload_blob->mimeType()
                               << upload_blob->size();

            AtProtocolType::Blob blob;
            blob.cid = upload_blob->cid();
            blob.mimeType = upload_blob->mimeType();
            blob.size = upload_blob->size();
            blob.alt = alt;
            blob.aspect_ratio = upload_blob->aspectRatio();
            m_embedImageBlobs.append(blob);

            if (m_embedImages.isEmpty()) {
                callback(true);
            } else {
                uploadBlob(callback);
            }
        } else {
            emit errorOccured(upload_blob->errorCode(), upload_blob->errorMessage());
            callback(false);
        }
        upload_blob->deleteLater();
    });
    upload_blob->setAccount(account());
    upload_blob->uploadBlob(path);
}

void RecordOperator::uploadVideoBlob(std::function<void(bool)> callback)
{
    AppBskyVideoUploadVideoEx *upload = new AppBskyVideoUploadVideoEx(this);
    connect(upload, &AppBskyVideoUploadVideoEx::finished, [=](bool success) {
        if (success) {
            qDebug().noquote() << "Uploaded video blob" << upload->cid() << upload->mimeType()
                               << upload->size();

            AtProtocolType::Blob blob;
            blob.cid = upload->cid();
            blob.mimeType = upload->mimeType();
            blob.size = upload->size();
            blob.aspect_ratio = upload->aspectRatio();
            m_embedImageBlobs.append(blob);

        } else {
            emit errorOccured(upload->errorCode(), upload->errorMessage());
        }
        callback(success);
        upload->deleteLater();
    });
#ifdef QT_DEBUG
    upload->setEndpoint(m_videoEndpoint);
#endif
    upload->setAccount(account());
    upload->uploadVideo(m_embedVideo);
}

bool RecordOperator::getAllListItems(const QString &list_uri, std::function<void(bool)> callback)
{
    // 使う前にm_listItemsをクリアすること！

    if (m_listItemCursor.isEmpty()) {
        callback(true);
        return true;
    }
    QString cursor = m_listItemCursor;
    if (m_listItemCursor == "__start__") {
        cursor.clear();
        m_listItemCursor.clear();
    }

    AtProtocolInterface::ComAtprotoRepoListRecordsEx *list =
            new AtProtocolInterface::ComAtprotoRepoListRecordsEx(this);
    connect(list, &AtProtocolInterface::ComAtprotoRepoListRecordsEx::finished, [=](bool success) {
        if (success) {
            m_listItemCursor = list->cursor();
            if (list->recordsList().isEmpty())
                m_listItemCursor.clear();
            for (const auto &item : list->recordsList()) {
                AppBskyGraphListitem::Main record =
                        AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                                AppBskyGraphListitem::Main>(item.value);
                if (record.list == list_uri) {
                    qDebug().noquote() << "DELETE ITEM:" << item.uri << " in " << list_uri;
                    m_listItems.append(item.uri);
                }
            }
            if (!getAllListItems(list_uri, callback)) {
                callback(false);
            }
        } else {
            emit errorOccured(list->errorCode(), list->errorMessage());
            callback(false);
        }
        list->deleteLater();
    });
    list->setAccount(account());
    list->listListItems(account().did, cursor);

    return true;
}

void RecordOperator::deleteAllListItems(std::function<void(bool)> callback)
{
    if (m_listItems.isEmpty()) {
        callback(true);
        return;
    }

    setProgressMessage(tr("Delete list item ... (%1)").arg(m_listItems.count()));

    QString r_key = m_listItems.front().split("/").last();
    m_listItems.pop_front();

    ComAtprotoRepoDeleteRecordEx *delete_record = new ComAtprotoRepoDeleteRecordEx(this);
    connect(delete_record, &ComAtprotoRepoDeleteRecordEx::finished, [=](bool success) {
        if (success) {
            deleteAllListItems(callback);
        } else {
            callback(false);
        }
        delete_record->deleteLater();
    });
    delete_record->setAccount(account());
    delete_record->deleteListItem(r_key);
}

bool RecordOperator::threadGate(
        const QString &uri, std::function<void(bool, const QString &, const QString &)> callback)
{
    AtProtocolType::ThreadGateType type = AtProtocolType::ThreadGateType::Everybody;
    QList<AtProtocolType::ThreadGateAllow> rules;
    AtProtocolType::ThreadGateAllow rule;

    if (m_threadGateType == "everybody") {
        qDebug().noquote() << "Not set thread gate.";
        callback(true, QString(), QString());
        return true;
    } else if (m_threadGateType == "nobody") {
        type = AtProtocolType::ThreadGateType::Nobody;
    } else {
        for (const auto &cmd : m_threadGateRules) {
            if (cmd == "mentioned") {
                type = AtProtocolType::ThreadGateType::Choice;
                rule.type = AtProtocolType::ThreadGateAllowType::Mentioned;
                rule.uri.clear();
                rules.append(rule);
            } else if (cmd == "followed") {
                type = AtProtocolType::ThreadGateType::Choice;
                rule.type = AtProtocolType::ThreadGateAllowType::Followed;
                rule.uri.clear();
                rules.append(rule);
            } else if (cmd == "follower") {
                type = AtProtocolType::ThreadGateType::Choice;
                rule.type = AtProtocolType::ThreadGateAllowType::Follower;
                rule.uri.clear();
                rules.append(rule);
            } else if (cmd.startsWith("at://")) {
                type = AtProtocolType::ThreadGateType::Choice;
                rule.type = AtProtocolType::ThreadGateAllowType::List;
                rule.uri = cmd;
                rules.append(rule);
            }
        }
        if (m_threadGateRules.isEmpty()) {
            return false;
        }
    }

    ComAtprotoRepoCreateRecordEx *create_record = new ComAtprotoRepoCreateRecordEx(this);
    connect(create_record, &ComAtprotoRepoCreateRecordEx::finished, [=](bool success) {
        if (!success) {
            emit errorOccured(create_record->errorCode(), create_record->errorMessage());
        }
        callback(success, create_record->uri(), create_record->cid());
        create_record->deleteLater();
    });
    create_record->setAccount(account());
    create_record->threadGate(uri, type, rules);
    return true;
}

void RecordOperator::postGate(const QString &uri,
                              std::function<void(bool, const QString &, const QString &)> callback)
{
    const QString rule = "disableRule";

    if (m_postGateEmbeddingRule != rule && m_postGateDetachedEmbeddingUris.isEmpty()) {
        qDebug().noquote() << "Not set post gate.";
        callback(true, QString(), QString());
        return;
    }

    AtProtocolType::AppBskyFeedPostgate::MainEmbeddingRulesType type =
            AtProtocolType::AppBskyFeedPostgate::MainEmbeddingRulesType::none;
    if (m_postGateEmbeddingRule == rule) {
        type = AtProtocolType::AppBskyFeedPostgate::MainEmbeddingRulesType::
                embeddingRules_DisableRule;
    }

    ComAtprotoRepoCreateRecordEx *create_record = new ComAtprotoRepoCreateRecordEx(this);
    connect(create_record, &ComAtprotoRepoCreateRecordEx::finished, [=](bool success) {
        if (!success) {
            emit errorOccured(create_record->errorCode(), create_record->errorMessage());
        }
        callback(success, create_record->uri(), create_record->cid());
        create_record->deleteLater();
    });
    create_record->setAccount(account());
    create_record->postGate(uri, type, m_postGateDetachedEmbeddingUris);
}

#ifdef QT_DEBUG
void RecordOperator::setVideoEndpoint(const QString &newVideoEndpoint)
{
    m_videoEndpoint = newVideoEndpoint;
}
#endif

QString RecordOperator::progressMessage() const
{
    return m_progressMessage;
}

void RecordOperator::setProgressMessage(const QString &newProgressMessage)
{
    if (m_progressMessage == newProgressMessage)
        return;
    m_progressMessage = newProgressMessage;
    emit progressMessageChanged();
}

QString RecordOperator::handle() const
{
    return account().handle;
}
