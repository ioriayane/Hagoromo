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
        SubscribedRole,
        CreatorHandleRole,
        CreatorDisplayNameRole,
        CreatoravatarRole,
        SearchStatusRole,
    };
    Q_ENUM(ListsListModelRoles);
    enum VisibilityType {
        VisibilityTypeAll,
        VisibilityTypeCuration,
        VisibilityTypeModeration,
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
    virtual Q_INVOKABLE int indexOf(const QString &cid) const;
    virtual Q_INVOKABLE QString getRecordText(const QString &cid);
    Q_INVOKABLE void clear();

    QString actor() const;
    void setActor(const QString &newActor);
    VisibilityType visibilityType() const;
    void setVisibilityType(const VisibilityType &newVisibilityType);
    QString searchTarget() const;
    void setSearchTarget(const QString &newSearchTarget);

public slots:
    Q_INVOKABLE bool getLatest();
    Q_INVOKABLE bool getNext();

signals:
    void actorChanged();
    void visibilityTypeChanged();
    void searchTargetChanged();

protected:
    QHash<int, QByteArray> roleNames() const;
    virtual void finishedDisplayingQueuedPosts();
    virtual bool checkVisibility(const QString &cid);

private:
    void copyFrom(AtProtocolInterface::AppBskyGraphGetLists *lists);
    void searchActorInEachLists();

    QHash<QString, AtProtocolType::AppBskyGraphDefs::ListView> m_listViewHash;
    QStringList m_searchQue;
    QString m_actor;
    VisibilityType m_visibilityType;
    QString m_searchTarget;
};

#endif // LISTSLISTMODEL_H
