#ifndef LISTSLISTMODEL_H
#define LISTSLISTMODEL_H

#include "atprotocol/app/bsky/graph/appbskygraphgetlists.h"
#include "atpabstractlistmodel.h"

class ListsListModel : public AtpAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString actor READ actor WRITE setActor NOTIFY actorChanged)
    Q_PROPERTY(VisibilityType visibilityType READ visibilityType WRITE setVisibilityType NOTIFY
                       visibilityTypeChanged)
    // 取得したリストそれぞれの中に登録されているか確認するユーザーのdid
    Q_PROPERTY(
            QString searchTarget READ searchTarget WRITE setSearchTarget NOTIFY searchTargetChanged)
    Q_PROPERTY(int checkedCount READ checkedCount WRITE setCheckedCount NOTIFY checkedCountChanged)
public:
    explicit ListsListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum ListsListModelRoles {
        ModelData = Qt::UserRole + 1,
        CidRole,
        UriRole,
        AvatarRole,
        NameRole,
        DescriptionRole,
        MutedRole,
        BlockedRole,
        BlockedUriRole,
        CreatorHandleRole,
        CreatorDisplayNameRole,
        CreatorAvatarRole,
        IsModerationRole,
        SearchStatusRole,
        ListItemUriRole, // searchTargetで登録しているか確認したユーザーのListItemのレコードURI
        CheckedRole,
    };
    Q_ENUM(ListsListModelRoles);
    enum VisibilityType {
        VisibilityTypeAll,
        VisibilityTypeCuration,
        VisibilityTypeModeration,
        VisibilityTypeUnknown,
    };
    Q_ENUM(VisibilityType);
    enum SearchStatusType {
        SearchStatusTypeUnknown, // まだ探していない
        SearchStatusTypeRunning, // 検索中
        SearchStatusTypeNotContains,
        SearchStatusTypeContains,
    };
    Q_ENUM(SearchStatusType);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE QVariant item(int row, ListsListModel::ListsListModelRoles role) const;
    Q_INVOKABLE void update(int row, ListsListModel::ListsListModelRoles role,
                            const QVariant &value);
    virtual Q_INVOKABLE int indexOf(const QString &cid) const;
    virtual Q_INVOKABLE QString getRecordText(const QString &cid);
    virtual Q_INVOKABLE QString getOfficialUrl() const { return QString(); }
    virtual Q_INVOKABLE QString getItemOfficialUrl(int row) const { return QString(); }
    Q_INVOKABLE void clear();
    Q_INVOKABLE void clearListItemCache();
    Q_INVOKABLE bool addRemoveFromList(const int row, const QString &did);

    Q_INVOKABLE void mute(const int row);
    Q_INVOKABLE void block(const int row);

    VisibilityType toVisibilityType(const QString &purpose) const;

    QString actor() const;
    void setActor(const QString &newActor);
    VisibilityType visibilityType() const;
    void setVisibilityType(const VisibilityType &newVisibilityType);
    QString searchTarget() const;
    void setSearchTarget(const QString &newSearchTarget);
    int checkedCount() const;
    void setCheckedCount(int newCheckedCount);

public slots:
    Q_INVOKABLE bool getLatest();
    Q_INVOKABLE bool getNext();

signals:
    void actorChanged();
    void visibilityTypeChanged();
    void searchTargetChanged();
    void checkedCountChanged();

protected:
    QHash<int, QByteArray> roleNames() const;
    virtual bool aggregateQueuedPosts(const QString &cid, const bool next = false);
    virtual bool aggregated(const QString &cid) const;
    virtual void finishedDisplayingQueuedPosts();
    virtual bool checkVisibility(const QString &cid);
    void copyFrom(AtProtocolInterface::AppBskyGraphGetLists *lists);

private:
    void searchActorInEachLists();
    void searchActorInEachListsFromCache();
    QString getListCidByUri(const QString &uri) const;
    void setListItemStatus(const SearchStatusType status);

    QHash<QString, AtProtocolType::AppBskyGraphDefs::ListView> m_listViewHash;
    QHash<QString, SearchStatusType> m_searchStatusHash;
    QHash<QString, QString> m_listItemUriHash;
    QHash<QString, bool> m_checkedHash;
    QStringList m_searchCidQue;
    QString m_listItemCursor;
    QString m_actor;
    VisibilityType m_visibilityType;
    QString m_searchTarget;
    int m_checkedCount;
};

#endif // LISTSLISTMODEL_H
