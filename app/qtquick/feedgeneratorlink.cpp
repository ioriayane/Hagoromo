#include "feedgeneratorlink.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetfeedgenerator.h"

using AtProtocolInterface::AppBskyFeedGetFeedGenerator;

FeedGeneratorLink::FeedGeneratorLink(QObject *parent)
    : QObject { parent }, m_running(false), m_valid(false), m_likeCount(0)
{
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

bool FeedGeneratorLink::checkUri(const QString &uri) const
{
    // https://bsky.app/profile/did:plc:hoge/feed/aaaaaaaaaa
    if (uri.isEmpty())
        return false;
    if (!uri.startsWith("https://bsky.app/profile/"))
        return false;
    QStringList items = uri.split("/");
    if (items.length() != 7)
        return false;
    if (!items.at(4).startsWith("did:plc:"))
        return false;
    if (items.at(5) != "feed")
        return false;
    if (items.at(6).isEmpty())
        return false;

    return true;
}

QString FeedGeneratorLink::convertToAtUri(const QString &uri)
{
    if (!checkUri(uri))
        return QString();

    QStringList items = uri.split("/");

    return QString("at://%1/app.bsky.feed.generator/%2").arg(items.at(4), items.at(6));
}

void FeedGeneratorLink::getFeedGenerator(const QString &uri)
{
    if (running())
        return;
    setRunning(true);

    clear();

    AppBskyFeedGetFeedGenerator *generator = new AppBskyFeedGetFeedGenerator(this);
    connect(generator, &AppBskyFeedGetFeedGenerator::finished, [=](bool success) {
        if (success) {
            setAvatar(generator->generatorView().avatar);
            setDisplayName(generator->generatorView().displayName);
            setCreatorHandle(generator->generatorView().creator.handle);
            setLikeCount(generator->generatorView().likeCount);
            setValid(true);
        }
        setRunning(false);
        generator->deleteLater();
    });
    generator->setAccount(m_account);
    generator->getFeedGenerator(uri);
}

void FeedGeneratorLink::clear()
{
    setValid(false);
    setAvatar(QString());
    setDisplayName(QString());
    setCreatorHandle(QString());
    setLikeCount(0);
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
