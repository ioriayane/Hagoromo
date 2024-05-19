#include "postlink.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetposts.h"
#include "atprotocol/lexicons_func_unknown.h"

using AtProtocolInterface::AppBskyFeedGetPosts;

PostLink::PostLink(QObject *parent) : FeedGeneratorLink { parent } { }

void PostLink::getPost(const QString &uri)
{
    if (running())
        return;
    setRunning(true);

    clear();

    convertToAtUri("at://%1/app.bsky.feed.post/%2", uri, [=](const QString &at_uri) {
        if (at_uri.isEmpty()) {
            setRunning(false);
            return;
        }
        AppBskyFeedGetPosts *post = new AppBskyFeedGetPosts(this);
        connect(post, &AppBskyFeedGetPosts::finished, [=](bool success) {
            if (success && !post->postsPostViewList().isEmpty()) {
                setAvatar(post->postsPostViewList().at(0).author.avatar);
                setDisplayName(post->postsPostViewList().at(0).author.displayName);
                setCreatorHandle(post->postsPostViewList().at(0).author.handle);
                setLikeCount(0);
                setUri(post->postsPostViewList().at(0).uri);
                setCid(post->postsPostViewList().at(0).cid);
                setIndexedAt(AtProtocolType::LexiconsTypeUnknown::formatDateTime(
                        post->postsPostViewList().at(0).indexedAt));
                setText(AtProtocolType::LexiconsTypeUnknown::copyRecordText(
                        post->postsPostViewList().at(0).record));
                setValid(true);
            }
            setRunning(false);
            post->deleteLater();
        });
        post->setAccount(m_account);
        post->getPosts(QStringList() << at_uri);
    });
}

void PostLink::clear()
{
    FeedGeneratorLink::clear();
    setIndexedAt(QString());
    setText(QString());
}

QString PostLink::indexedAt() const
{
    return m_indexedAt;
}

void PostLink::setIndexedAt(const QString &newIndexedAt)
{
    if (m_indexedAt == newIndexedAt)
        return;
    m_indexedAt = newIndexedAt;
    emit indexedAtChanged();
}

QString PostLink::text() const
{
    return m_text;
}

void PostLink::setText(const QString &newText)
{
    if (m_text == newText)
        return;
    m_text = newText;
    emit textChanged();
}
