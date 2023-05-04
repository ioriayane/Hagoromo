#include "timelinelistmodel.h"

#include <QDebug>

TimelineListModel::TimelineListModel(QObject *parent) : QAbstractListModel { parent }
{
    connect(&m_timeline, &AppBskyFeedGetTimeline::finished, [=](bool success) {
        // data copy
        //        QList<AppBskyFeedDefs::FeedViewPost>::const_iterator i =
        //                m_timeline.feedList()->constBegin();
        //        while (i != m_timeline.feedList()->constEnd()) {
        //            i->post
        //            ++i;
        //        }
        beginInsertRows(QModelIndex(), 0, m_timeline.feedList()->count() - 1);
        for (const AppBskyFeedDefs::FeedViewPost &item : *m_timeline.feedList()) {
            m_cidList.append(item.post.cid);
            m_viewPostHash[item.post.cid] = item;
        }
        endInsertRows();
    });
}

int TimelineListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_cidList.count();
}

QVariant TimelineListModel::data(const QModelIndex &index, int role) const
{
    return item(index.row(), static_cast<TimelineListModelRoles>(role));
}

QVariant TimelineListModel::item(int row, TimelineListModelRoles role) const
{
    if (row < 0 || row >= m_cidList.count())
        return QVariant();

    const AppBskyFeedDefs::FeedViewPost &current = m_viewPostHash.value(m_cidList.at(row));

    if (role == DisplayNameRole)
        return current.post.author.displayName;
    else if (role == HandleRole)
        return current.post.author.handle;
    else if (role == AvatarRole)
        return current.post.author.avatar;
    else if (role == RecordTextRole) {
        if (current.post.record.canConvert<AppBskyFeedPost::Record>()) {
            const AppBskyFeedPost::Record record =
                    current.post.record.value<AppBskyFeedPost::Record>();
            return record.text;
        } else {
            return QString();
        }
    } else if (role == ReplyCountRole)
        return current.post.replyCount;
    else if (role == RepostCountRole)
        return current.post.repostCount;
    else if (role == LikeCountRole)
        return current.post.likeCount;
    else if (role == IndexedAtRole)
        return current.post.indexedAt;
    else if (role == EmbedImagesRole) {
        if (current.post.embed_type
            == AppBskyFeedDefs::PostViewEmbedType::embed_AppBskyEmbedImages_View) {
            QString images;
            for (const auto &image : current.post.embed_AppBskyEmbedImages_View.images) {
                if (!images.isEmpty())
                    images.append("\n");
                images.append(image.thumb);
            }
            return images;
        } else {
            return QString();
        }
    } else if (role == HasParentRole)
        return current.reply.parent.cid.length() > 0;
    else if (role == ParentDisPlayName)
        return current.reply.parent.author.displayName;

    return QVariant();
}

void TimelineListModel::update(int row, TimelineListModelRoles role, const QVariant &value)
{
    if (row < 0 || row >= m_cidList.count())
        return;

    // 外から更新しない

    return;
}

void TimelineListModel::updateAccount(const QString &service, const QString &did,
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

void TimelineListModel::getLatest()
{
    m_timeline.setService(m_account.service);
    m_timeline.setSession(m_account.did, m_account.handle, m_account.email, m_account.accessJwt,
                          m_account.refreshJwt);
    m_timeline.getTimeline();
}

QHash<int, QByteArray> TimelineListModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[DisplayNameRole] = "displayName";
    roles[HandleRole] = "handle";
    roles[AvatarRole] = "avatar";
    roles[RecordTextRole] = "recordText";
    roles[ReplyCountRole] = "replyCount";
    roles[RepostCountRole] = "repostCount";
    roles[LikeCountRole] = "likeCount";
    roles[IndexedAtRole] = "indexedAt";
    roles[EmbedImagesRole] = "embedImages";
    roles[HasParentRole] = "hasParent";
    roles[ParentDisPlayName] = "parentDisplayName";

    return roles;
}
