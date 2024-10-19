#include "userpost.h"

#include "atprotocol/app/bsky/feed/appbskyfeedgetposts.h"
#include "atprotocol/app/bsky/actor/appbskyactorgetprofile.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "tools/accountmanager.h"
#include "tools/labelerprovider.h"

using AtProtocolInterface::AppBskyActorGetProfile;
using AtProtocolInterface::AppBskyFeedGetPosts;
using namespace AtProtocolType;

UserPost::UserPost(QObject *parent) : QObject { parent }, m_running(false), m_authorMuted(false) { }

void UserPost::setAccount(const QString &uuid)
{
    qDebug().noquote() << this << uuid;
    m_account.uuid = uuid;
}

void UserPost::getPost(const QString &uri)
{
    if (running() || uri.isEmpty())
        return;
    setRunning(true);

    clear();

    convertToAtUri("at://%1/app.bsky.feed.post/%2", uri, [=](const QString &at_uri) {
        if (at_uri.isEmpty()) {
            setRunning(false);
            return;
        }

        AppBskyFeedGetPosts *posts = new AppBskyFeedGetPosts(this);
        connect(posts, &AppBskyFeedGetPosts::finished, this, [=](bool success) {
            if (success) {
                if (!posts->postsList().isEmpty()) {
                    const auto &post = posts->postsList().first();

                    setUri(post.uri);
                    setAuthorDid(post.author.did);
                    setAuthorDisplayName(post.author.displayName);
                    setAuthorHandle(post.author.handle);
                    setAuthorAvatar(post.author.avatar);
                    setAuthorMuted(post.author.viewer.muted);
                    setRecordText(LexiconsTypeUnknown::copyRecordText(post.record));
                    setRecordTextPlain(
                            LexiconsTypeUnknown::fromQVariant<AppBskyFeedPost::Main>(post.record)
                                    .text);
                    setIndexedAt(LexiconsTypeUnknown::formatDateTime(post.indexedAt));
                    setEmbedImages(LexiconsTypeUnknown::copyImagesFromPostView(
                            post, LexiconsTypeUnknown::CopyImageType::Thumb));
                    setEmbedImagesFull(LexiconsTypeUnknown::copyImagesFromPostView(
                            post, LexiconsTypeUnknown::CopyImageType::FullSize));
                    setEmbedImagesAlt(LexiconsTypeUnknown::copyImagesFromPostView(
                            post, LexiconsTypeUnknown::CopyImageType::Alt));

                    setCid(post.cid);
                }
            } else {
                emit errorOccured(posts->errorCode(), posts->errorMessage());
            }
            setRunning(false);
            posts->deleteLater();
        });
        posts->setAccount(AccountManager::getInstance()->getAccount(m_account.uuid));
        posts->setLabelers(labelerDids());
        posts->getPosts(QStringList() << at_uri);
    });
}

void UserPost::clear()
{
    setCid(QString());
    setUri(QString());
    setAuthorDid(QString());
    setAuthorDisplayName(QString());
    setAuthorHandle(QString());
    setAuthorAvatar(QString());
    setAuthorMuted(false);
    setRecordText(QString());
    setRecordTextPlain(QString());
    setIndexedAt(QString());
    setEmbedImages(QStringList());
    setEmbedImagesFull(QStringList());
    setEmbedImagesAlt(QStringList());
}

bool UserPost::running() const
{
    return m_running;
}

void UserPost::setRunning(bool newRunning)
{
    if (m_running == newRunning)
        return;
    m_running = newRunning;
    emit runningChanged();
}

QString UserPost::cid() const
{
    return m_cid;
}

void UserPost::setCid(const QString &newCid)
{
    if (m_cid == newCid)
        return;
    m_cid = newCid;
    emit cidChanged();
}

QString UserPost::uri() const
{
    return m_uri;
}

void UserPost::setUri(const QString &newUri)
{
    if (m_uri == newUri)
        return;
    m_uri = newUri;
    emit uriChanged();
}

QString UserPost::authorDid() const
{
    return m_authorDid;
}

void UserPost::setAuthorDid(const QString &newAuthorDid)
{
    if (m_authorDid == newAuthorDid)
        return;
    m_authorDid = newAuthorDid;
    emit authorDidChanged();
}

