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

    QList<AtProtocolType::AppBskyDraftDefs::DraftView> m_drafts;
    int m_pageSize;
};

#endif // DRAFTLISTMODEL_H
