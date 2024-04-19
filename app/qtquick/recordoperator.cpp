#include "recordoperator.h"
#include "atprotocol/com/atproto/repo/comatprotorepouploadblob.h"
#include "atprotocol/app/bsky/actor/appbskyactorgetprofiles.h"
#include "atprotocol/app/bsky/graph/appbskygraphmuteactor.h"
#include "atprotocol/app/bsky/graph/appbskygraphunmuteactor.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "extension/com/atproto/repo/comatprotorepodeleterecordex.h"
#include "extension/com/atproto/repo/comatprotorepogetrecordex.h"
#include "extension/com/atproto/repo/comatprotorepolistrecordsex.h"
#include "extension/com/atproto/repo/comatprotorepoputrecordex.h"
#include "systemtool.h"

#include <QTimer>

using AtProtocolInterface::AppBskyActorGetProfiles;
using AtProtocolInterface::AppBskyGraphMuteActor;
using AtProtocolInterface::AppBskyGraphUnmuteActor;
using AtProtocolInterface::ComAtprotoRepoCreateRecordEx;
using AtProtocolInterface::ComAtprotoRepoDeleteRecordEx;
using AtProtocolInterface::ComAtprotoRepoGetRecordEx;
using AtProtocolInterface::ComAtprotoRepoListRecordsEx;
using AtProtocolInterface::ComAtprotoRepoPutRecordEx;
using AtProtocolInterface::ComAtprotoRepoUploadBlob;
using namespace AtProtocolType;

struct MentionData
{
    int start = -1;
    int end = -1;
};

