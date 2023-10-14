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
        CreatorHandleRole,
        CreatorDisplayNameRole,
        CreatoravatarRole,
    };
    Q_ENUM(ListsListModelRoles);
    enum VisibilityType {
        VisibilityTypeAll,
        VisibilityTypeCuration,
        VisibilityTypeModeration,
    };
    Q_ENUM(VisibilityType);

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

public slots:
    Q_INVOKABLE void getLatest();
    Q_INVOKABLE void getNext();

signals:
    void actorChanged();

    void visibilityTypeChanged();

protected:
    QHash<int, QByteArray> roleNames() const;
    virtual void finishedDisplayingQueuedPosts();
    virtual bool checkVisibility(const QString &cid);

private:
    void copyFrom(AtProtocolInterface::AppBskyGraphGetLists *lists);

    QHash<QString, AtProtocolType::AppBskyGraphDefs::ListView> m_listViewHash;
    QString m_actor;
    VisibilityType m_visibilityType;
};

#endif // LISTSLISTMODEL_H
