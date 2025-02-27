#ifndef CHATNOTIFICATIONLISTMODEL_H
#define CHATNOTIFICATIONLISTMODEL_H

#include "atpchatabstractlistmodel.h"

class ChatNotificationListModel : public AtpChatAbstractListModel
{
    Q_OBJECT
public:
    explicit ChatNotificationListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum ChatNotificationListModelRoles {
        ModelData = Qt::UserRole + 1,
        IdRole,
        RevRole,

        MemberHandlesRole,
        MemberDisplayNamesRole,
        MemberAvatarsRole,

        LastMessageIdRole,
        LastMessageRevRole,
        LastMessageSenderDidRole,
        LastMessageTextRole,
        LastMessageSentAtRole,

        MutedRole,
        UnreadCountRole,

        RunningRole,
    };
    Q_ENUM(ChatNotificationListModelRoles);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE QVariant item(int row,
                              ChatNotificationListModel::ChatNotificationListModelRoles role) const;

    virtual Q_INVOKABLE bool getLatest();
    virtual Q_INVOKABLE bool getNext();

protected:
    QHash<int, QByteArray> roleNames() const;
};

#endif // CHATNOTIFICATIONLISTMODEL_H