QString UserPost::authorDisplayName() const
{
    return m_authorDisplayName;
}

void UserPost::setAuthorDisplayName(const QString &newAuthorDisplayName)
{
    if (m_authorDisplayName == newAuthorDisplayName)
        return;
    m_authorDisplayName = newAuthorDisplayName;
    emit authorDisplayNameChanged();
}

QString UserPost::authorHandle() const
{
    return m_authorHandle;
}

void UserPost::setAuthorHandle(const QString &newAuthorHandle)
{
    if (m_authorHandle == newAuthorHandle)
        return;
    m_authorHandle = newAuthorHandle;
    emit authorHandleChanged();
}

QString UserPost::authorAvatar() const
{
    return m_authorAvatar;
}

void UserPost::setAuthorAvatar(const QString &newAuthorAvatar)
{
    if (m_authorAvatar == newAuthorAvatar)
        return;
    m_authorAvatar = newAuthorAvatar;
    emit authorAvatarChanged();
}

bool UserPost::authorMuted() const
{
    return m_authorMuted;
}

void UserPost::setAuthorMuted(const bool newAuthorMuted)
{
    if (m_authorMuted == newAuthorMuted)
        return;
    m_authorMuted = newAuthorMuted;
    emit authorMutedChanged();
}

QString UserPost::recordText() const
{
    return m_recordText;
}

void UserPost::setRecordText(const QString &newRecordText)
{
    if (m_recordText == newRecordText)
        return;
    m_recordText = newRecordText;
    emit recordTextChanged();
}

QString UserPost::recordTextPlain() const
{
    return m_recordTextPlain;
}

void UserPost::setRecordTextPlain(const QString &newRecordTextPlain)
{
    if (m_recordTextPlain == newRecordTextPlain)
        return;
    m_recordTextPlain = newRecordTextPlain;
    emit recordTextPlainChanged();
}

QString UserPost::indexedAt() const
{
    return m_indexedAt;
}

void UserPost::setIndexedAt(const QString &newIndexedAt)
{
    if (m_indexedAt == newIndexedAt)
        return;
    m_indexedAt = newIndexedAt;
    emit indexedAtChanged();
}

QStringList UserPost::embedImages() const
{
    return m_embedImages;
}

void UserPost::setEmbedImages(const QStringList &newEmbedImages)
{
    if (m_embedImages == newEmbedImages)
        return;
    m_embedImages = newEmbedImages;
    emit embedImagesChanged();
}

QStringList UserPost::embedImagesFull() const
{
    return m_embedImagesFull;
}

void UserPost::setEmbedImagesFull(const QStringList &newEmbedImagesFull)
{
    if (m_embedImagesFull == newEmbedImagesFull)
        return;
    m_embedImagesFull = newEmbedImagesFull;
    emit embedImagesFullChanged();
}

QStringList UserPost::embedImagesAlt() const
{
    return m_embedImagesAlt;
}

void UserPost::setEmbedImagesAlt(const QStringList &newEmbedImagesAlt)
{
    if (m_embedImagesAlt == newEmbedImagesAlt)
        return;
    m_embedImagesAlt = newEmbedImagesAlt;
    emit embedImagesAltChanged();
}

void UserPost::convertToAtUri(const QString &base_at_uri, const QString &uri,
                              std::function<void(const QString &)> callback)
{
    if (uri.startsWith("at://")) {
        callback(uri);
        return;
    }
    QStringList items = uri.split("/");
    if (items.length() != 7) {
        callback(QString());
        return;
    }
    QString user_id = items.at(4);
    if (user_id.startsWith("did:")) {
        // did
        callback(QString(base_at_uri).arg(user_id, items.at(6)));
    } else {
        // handle
        AppBskyActorGetProfile *profile = new AppBskyActorGetProfile(this);
        connect(profile, &AppBskyActorGetProfile::finished, [=](bool success) {
            if (success) {
                // handle -> did
                callback(QString(base_at_uri).arg(profile->profileViewDetailed().did, items.at(6)));
            } else {
                callback(QString());
            }
            profile->deleteLater();
        });
        profile->setAccount(AccountManager::getInstance()->getAccount(m_account.uuid));
        profile->getProfile(user_id);
    }
}

QStringList UserPost::labelerDids() const
{
    return LabelerProvider::getInstance()->labelerDids(m_account);
}
