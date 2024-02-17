#include "feedgeneratorlink.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetfeedgenerator.h"
#include "systemtool.h"

using AtProtocolInterface::AppBskyFeedGetFeedGenerator;

FeedGeneratorLink::FeedGeneratorLink(QObject *parent)
    : QObject { parent }, m_running(false), m_valid(false), m_likeCount(0)
{
    m_rxHandle.setPattern(QString("^%1$").arg(REG_EXP_HANDLE));
}

void FeedGeneratorLink::setAccount(const QString &service, const QString &did,
                                   const QString &handle, const QString &email,
                                   const QString &accessJwt, const QString &refreshJwt)
{
    m_account.service = service;
    m_account.did = did;
    m_account.handle = handle;
    m_account.email = email;
    m_account.accessJwt = accessJwt;
    m_account.refreshJwt = refreshJwt;
}

bool FeedGeneratorLink::checkUri(const QString &uri, const QString &type) const
{
    // https://bsky.app/profile/did:plc:hoge/feed/aaaaaaaaaa
    if (uri.isEmpty())
        return false;
    if (!uri.startsWith("https://bsky.app/profile/"))
        return false;
    QStringList items = uri.split("/");
    if (items.length() != 7)
        return false;
    if (!(items.at(4).startsWith("did:plc:") || m_rxHandle.match(items.at(4)).hasMatch()))
        return false;
    if (items.at(5) != type)
        return false;
    if (items.at(6).isEmpty())
        return false;

    return true;
}

void FeedGeneratorLink::convertToAtUri(const QString &base_at_uri, const QString &uri,
                                       std::function<void(const QString &)> callback)
{
    QStringList items = uri.split("/");
    if (items.length() != 7) {
        callback(QString());
        return;
    }
    QString user_id = items.at(4);
    if (m_rxHandle.match(user_id).hasMatch()) {
        // handle
        callback(QString());
    } else {
        // did
        callback(QString(base_at_uri).arg(user_id, items.at(6)));
    }
}

void FeedGeneratorLink::getFeedGenerator(const QString &uri)
{
    if (running())
        return;
    setRunning(true);

    clear();

    convertToAtUri("at://%1/app.bsky.feed.generator/%2", uri, [=](const QString &at_uri) {
        if (at_uri.isEmpty()) {
            setRunning(false);
            return;
        }
        AppBskyFeedGetFeedGenerator *generator = new AppBskyFeedGetFeedGenerator(this);
        connect(generator, &AppBskyFeedGetFeedGenerator::finished, [=](bool success) {
            if (success) {
                setAvatar(generator->generatorView().avatar);
                setDisplayName(generator->generatorView().displayName);
                setCreatorHandle(generator->generatorView().creator.handle);
                setLikeCount(generator->generatorView().likeCount);
                setUri(generator->generatorView().uri);
                setCid(generator->generatorView().cid);
                setValid(true);
            }
            setRunning(false);
            generator->deleteLater();
        });
        generator->setAccount(m_account);
        generator->getFeedGenerator(at_uri);
    });
}

void FeedGeneratorLink::clear()
{
    setValid(false);
    setAvatar(QString());
    setDisplayName(QString());
    setCreatorHandle(QString());
    setLikeCount(0);
    setUri(QString());
    setCid(QString());
}

bool FeedGeneratorLink::running() const
{
    return m_running;
}

void FeedGeneratorLink::setRunning(bool newRunning)
{
    if (m_running == newRunning)
        return;
    m_running = newRunning;
    emit runningChanged();
}

bool FeedGeneratorLink::valid() const
{
    return m_valid;
}

void FeedGeneratorLink::setValid(bool newValid)
{
    if (m_valid == newValid)
        return;
    m_valid = newValid;
    emit validChanged();
}

QString FeedGeneratorLink::avatar() const
{
    return m_avatar;
}

void FeedGeneratorLink::setAvatar(const QString &newAvatar)
{
    if (m_avatar == newAvatar)
        return;
    m_avatar = newAvatar;
    emit avatarChanged();
}

QString FeedGeneratorLink::displayName() const
{
    return m_displayName;
}

void FeedGeneratorLink::setDisplayName(const QString &newDisplayName)
{
    if (m_displayName == newDisplayName)
        return;
    m_displayName = newDisplayName;
    emit displayNameChanged();
}

QString FeedGeneratorLink::creatorHandle() const
{
    return m_creatorHandle;
}

void FeedGeneratorLink::setCreatorHandle(const QString &newCreatorHandle)
{
    if (m_creatorHandle == newCreatorHandle)
        return;
    m_creatorHandle = newCreatorHandle;
    emit creatorHandleChanged();
}

int FeedGeneratorLink::likeCount() const
{
    return m_likeCount;
}

void FeedGeneratorLink::setLikeCount(int newLikeCount)
{
    if (m_likeCount == newLikeCount)
        return;
    m_likeCount = newLikeCount;
    emit likeCountChanged();
}

QString FeedGeneratorLink::uri() const
{
    return m_uri;
}

void FeedGeneratorLink::setUri(const QString &newUri)
{
    if (m_uri == newUri)
        return;
    m_uri = newUri;
    emit uriChanged();
}

QString FeedGeneratorLink::cid() const
{
    return m_cid;
}

void FeedGeneratorLink::setCid(const QString &newCid)
{
    if (m_cid == newCid)
        return;
    m_cid = newCid;
    emit cidChanged();
}
