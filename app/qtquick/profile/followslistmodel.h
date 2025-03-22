#ifndef FOLLOWSLISTMODEL_H
#define FOLLOWSLISTMODEL_H

#include "atprotocol/lexicons.h"
#include "atpabstractlistmodel.h"
#include "systemtool.h"
#include "atprotocol/app/bsky/graph/appbskygraphgetfollows.h"

class FollowsListModel : public AtpAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString targetDid READ targetDid WRITE setTargetDid NOTIFY targetDidChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit FollowsListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum FollowsListModelRoles {
        ModelData = Qt::UserRole + 1,
        DidRole,
        HandleRole,
        DisplayNameRole,
        DescriptionRole,
        AvatarRole,
        IndexedAtRole,
        MutedRole,
        BlockingRole,
        FollowingRole,
        FollowedByRole,
        BlockingUriRole,
        FollowingUriRole,
        LabelsRole,

        AssociatedChatAllowIncomingRole,
        AssociatedChatAllowRole,
    };
    Q_ENUM(FollowsListModelRoles)

    enum AssociatedChatAllowIncoming {
        AssociatedChatAllowIncomingAll,
        AssociatedChatAllowIncomingFollowing,
        AssociatedChatAllowIncomingNone,
        AssociatedChatAllowIncomingNotSet,
    };
    Q_ENUM(AssociatedChatAllowIncoming)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE QVariant item(int row, FollowsListModel::FollowsListModelRoles role) const;
    Q_INVOKABLE void remove(const QString &did);
    Q_INVOKABLE void clear();

    virtual Q_INVOKABLE int indexOf(const QString &cid) const;
    virtual Q_INVOKABLE QString getRecordText(const QString &cid);
    virtual Q_INVOKABLE QString getOfficialUrl() const { return QString(); }
    virtual Q_INVOKABLE QString getItemOfficialUrl(int row) const
    {
        Q_UNUSED(row)
        return QString();
    }
    Q_INVOKABLE void getProfile(const QString &did);

    QString targetDid() const;
    void setTargetDid(const QString &newTargetDid);

    int count() const;

public slots:
    Q_INVOKABLE bool getLatest();
    Q_INVOKABLE bool getNext();

signals:
    void profileTypeChanged();
    void targetDidChanged();
    void countChanged();

protected:
    QHash<int, QByteArray> roleNames() const;
    virtual bool aggregateQueuedPosts(const QString &cid, const bool next = false)
    {
        Q_UNUSED(cid)
        Q_UNUSED(next)
        return true;
    }
    virtual bool aggregated(const QString &cid) const
    {
        Q_UNUSED(cid)
        return false;
    }
    virtual void finishedDisplayingQueuedPosts() { }
    virtual bool checkVisibility(const QString &cid);
    void getProfiles();
    void copyProfiles(const AtProtocolInterface::AppBskyGraphGetFollows *followers);

    SystemTool m_systemTool;
    QStringList m_didList;
    QHash<QString, AtProtocolType::AppBskyActorDefs::ProfileView> m_profileHash;
    QHash<QString, QString> m_formattedDescriptionHash;

    QStringList m_cueGetProfile; // did

private:
    QString m_targetDid;
};

#endif // FOLLOWSLISTMODEL_H
