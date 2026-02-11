#include "draftlistmodel.h"

#include "atprotocol/app/bsky/draft/appbskydraftgetdrafts.h"

using AtProtocolInterface::AppBskyDraftGetDrafts;
using namespace AtProtocolType;

namespace {
using DraftView = AppBskyDraftDefs::DraftView;
using Draft = AppBskyDraftDefs::Draft;
using DraftPost = AppBskyDraftDefs::DraftPost;
using DraftEmbedImage = AppBskyDraftDefs::DraftEmbedImage;
using DraftEmbedVideo = AppBskyDraftDefs::DraftEmbedVideo;
using DraftEmbedCaption = AppBskyDraftDefs::DraftEmbedCaption;
using DraftEmbedExternal = AppBskyDraftDefs::DraftEmbedExternal;
using DraftEmbedRecord = AppBskyDraftDefs::DraftEmbedRecord;

constexpr auto kSelfLabelsType = "com.atproto.label.defs#selfLabels";
constexpr auto kDisableRuleType = "app.bsky.feed.postgate#disableRule";
constexpr auto kMentionRuleType = "app.bsky.feed.threadgate#mentionRule";
constexpr auto kFollowerRuleType = "app.bsky.feed.threadgate#followerRule";
constexpr auto kFollowingRuleType = "app.bsky.feed.threadgate#followingRule";
constexpr auto kListRuleType = "app.bsky.feed.threadgate#listRule";

QVariantMap labelsToVariant(const ComAtprotoLabelDefs::SelfLabels &labels)
{
    QVariantMap map;
    if (labels.values.isEmpty())
        return map;

    QVariantList values;
    for (const auto &value : labels.values) {
        QVariantMap entry;
        entry["val"] = value.val;
        values.append(entry);
    }
    map["$type"] = QString::fromLatin1(kSelfLabelsType);
    map["values"] = values;
    return map;
}

QVariantMap imageToVariant(const DraftEmbedImage &image)
{
    QVariantMap map;
    map["path"] = image.localRef.path;
    if (!image.alt.isEmpty()) {
        map["alt"] = image.alt;
    }
    return map;
}

QVariantMap captionToVariant(const DraftEmbedCaption &caption)
{
    QVariantMap map;
    map["lang"] = caption.lang;
    map["content"] = caption.content;
    return map;
}

QVariantMap videoToVariant(const DraftEmbedVideo &video)
{
    QVariantMap map;
    map["path"] = video.localRef.path;
    if (!video.alt.isEmpty()) {
        map["alt"] = video.alt;
    }
    if (!video.captions.isEmpty()) {
        QVariantList captions;
        for (const auto &caption : video.captions) {
            captions.append(captionToVariant(caption));
        }
        map["captions"] = captions;
    }
    return map;
}

QVariantMap externalToVariant(const DraftEmbedExternal &external)
{
    QVariantMap map;
    map["uri"] = external.uri;
    return map;
}

QVariantMap recordToVariant(const DraftEmbedRecord &record)
{
    QVariantMap map;
    map["uri"] = record.record.uri;
    map["cid"] = record.record.cid;
    return map;
}

QVariantList imagesToVariant(const QList<DraftEmbedImage> &images)
{
    QVariantList list;
    for (const auto &image : images) {
        list.append(imageToVariant(image));
    }
    return list;
}

QVariantList videosToVariant(const QList<DraftEmbedVideo> &videos)
{
    QVariantList list;
    for (const auto &video : videos) {
        list.append(videoToVariant(video));
    }
    return list;
}

QVariantList externalsToVariant(const QList<DraftEmbedExternal> &externals)
{
    QVariantList list;
    for (const auto &external : externals) {
        list.append(externalToVariant(external));
    }
    return list;
}

QVariantList recordsToVariant(const QList<DraftEmbedRecord> &records)
{
    QVariantList list;
    for (const auto &record : records) {
        list.append(recordToVariant(record));
    }
    return list;
}

QVariantMap draftPostToVariant(const DraftPost &post)
{
    QVariantMap map;
    map["text"] = post.text;

    if (post.labels_type
        == AppBskyDraftDefs::DraftPostLabelsType::labels_ComAtprotoLabelDefs_SelfLabels) {
        auto labels = labelsToVariant(post.labels_ComAtprotoLabelDefs_SelfLabels);
        if (!labels.isEmpty()) {
            map["labels"] = labels;
        }
    }

    if (!post.embedImages.isEmpty()) {
        map["embedImages"] = imagesToVariant(post.embedImages);
    }
    if (!post.embedVideos.isEmpty()) {
        map["embedVideos"] = videosToVariant(post.embedVideos);
    }
    if (!post.embedExternals.isEmpty()) {
        map["embedExternals"] = externalsToVariant(post.embedExternals);
    }
    if (!post.embedRecords.isEmpty()) {
        map["embedRecords"] = recordsToVariant(post.embedRecords);
    }

    return map;
}

QVariantList postsToVariant(const Draft &draft)
{
    QVariantList list;
    for (const auto &post : draft.posts) {
        list.append(draftPostToVariant(post));
    }
    return list;
}

QVariantList postgateRulesToVariant(const Draft &draft)
{
    QVariantList rules;
    if (draft.postgateEmbeddingRules_type
        == AppBskyDraftDefs::DraftPostgateEmbeddingRulesType::
                postgateEmbeddingRules_AppBskyFeedPostgate_DisableRule) {
        for (int i = 0; i < draft.postgateEmbeddingRules_AppBskyFeedPostgate_DisableRule.count();
             ++i) {
            QVariantMap rule;
            rule["$type"] = QString::fromLatin1(kDisableRuleType);
            rules.append(rule);
        }
    }
    return rules;
}

QVariantList threadgateAllowToVariant(const Draft &draft)
{
    QVariantList allows;
    switch (draft.threadgateAllow_type) {
    case AppBskyDraftDefs::DraftThreadgateAllowType::
            threadgateAllow_AppBskyFeedThreadgate_MentionRule:
        for (int i = 0; i < draft.threadgateAllow_AppBskyFeedThreadgate_MentionRule.count(); ++i) {
            QVariantMap allow;
            allow["$type"] = QString::fromLatin1(kMentionRuleType);
            allows.append(allow);
        }
        break;
    case AppBskyDraftDefs::DraftThreadgateAllowType::
            threadgateAllow_AppBskyFeedThreadgate_FollowerRule:
        for (int i = 0; i < draft.threadgateAllow_AppBskyFeedThreadgate_FollowerRule.count(); ++i) {
            QVariantMap allow;
            allow["$type"] = QString::fromLatin1(kFollowerRuleType);
            allows.append(allow);
        }
        break;
    case AppBskyDraftDefs::DraftThreadgateAllowType::
            threadgateAllow_AppBskyFeedThreadgate_FollowingRule:
        for (int i = 0; i < draft.threadgateAllow_AppBskyFeedThreadgate_FollowingRule.count();
             ++i) {
            QVariantMap allow;
            allow["$type"] = QString::fromLatin1(kFollowingRuleType);
            allows.append(allow);
        }
        break;
    case AppBskyDraftDefs::DraftThreadgateAllowType::threadgateAllow_AppBskyFeedThreadgate_ListRule:
        for (const auto &rule : draft.threadgateAllow_AppBskyFeedThreadgate_ListRule) {
            QVariantMap allow;
            allow["$type"] = QString::fromLatin1(kListRuleType);
            allow["list"] = rule.list;
            allows.append(allow);
        }
        break;
    default:
        break;
    }
    return allows;
}

QVariantMap draftToVariant(const Draft &draft)
{
    QVariantMap map;
    map["posts"] = postsToVariant(draft);
    if (!draft.langs.isEmpty()) {
        map["langs"] = QVariant::fromValue(draft.langs);
    }
    auto postgateRules = postgateRulesToVariant(draft);
    if (!postgateRules.isEmpty()) {
        map["postgateEmbeddingRules"] = postgateRules;
    }
    auto threadgateAllow = threadgateAllowToVariant(draft);
    if (!threadgateAllow.isEmpty()) {
        map["threadgateAllow"] = threadgateAllow;
    }
    return map;
}

QVariantMap draftViewToVariant(const DraftView &view)
{
    QVariantMap map;
    map["id"] = view.id;
    map["createdAt"] = view.createdAt;
    map["updatedAt"] = view.updatedAt;
    map["postCount"] = view.draft.posts.count();
    map["draft"] = draftToVariant(view.draft);
    return map;
}
}

