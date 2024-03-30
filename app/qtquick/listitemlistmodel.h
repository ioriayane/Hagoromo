#ifndef LISTITEMLISTMODEL_H
#define LISTITEMLISTMODEL_H

#include "atprotocol/app/bsky/graph/appbskygraphgetlist.h"
#include "atpabstractlistmodel.h"
#include "systemtool.h"

class ListItemListModel : public AtpAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString uri READ uri WRITE setUri NOTIFY uriChanged)
    Q_PROPERTY(QString cid READ cid WRITE setCid NOTIFY cidChanged)
    Q_PROPERTY(QString rkey READ rkey WRITE setRkey NOTIFY rkeyChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString avatar READ avatar WRITE setAvatar NOTIFY avatarChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(bool subscribed READ subscribed WRITE setSubscribed NOTIFY subscribedChanged)
    Q_PROPERTY(QString creatorDid READ creatorDid WRITE setCreatorDid NOTIFY creatorDidChanged)
    Q_PROPERTY(QString creatorHandle READ creatorHandle WRITE setCreatorHandle NOTIFY
                       creatorHandleChanged)
    Q_PROPERTY(QString creatorDisplayName READ creatorDisplayName WRITE setCreatorDisplayName NOTIFY
                       creatorDisplayNameChanged)
    Q_PROPERTY(bool isModeration READ isModeration WRITE setIsModeration NOTIFY isModerationChanged
                       FINAL)

public:
    explicit ListItemListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum ListItemListModelRoles {
        ModelData = Qt::UserRole + 1,
        DidRole,
        HandleRole,
        DisplayNameRole,
        DescriptionRole,
        AvatarRole,
        IndexedAtRole,
    };
    Q_ENUM(ListItemListModelRoles);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE QVariant item(int row, ListItemListModel::ListItemListModelRoles role) const;
    virtual Q_INVOKABLE int indexOf(const QString &cid) const;
    virtual Q_INVOKABLE QString getRecordText(const QString &cid);
    virtual Q_INVOKABLE QString getOfficialUrl() const;
    virtual Q_INVOKABLE QString getItemOfficialUrl(int row) const { return QString(); }
    Q_INVOKABLE void clear();

    QString uri() const;
    void setUri(const QString &newUri);
    QString cid() const;
    void setCid(const QString &newCid);
    QString rkey() const;
    void setRkey(const QString &newRkey);
    QString name() const;
    void setName(const QString &newName);
    QString avatar() const;
    void setAvatar(const QString &newAvatar);
    QString description() const;
    void setDescription(const QString &newDescription);
    bool subscribed() const;
    void setSubscribed(bool newSubscribed);
    QString creatorDid() const;
    void setCreatorDid(const QString &newCreatorDid);
    QString creatorHandle() const;
    void setCreatorHandle(const QString &newCreatorHandle);
    QString creatorDisplayName() const;
    void setCreatorDisplayName(const QString &newCreatorDisplayName);
    bool isModeration() const;
    void setIsModeration(bool newIsModeration);

public slots:
    Q_INVOKABLE bool getLatest();
    Q_INVOKABLE bool getNext();

signals:
    void finished(bool success);
    void uriChanged();
    void cidChanged();
    void rkeyChanged();
    void nameChanged();
    void avatarChanged();
    void descriptionChanged();
    void subscribedChanged();
    void creatorDidChanged();
    void creatorHandleChanged();
    void creatorDisplayNameChanged();
    void isModerationChanged();

protected:
    QHash<int, QByteArray> roleNames() const;
    virtual void finishedDisplayingQueuedPosts();
    virtual bool checkVisibility(const QString &cid);

private:
    void copyFrom(AtProtocolInterface::AppBskyGraphGetList *list);

    SystemTool m_systemTool;
    QHash<QString, AtProtocolType::AppBskyGraphDefs::ListItemView> m_listItemViewHash;
    QString m_formattedDescription;

    QString m_uri;
    QString m_cid;
    QString m_rkey;
    QString m_name;
    QString m_avatar;
    QString m_description;
    bool m_subscribed;
    QString m_creatorDid;
    QString m_creatorHandle;
    QString m_creatorDisplayName;
    bool m_isModeration;
};

#endif // LISTITEMLISTMODEL_H
