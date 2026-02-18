#include "draftlistmodel.h"

#include "atprotocol/app/bsky/draft/appbskydraftgetdrafts.h"
#include "atprotocol/lexicons_func_unknown.h"

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

QStringList labelsToStringList(const ComAtprotoLabelDefs::SelfLabels &labels)
{
    QStringList list;
    for (const auto &value : labels.values) {
        list.append(value.val);
    }
    return list;
}

QStringList imagesToPathsList(const QList<DraftEmbedImage> &images)
{
    QStringList list;
    for (const auto &image : images) {
        list.append(image.localRef.path);
    }
    return list;
}

QStringList imagesToAltsList(const QList<DraftEmbedImage> &images)
{
    QStringList list;
    for (const auto &image : images) {
        list.append(image.alt);
    }
    return list;
}

QStringList videosToPathsList(const QList<DraftEmbedVideo> &videos)
{
    QStringList list;
    for (const auto &video : videos) {
        list.append(video.localRef.path);
    }
    return list;
}

QStringList videosToAltsList(const QList<DraftEmbedVideo> &videos)
{
    QStringList list;
    for (const auto &video : videos) {
        list.append(video.alt);
    }
    return list;
}

QVariantList videosCaptionsToVariant(const QList<DraftEmbedVideo> &videos)
{
    QVariantList list;
    for (const auto &video : videos) {
        if (!video.captions.isEmpty()) {
            QVariantList captions;
            for (const auto &caption : video.captions) {
                QVariantMap map;
                map["lang"] = caption.lang;
                map["content"] = caption.content;
                captions.append(map);
            }
            list.append(captions);
        } else {
            list.append(QVariantList());
        }
    }
    return list;
}

QStringList externalsToUrisList(const QList<DraftEmbedExternal> &externals)
{
    QStringList list;
    for (const auto &external : externals) {
        list.append(external.uri);
    }
    return list;
}

QStringList recordsToUrisList(const QList<DraftEmbedRecord> &records)
{
    QStringList list;
    for (const auto &record : records) {
        list.append(record.record.uri);
    }
    return list;
}

QStringList recordsToCidsList(const QList<DraftEmbedRecord> &records)
{
    QStringList list;
    for (const auto &record : records) {
        list.append(record.record.cid);
    }
    return list;
}

bool quoteEnabledFromDraft(const Draft &draft)
{
    // If disableRule exists, quote is disabled (return false)
    // Otherwise, quote is enabled (return true)
    if (draft.postgateEmbeddingRules_type
        == AppBskyDraftDefs::DraftPostgateEmbeddingRulesType::
                postgateEmbeddingRules_AppBskyFeedPostgate_DisableRule) {
        if (!draft.postgateEmbeddingRules_AppBskyFeedPostgate_DisableRule.isEmpty()) {
            return false;
        }
    }
    return true;
}

QString threadgateTypeFromDraft(const Draft &draft)
{
    if (draft.threadgateAllow_type == AppBskyDraftDefs::DraftThreadgateAllowType::none) {
        return QStringLiteral("everybody");
    }

    // Check if all lists are empty (nobody)
    bool hasRules = false;
    if (!draft.threadgateAllow_AppBskyFeedThreadgate_MentionRule.isEmpty()) {
        hasRules = true;
    }
    if (!draft.threadgateAllow_AppBskyFeedThreadgate_FollowerRule.isEmpty()) {
        hasRules = true;
    }
    if (!draft.threadgateAllow_AppBskyFeedThreadgate_FollowingRule.isEmpty()) {
        hasRules = true;
    }
    if (!draft.threadgateAllow_AppBskyFeedThreadgate_ListRule.isEmpty()) {
        hasRules = true;
    }

    if (!hasRules) {
        return QStringLiteral("nobody");
    }

    return QStringLiteral("choice");
}

