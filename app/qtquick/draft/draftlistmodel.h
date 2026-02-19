#ifndef DRAFTLISTMODEL_H
#define DRAFTLISTMODEL_H

#include "atpabstractlistmodel.h"

#include "atprotocol/lexicons.h"

#include <QVariant>

class DraftListModel : public AtpAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int pageSize READ pageSize WRITE setPageSize NOTIFY pageSizeChanged FINAL)
    Q_PROPERTY(bool hasMore READ hasMore NOTIFY hasMoreChanged FINAL)

public:
    explicit DraftListModel(QObject *parent = nullptr);

    enum DraftListModelRoles {
        ModelData = Qt::UserRole + 1,
        IdRole,
        CreatedAtRole,
        UpdatedAtRole,
        PrimaryTextRole,
        PrimaryLabelsRole,
        PrimaryEmbedImagesPathsRole,
        PrimaryEmbedImagesAltsRole,
        PrimaryEmbedVideosPathsRole,
        PrimaryEmbedVideosAltsRole,
        PrimaryEmbedVideosCaptionsRole,
        PrimaryEmbedExternalsRole,
        PrimaryEmbedRecordsUrisRole,
        PrimaryEmbedRecordsCidsRole,
        LangsRole,
        PostgateEmbeddingRulesRole,
        ThreadGateTypeRole,
        ThreadGateRulesRole,
        PostCountRole,
        IsThreadRole,
        IsCurrentDeviceRole,
        DeviceNameRole,
    };
    Q_ENUM(DraftListModelRoles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Q_INVOKABLE QVariant item(int row,
                              DraftListModelRoles role = DraftListModelRoles::ModelData) const;

    Q_INVOKABLE bool getLatest() override;
    Q_INVOKABLE bool getNext() override;
    Q_INVOKABLE void clear();

    int pageSize() const;
    void setPageSize(int newPageSize);
    bool hasMore() const;

    int indexOf(const QString &cid) const override;
    QString getRecordText(const QString &cid) override;
    QString getOfficialUrl() const override;
    QString getItemOfficialUrl(int row) const override;

protected:
    QHash<int, QByteArray> roleNames() const override;
    bool aggregateQueuedPosts(const QString &cid, const bool next = false) override;
    bool aggregated(const QString &cid) const override;
    void finishedDisplayingQueuedPosts() override;
    bool checkVisibility(const QString &cid) override;

signals:
    void pageSizeChanged();
    void hasMoreChanged();

private:
    void requestDrafts(const QString &cursor, bool append);

    QStringList
    labelsToStringList(const AtProtocolType::ComAtprotoLabelDefs::SelfLabels &labels) const;
    QStringList
    imagesToPathsList(const QList<AtProtocolType::AppBskyDraftDefs::DraftEmbedImage> &images) const;
    QStringList
    imagesToAltsList(const QList<AtProtocolType::AppBskyDraftDefs::DraftEmbedImage> &images) const;
    QStringList
    videosToPathsList(const QList<AtProtocolType::AppBskyDraftDefs::DraftEmbedVideo> &videos) const;
    QStringList
    videosToAltsList(const QList<AtProtocolType::AppBskyDraftDefs::DraftEmbedVideo> &videos) const;
    QVariantList videosCaptionsToVariant(
            const QList<AtProtocolType::AppBskyDraftDefs::DraftEmbedVideo> &videos) const;
    QStringList externalsToUrisList(
            const QList<AtProtocolType::AppBskyDraftDefs::DraftEmbedExternal> &externals) const;
    QStringList recordsToUrisList(
            const QList<AtProtocolType::AppBskyDraftDefs::DraftEmbedRecord> &records) const;
    QStringList recordsToCidsList(
            const QList<AtProtocolType::AppBskyDraftDefs::DraftEmbedRecord> &records) const;
    bool quoteEnabledFromDraft(const AtProtocolType::AppBskyDraftDefs::Draft &draft) const;
    QString threadgateTypeFromDraft(const AtProtocolType::AppBskyDraftDefs::Draft &draft) const;
    QStringList
    threadgateRulesFromDraft(const AtProtocolType::AppBskyDraftDefs::Draft &draft) const;

    QList<AtProtocolType::AppBskyDraftDefs::DraftView> m_drafts;
    int m_pageSize;
};

#endif // DRAFTLISTMODEL_H