RecordOperator::RecordOperator(QObject *parent)
    : QObject { parent },
      m_sequentialPostsTotal(0),
      m_sequentialPostsCurrent(0),
      m_embedImagesTotal(0),
      m_running(false)
{
    m_rxFacet = QRegularExpression(QString("(?:%1)|(?:%2)|(?:%3)")
                                           .arg(REG_EXP_URL, REG_EXP_MENTION)
                                           .arg(REG_EXP_HASH_TAG));
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
    EmbedImage e;
    e.path = image_path;
    m_embedImages.append(e);
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

void RecordOperator::clear()
{
    m_text.clear();
    m_replyParent = AtProtocolType::ComAtprotoRepoStrongRef::Main();
    m_replyRoot = AtProtocolType::ComAtprotoRepoStrongRef::Main();
    m_embedQuote = AtProtocolType::ComAtprotoRepoStrongRef::Main();
    m_embedImages.clear();
    m_embedImageBlobs.clear();
    m_facets.clear();
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

    makeFacets(m_text, [=]() {
        ComAtprotoRepoCreateRecordEx *create_record = new ComAtprotoRepoCreateRecordEx(this);
        connect(create_record, &ComAtprotoRepoCreateRecordEx::finished, [=](bool success) {
            if (success) {
                QString last_post_uri = create_record->uri();
                QString last_post_cid = create_record->cid();
                bool ret = threadGate(create_record->uri(),
                                      [=](bool success2, const QString &uri, const QString &cid) {
                                          m_sequentialPostsCurrent++;
                                          if (m_sequentialPostsCurrent >= m_sequentialPostsTotal) {
                                              setProgressMessage(QString());
                                              emit finished(success2, uri, cid);
                                              setRunning(false);
                                          } else {
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
                if (!ret) {
                    setProgressMessage(QString());
                    emit errorOccured("InvalidThreadGateSetting",
                                      QString("Invalid thread gate setting.\ntype:%1\nrules:%2")
                                              .arg(m_threadGateType, m_threadGateRules.join(", ")));
                    emit finished(ret, QString(), QString());
                    setRunning(false);
                }
            } else {
                setProgressMessage(QString());
                emit errorOccured(create_record->errorCode(), create_record->errorMessage());
                emit finished(success, QString(), QString());
                setRunning(false);
            }
            create_record->deleteLater();
        });
        create_record->setAccount(m_account);
        create_record->setReply(m_replyParent.cid, m_replyParent.uri, m_replyRoot.cid,
                                m_replyRoot.uri);
        create_record->setQuote(m_embedQuote.cid, m_embedQuote.uri);
        create_record->setImageBlobs(embed_imageBlobs);
        create_record->setFacets(m_facets);
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
    if (m_embedImages.isEmpty())
        return;

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

void RecordOperator::repost(const QString &cid, const QString &uri)
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
    create_record->setAccount(m_account);
    create_record->repost(cid, uri);
}

void RecordOperator::like(const QString &cid, const QString &uri)
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
    create_record->setAccount(m_account);
    create_record->like(cid, uri);
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
    create_record->setAccount(m_account);
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
    mute->setAccount(m_account);
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
    create_record->setAccount(m_account);
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
    create_record->setAccount(m_account);
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
            create_record->setImageBlobs(m_embedImageBlobs);
            create_record->setAccount(m_account);
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
    create_record->setAccount(m_account);
    create_record->listItem(uri, did);
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
    delete_record->setAccount(m_account);
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
    delete_record->setAccount(m_account);
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
    delete_record->setAccount(m_account);
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
    delete_record->setAccount(m_account);
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
    unmute->setAccount(m_account);
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
    delete_record->setAccount(m_account);
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
    delete_record->setAccount(m_account);
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
                    delete_record->setAccount(m_account);
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
    delete_record->setAccount(m_account);
    delete_record->deleteListItem(r_key);
    return true;
}

void RecordOperator::updateProfile(const QString &avatar_url, const QString &banner_url,
                                   const QString &description, const QString &display_name)
{
    if (running())
        return;
    setRunning(true);

    setProgressMessage(tr("Update profile ... (%1)").arg(m_account.handle));

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
                    for (const auto &blob : qAsConst(m_embedImageBlobs)) {
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
                    new_profile->setAccount(m_account);
                    new_profile->profile(avatar, banner, description, display_name, old_cid);
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
    old_profile->setAccount(m_account);
    old_profile->profile(m_account.did);
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
                    for (const auto &blob : qAsConst(m_embedImageBlobs)) {
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
                    new_list->setAccount(m_account);
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
    old_list->setAccount(m_account);
    old_list->list(m_account.did, r_key);
}

void RecordOperator::updateThreadGate(const QString &uri, const QString &threadgate_uri,
                                      const QString &type, const QStringList &rules)
{
    if (running() || !uri.startsWith("at://"))
        return;
    setRunning(true);

    QString r_key = threadgate_uri.split("/").last();

    setProgressMessage(tr("Update who can reply ..."));

    ComAtprotoRepoDeleteRecordEx *delete_record = new ComAtprotoRepoDeleteRecordEx(this);
    connect(delete_record, &ComAtprotoRepoDeleteRecordEx::finished, [=](bool success) {
        if (!success) {
            emit errorOccured(delete_record->errorCode(), delete_record->errorMessage());
            setProgressMessage(QString());
            setRunning(false);
            emit finished(success, QString(), QString());
        } else if (type == "everybody") {
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
    delete_record->setAccount(m_account);
    delete_record->deleteThreadGate(r_key);
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

void RecordOperator::makeFacets(const QString &text, std::function<void()> callback)
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

            int trimmed_offset = 0;
            QString trimmed_temp = temp.trimmed();
            int temp_pos = temp.indexOf(trimmed_temp);
            int temp_diff_len = temp.length() - trimmed_temp.length();
            if (temp_diff_len > 0) {
                // 前後の空白を消す
                // 今のところhashtagだけここにくる可能性がある
                byte_start = text.left(pos + temp_pos).toUtf8().length();
                byte_end = byte_start + trimmed_temp.toUtf8().length();
                temp = trimmed_temp;
                if (temp_diff_len == 2 || (temp_diff_len == 1 && temp_pos == 0)) {
                    trimmed_offset = 1;
                }
            }
            if (temp.startsWith("@")) {
                temp.remove("@");
                MentionData position;
                position.start = byte_start;
                position.end = byte_end;
                mention.insert(temp, position);
            } else if (temp.startsWith("#")) {
                AppBskyRichtextFacet::Main facet;
                facet.index.byteStart = byte_start;
                facet.index.byteEnd = byte_end;
                AppBskyRichtextFacet::Tag tag;
                tag.tag = temp.mid(1);
                facet.features_type = AppBskyRichtextFacet::MainFeaturesType::features_Tag;
                facet.features_Tag.append(tag);
                m_facets.append(facet);
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

            match = m_rxFacet.match(text, pos + match.capturedLength() - trimmed_offset);
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

            AppBskyActorGetProfiles *profiles = new AppBskyActorGetProfiles(this);
            connect(profiles, &AppBskyActorGetProfiles::finished, [=](bool success) {
                if (success) {
                    for (const auto &item : qAsConst(profiles->profileViewDetailedList())) {
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
    upload_blob->setAccount(m_account);
    upload_blob->uploadBlob(path);
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
            if (list->recordList().isEmpty())
                m_listItemCursor.clear();
            for (const auto &item : list->recordList()) {
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
    list->setAccount(m_account);
    list->listListItems(m_account.did, cursor);
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
    delete_record->setAccount(m_account);
    delete_record->deleteListItem(r_key);
}

bool RecordOperator::threadGate(
        const QString &uri, std::function<void(bool, const QString &, const QString &)> callback)
{
    AtProtocolType::ThreadGateType type = AtProtocolType::ThreadGateType::Everybody;
    QList<AtProtocolType::ThreadGateAllow> rules;
    AtProtocolType::ThreadGateAllow rule;

    if (m_threadGateType == "everybody") {
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
    create_record->setAccount(m_account);
    create_record->threadGate(uri, type, rules);
    return true;
}

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
