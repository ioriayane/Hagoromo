#include "anyprofilelistmodel.h"

#include "atprotocol/app/bsky/feed/appbskyfeedgetlikes.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetrepostedby.h"

using AtProtocolInterface::AppBskyFeedGetLikes;
using AtProtocolInterface::AppBskyFeedGetRepostedBy;

AnyProfileListModel::AnyProfileListModel(QObject *parent) : FollowsListModel { parent } { }

bool AnyProfileListModel::getLatest()
{
    if (running() || targetUri().isEmpty())
        return false;
    setRunning(true);

    clear();

    if (type() == AnyProfileListModelType::Like) {
        AppBskyFeedGetLikes *likes = new AppBskyFeedGetLikes(this);
        connect(likes, &AppBskyFeedGetLikes::finished, [=](bool success) {
            if (success) {
                m_cursor = likes->cursor();
                for (const auto &like : likes->likesLikeList()) {
                    m_profileHash[like.actor.did] = like.actor;
                    m_formattedDescriptionHash[like.actor.did] =
                            m_systemTool.markupText(like.actor.description);
                    if (m_didList.contains(like.actor.did)) {
                        int row = m_didList.indexOf(like.actor.did);
                        emit dataChanged(index(row), index(row));
                    } else {
                        beginInsertRows(QModelIndex(), m_didList.count(), m_didList.count());
                        m_didList.append(like.actor.did);
                        endInsertRows();
                    }
                }
            } else {
                emit errorOccured(likes->errorCode(), likes->errorMessage());
            }
            setRunning(false);
            likes->deleteLater();
        });
        likes->setAccount(account());
        likes->getLikes(targetUri(), QString(), 0, QString());

    } else if (type() == AnyProfileListModelType::Repost) {
        AppBskyFeedGetRepostedBy *reposts = new AppBskyFeedGetRepostedBy(this);
        connect(reposts, &AppBskyFeedGetRepostedBy::finished, [=](bool success) {
            if (success) {
                m_cursor = reposts->cursor();
                for (const auto &profile : reposts->followsProfileViewList()) {
                    m_profileHash[profile.did] = profile;
                    m_formattedDescriptionHash[profile.did] =
                            m_systemTool.markupText(profile.description);
                    if (m_didList.contains(profile.did)) {
                        int row = m_didList.indexOf(profile.did);
                        emit dataChanged(index(row), index(row));
                    } else {
                        beginInsertRows(QModelIndex(), m_didList.count(), m_didList.count());
                        m_didList.append(profile.did);
                        endInsertRows();
                    }
                }
            } else {
                emit errorOccured(reposts->errorCode(), reposts->errorMessage());
            }
            setRunning(false);
            reposts->deleteLater();
        });
        reposts->setAccount(account());
        reposts->getRepostedBy(targetUri(), QString(), 0, QString());
    }
    return true;
}

bool AnyProfileListModel::getNext()
{
    if (running() || targetUri().isEmpty() || m_cursor.isEmpty())
        return false;
    setRunning(true);

    if (type() == AnyProfileListModelType::Like) {
        AppBskyFeedGetLikes *likes = new AppBskyFeedGetLikes(this);
        connect(likes, &AppBskyFeedGetLikes::finished, [=](bool success) {
            if (success) {
                if (likes->likesLikeList().isEmpty())
                    m_cursor.clear();
                else
                    m_cursor = likes->cursor();
                for (const auto &like : likes->likesLikeList()) {
                    m_profileHash[like.actor.did] = like.actor;
                    m_formattedDescriptionHash[like.actor.did] =
                            m_systemTool.markupText(like.actor.description);
                    if (m_didList.contains(like.actor.did)) {
                        int row = m_didList.indexOf(like.actor.did);
                        emit dataChanged(index(row), index(row));
                    } else {
                        beginInsertRows(QModelIndex(), m_didList.count(), m_didList.count());
                        m_didList.append(like.actor.did);
                        endInsertRows();
                    }
                }
            } else {
                emit errorOccured(likes->errorCode(), likes->errorMessage());
            }
            setRunning(false);
            likes->deleteLater();
        });
        likes->setAccount(account());
        likes->getLikes(targetUri(), QString(), 0, m_cursor);

    } else if (type() == AnyProfileListModelType::Repost) {
        AppBskyFeedGetRepostedBy *reposts = new AppBskyFeedGetRepostedBy(this);
        connect(reposts, &AppBskyFeedGetRepostedBy::finished, [=](bool success) {
            if (success) {
                if (reposts->followsProfileViewList().isEmpty())
                    m_cursor.clear();
                else
                    m_cursor = reposts->cursor();
                for (const auto &profile : reposts->followsProfileViewList()) {
                    m_profileHash[profile.did] = profile;
                    m_formattedDescriptionHash[profile.did] =
                            m_systemTool.markupText(profile.description);
                    if (m_didList.contains(profile.did)) {
                        int row = m_didList.indexOf(profile.did);
                        emit dataChanged(index(row), index(row));
                    } else {
                        beginInsertRows(QModelIndex(), m_didList.count(), m_didList.count());
                        m_didList.append(profile.did);
                        endInsertRows();
                    }
                }
            } else {
                emit errorOccured(reposts->errorCode(), reposts->errorMessage());
            }
            setRunning(false);
            reposts->deleteLater();
        });
        reposts->setAccount(account());
        reposts->getRepostedBy(targetUri(), QString(), 0, m_cursor);
    }
    return true;
}

QString AnyProfileListModel::targetUri() const
{
    return m_targetUri;
}

void AnyProfileListModel::setTargetUri(const QString &newTargetUri)
{
    if (m_targetUri == newTargetUri)
        return;
    m_targetUri = newTargetUri;
    emit targetUriChanged();
}

AnyProfileListModel::AnyProfileListModelType AnyProfileListModel::type() const
{
    return m_type;
}

void AnyProfileListModel::setType(const AnyProfileListModelType &newType)
{
    if (m_type == newType)
        return;
    m_type = newType;
    clear();
    emit typeChanged();
}
