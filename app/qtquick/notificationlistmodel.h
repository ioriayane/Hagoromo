#ifndef NOTIFICATIONLISTMODEL_H
#define NOTIFICATIONLISTMODEL_H

#include "../atprotocol/lexicons.h"
#include "../atprotocol/accessatprotocol.h"

#include <QAbstractListModel>
#include <QObject>

class NotificationListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit NotificationListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum NotificationListModelRoles {
        ModelData = Qt::UserRole + 1,
        DisplayNameRole,
        HandleRole,
        AvatarRole,
        IndexedAtRole,

        ReasonRole,
    };
    Q_ENUM(NotificationListModelRoles)

    enum NotificationListModelReason {
        ReasonUnknown,
        ReasonLike,
        ReasonRepost,
        ReasonFollow,
        ReasonMention,
        ReasonReply,
        ReasonQuote,
    };
    Q_ENUM(NotificationListModelReason)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE QVariant item(int row,
                              NotificationListModel::NotificationListModelRoles role) const;

    Q_INVOKABLE void setAccount(const QString &service, const QString &did, const QString &handle,
                                const QString &email, const QString &accessJwt,
                                const QString &refreshJwt);

    Q_INVOKABLE void getLatest();

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QList<QString> m_cidList; // これで取得したポストの順番を管理して実態はm_notificationHashで管理
    QHash<QString, AtProtocolType::AppBskyNotificationListNotifications::Notification>
            m_notificationHash;

    AtProtocolInterface::AccountData m_account;
};

#endif // NOTIFICATIONLISTMODEL_H