QStringList threadgateRulesFromDraft(const Draft &draft)
{
    QStringList rules;

    // Check all rule lists since the parser may put data in any of them
    for (int i = 0; i < draft.threadgateAllow_AppBskyFeedThreadgate_MentionRule.count(); ++i) {
        rules.append(QStringLiteral("mentioned"));
    }
    for (int i = 0; i < draft.threadgateAllow_AppBskyFeedThreadgate_FollowerRule.count(); ++i) {
        rules.append(QStringLiteral("follower"));
    }
    for (int i = 0; i < draft.threadgateAllow_AppBskyFeedThreadgate_FollowingRule.count(); ++i) {
        rules.append(QStringLiteral("followed"));
    }
    for (const auto &rule : draft.threadgateAllow_AppBskyFeedThreadgate_ListRule) {
        rules.append(rule.list);
    }

    return rules;
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
        return QVariant();
    case DraftListModelRoles::IdRole:
        return view.id;
    case DraftListModelRoles::CreatedAtRole:
        return LexiconsTypeUnknown::formatDateTime(view.createdAt, true);
    case DraftListModelRoles::UpdatedAtRole:
        return LexiconsTypeUnknown::formatDateTime(view.updatedAt, true);
    case DraftListModelRoles::PrimaryTextRole:
        if (!view.draft.posts.isEmpty()) {
            return view.draft.posts.constFirst().text;
        }
        return QString();
    case DraftListModelRoles::PrimaryLabelsRole:
        if (!view.draft.posts.isEmpty()) {
            const auto &post = view.draft.posts.constFirst();
            if (post.labels_type
                == AppBskyDraftDefs::DraftPostLabelsType::labels_ComAtprotoLabelDefs_SelfLabels) {
                return labelsToStringList(post.labels_ComAtprotoLabelDefs_SelfLabels);
            }
        }
        return QVariant();
    case DraftListModelRoles::PrimaryEmbedImagesPathsRole:
        if (!view.draft.posts.isEmpty()) {
            const auto &post = view.draft.posts.constFirst();
            if (!post.embedImages.isEmpty()) {
                return imagesToPathsList(post.embedImages);
            }
        }
        return QVariant();
    case DraftListModelRoles::PrimaryEmbedImagesAltsRole:
        if (!view.draft.posts.isEmpty()) {
            const auto &post = view.draft.posts.constFirst();
            if (!post.embedImages.isEmpty()) {
                return imagesToAltsList(post.embedImages);
            }
        }
        return QVariant();
    case DraftListModelRoles::PrimaryEmbedVideosPathsRole:
        if (!view.draft.posts.isEmpty()) {
            const auto &post = view.draft.posts.constFirst();
            if (!post.embedVideos.isEmpty()) {
                return videosToPathsList(post.embedVideos);
            }
        }
        return QVariant();
    case DraftListModelRoles::PrimaryEmbedVideosAltsRole:
        if (!view.draft.posts.isEmpty()) {
            const auto &post = view.draft.posts.constFirst();
            if (!post.embedVideos.isEmpty()) {
                return videosToAltsList(post.embedVideos);
            }
        }
        return QVariant();
    case DraftListModelRoles::PrimaryEmbedVideosCaptionsRole:
        if (!view.draft.posts.isEmpty()) {
            const auto &post = view.draft.posts.constFirst();
            if (!post.embedVideos.isEmpty()) {
                return videosCaptionsToVariant(post.embedVideos);
            }
        }
        return QVariant();
    case DraftListModelRoles::PrimaryEmbedExternalsRole:
        if (!view.draft.posts.isEmpty()) {
            const auto &post = view.draft.posts.constFirst();
            if (!post.embedExternals.isEmpty()) {
                return externalsToUrisList(post.embedExternals);
            }
        }
        return QVariant();
    case DraftListModelRoles::PrimaryEmbedRecordsUrisRole:
        if (!view.draft.posts.isEmpty()) {
            const auto &post = view.draft.posts.constFirst();
            if (!post.embedRecords.isEmpty()) {
                return recordsToUrisList(post.embedRecords);
            }
        }
        return QVariant();
    case DraftListModelRoles::PrimaryEmbedRecordsCidsRole:
        if (!view.draft.posts.isEmpty()) {
            const auto &post = view.draft.posts.constFirst();
            if (!post.embedRecords.isEmpty()) {
                return recordsToCidsList(post.embedRecords);
            }
        }
        return QVariant();
    case DraftListModelRoles::LangsRole:
        return view.draft.langs;
    case DraftListModelRoles::PostgateEmbeddingRulesRole:
        return quoteEnabledFromDraft(view.draft);
    case DraftListModelRoles::ThreadGateTypeRole:
        return threadgateTypeFromDraft(view.draft);
    case DraftListModelRoles::ThreadGateRulesRole:
        return threadgateRulesFromDraft(view.draft);
    case DraftListModelRoles::PostCountRole:
        return view.draft.posts.count();
    case DraftListModelRoles::IsThreadRole:
        return view.draft.posts.count() > 1;
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
    roles[DraftListModelRoles::PrimaryLabelsRole] = "primaryLabels";
    roles[DraftListModelRoles::PrimaryEmbedImagesPathsRole] = "primaryEmbedImagesPaths";
    roles[DraftListModelRoles::PrimaryEmbedImagesAltsRole] = "primaryEmbedImagesAlts";
    roles[DraftListModelRoles::PrimaryEmbedVideosPathsRole] = "primaryEmbedVideosPaths";
    roles[DraftListModelRoles::PrimaryEmbedVideosAltsRole] = "primaryEmbedVideosAlts";
    roles[DraftListModelRoles::PrimaryEmbedVideosCaptionsRole] = "primaryEmbedVideosCaptions";
    roles[DraftListModelRoles::PrimaryEmbedExternalsRole] = "primaryEmbedExternals";
    roles[DraftListModelRoles::PrimaryEmbedRecordsUrisRole] = "primaryEmbedRecordsUris";
    roles[DraftListModelRoles::PrimaryEmbedRecordsCidsRole] = "primaryEmbedRecordsCids";
    roles[DraftListModelRoles::LangsRole] = "langs";
    roles[DraftListModelRoles::PostgateEmbeddingRulesRole] = "postgateEmbeddingRules";
    roles[DraftListModelRoles::ThreadGateTypeRole] = "threadGateType";
    roles[DraftListModelRoles::ThreadGateRulesRole] = "threadGateRules";
    roles[DraftListModelRoles::PostCountRole] = "postCount";
    roles[DraftListModelRoles::IsThreadRole] = "isThread";
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
