#ifndef CHATNOTIFICATIONLISTMODEL_H
#define CHATNOTIFICATIONLISTMODEL_H

#include "atpchatabstractlistmodel.h"

struct ChatNotificationData
{
    QString account_uuid;
    QString avatar;
    int unread_count = 0;
    bool visible = false;
};

class ChatNotificationListModel : public AtpChatAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged FINAL)

public:
    explicit ChatNotificationListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum ChatNotificationListModelRoles {
        ModelData = Qt::UserRole + 1,

        AccountUuidRole,
        AvatarRole,
        UnreadCountRole,
        VisibleRole,
    };
    Q_ENUM(ChatNotificationListModelRoles);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE QVariant item(int row,
                              ChatNotificationListModel::ChatNotificationListModelRoles role) const;

    virtual Q_INVOKABLE bool getLatest();
    virtual Q_INVOKABLE bool getNext();

    Q_INVOKABLE void start();
    Q_INVOKABLE void hideItem(int row);

    bool enabled() const;
    void setEnabled(bool newEnabled);

signals:
    void enabledChanged();

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    void getChatList();
    void appendData(const ChatNotificationData &data);
    QList<ChatNotificationData> m_chatNotificationData;
    QStringList m_accountUuidCue;
    bool m_enabled;
};

#endif // CHATNOTIFICATIONLISTMODEL_H
