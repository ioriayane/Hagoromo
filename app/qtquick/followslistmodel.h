#ifndef FOLLOWSLISTMODEL_H
#define FOLLOWSLISTMODEL_H

#include "atprotocol/lexicons.h"
#include "atpabstractlistmodel.h"

class FollowsListModel : public AtpAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString targetDid READ targetDid WRITE setTargetDid NOTIFY targetDidChanged)

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
        BlockedByRole,
        FollowingRole,
        FollowedByRole,
        FollowingUriRole,
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE QVariant item(int row, FollowsListModel::FollowsListModelRoles role) const;
    Q_INVOKABLE void remove(const QString &did);

    virtual Q_INVOKABLE int indexOf(const QString &cid) const;
    virtual Q_INVOKABLE QString getRecordText(const QString &cid);

    QString targetDid() const;
    void setTargetDid(const QString &newTargetDid);

public slots:
    Q_INVOKABLE void getLatest();

signals:
    void profileTypeChanged();
    void targetDidChanged();

protected:
    QHash<int, QByteArray> roleNames() const;

    QStringList m_didList;
    QHash<QString, AtProtocolType::AppBskyActorDefs::ProfileView> m_profileHash;

private:
    QString m_targetDid;
};

#endif // FOLLOWSLISTMODEL_H