DraftListModel::DraftListModel(QObject *parent)
    : AtpAbstractListModel { parent }, m_pageSize(25) { }

int DraftListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_drafts.count();
}

QVariant DraftListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    return item(index.row(), static_cast<DraftListModelRoles>(role));
}

QVariant DraftListModel::item(int row, DraftListModelRoles role) const
{
    if (row < 0 || row >= m_drafts.count())
        return QVariant();

    const auto &view = m_drafts.at(row);
    switch (role) {
    case DraftListModelRoles::ModelData:
    case DraftListModelRoles::DraftRole:
        return draftViewToVariant(view);
    case DraftListModelRoles::IdRole:
        return view.id;
    case DraftListModelRoles::CreatedAtRole:
        return view.createdAt;
    case DraftListModelRoles::UpdatedAtRole:
        return view.updatedAt;
    case DraftListModelRoles::PrimaryTextRole:
        if (!view.draft.posts.isEmpty()) {
            return view.draft.posts.constFirst().text;
        }
        return QString();
    case DraftListModelRoles::PostsRole:
        return postsToVariant(view.draft);
    case DraftListModelRoles::LangsRole:
        return QVariant::fromValue(view.draft.langs);
    case DraftListModelRoles::PostgateEmbeddingRulesRole:
        return postgateRulesToVariant(view.draft);
    case DraftListModelRoles::ThreadgateAllowRole:
        return threadgateAllowToVariant(view.draft);
    default:
        break;
    }

    return QVariant();
}

bool DraftListModel::getLatest()
{
    if (running())
        return false;

    if (account().uuid.isEmpty())
        return false;

    setRunning(true);
    requestDrafts(QString(), false);
    return true;
}

bool DraftListModel::getNext()
{
    if (running())
        return false;

    if (cursor().isEmpty())
        return false;

    setRunning(true);
    requestDrafts(cursor(), true);
    return true;
}

void DraftListModel::clear()
{
    bool hadMore = hasMore();
    AtpAbstractListModel::clear();
    if (!m_drafts.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_drafts.count() - 1);
        m_drafts.clear();
        endRemoveRows();
    }
    if (hadMore != hasMore()) {
        emit hasMoreChanged();
    }
}

int DraftListModel::pageSize() const
{
    return m_pageSize;
}

void DraftListModel::setPageSize(int newPageSize)
{
    if (newPageSize <= 0 || newPageSize == m_pageSize)
        return;

    m_pageSize = newPageSize;
    emit pageSizeChanged();
}

bool DraftListModel::hasMore() const
{
    return !cursor().isEmpty();
}

int DraftListModel::indexOf(const QString &cid) const
{
    for (int i = 0; i < m_drafts.count(); ++i) {
        if (m_drafts.at(i).id == cid)
            return i;
    }
    return -1;
}

QString DraftListModel::getRecordText(const QString &cid)
{
    int row = indexOf(cid);
    if (row < 0)
        return QString();

    if (m_drafts.at(row).draft.posts.isEmpty())
        return QString();

    return m_drafts.at(row).draft.posts.constFirst().text;
}

QString DraftListModel::getOfficialUrl() const
{
    return QString();
}

QString DraftListModel::getItemOfficialUrl(int row) const
{
    Q_UNUSED(row)
    return QString();
}

QHash<int, QByteArray> DraftListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[DraftListModelRoles::ModelData] = "modelData";
    roles[DraftListModelRoles::IdRole] = "id";
    roles[DraftListModelRoles::CreatedAtRole] = "createdAt";
    roles[DraftListModelRoles::UpdatedAtRole] = "updatedAt";
    roles[DraftListModelRoles::PrimaryTextRole] = "primaryText";
    roles[DraftListModelRoles::PostsRole] = "posts";
    roles[DraftListModelRoles::LangsRole] = "langs";
    roles[DraftListModelRoles::PostgateEmbeddingRulesRole] = "postgateEmbeddingRules";
    roles[DraftListModelRoles::ThreadgateAllowRole] = "threadgateAllow";
    roles[DraftListModelRoles::DraftRole] = "draft";
    return roles;
}

bool DraftListModel::aggregateQueuedPosts(const QString &cid, const bool next)
{
    Q_UNUSED(cid)
    Q_UNUSED(next)
    return true;
}

bool DraftListModel::aggregated(const QString &cid) const
{
    Q_UNUSED(cid)
    return false;
}

void DraftListModel::finishedDisplayingQueuedPosts() { }

bool DraftListModel::checkVisibility(const QString &cid)
{
    Q_UNUSED(cid)
    return true;
}

void DraftListModel::requestDrafts(const QString &cursorValue, bool append)
{
    AppBskyDraftGetDrafts *get = new AppBskyDraftGetDrafts(this);
    connect(get, &AppBskyDraftGetDrafts::finished, this, [=](bool success) {
        if (success) {
            const auto drafts = get->draftsList();
            if (!append && !m_drafts.isEmpty()) {
                beginRemoveRows(QModelIndex(), 0, m_drafts.count() - 1);
                m_drafts.clear();
                endRemoveRows();
            }
            if (!drafts.isEmpty()) {
                int insertPos = m_drafts.count();
                beginInsertRows(QModelIndex(), insertPos, insertPos + drafts.count() - 1);
                for (const auto &draft : drafts) {
                    m_drafts.append(draft);
                }
                endInsertRows();
            }

            bool hadMore = hasMore();
            setCursor(get->cursor());
            if (hadMore != hasMore()) {
                emit hasMoreChanged();
            }
        } else {
            emit errorOccured(get->errorCode(), get->errorMessage());
        }
        setRunning(false);
        get->deleteLater();
    });
    get->setAccount(account());
    get->getDrafts(m_pageSize, cursorValue);
}
