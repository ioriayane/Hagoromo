#ifndef CHATLOGSUBSCRIBER_H
#define CHATLOGSUBSCRIBER_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvogetlog.h"

#include <QObject>
#include <QHash>

class ChatLogSubscriber : public QObject
{
    Q_OBJECT
    explicit ChatLogSubscriber(QObject *parent = nullptr);
    ~ChatLogSubscriber();

public:
    static ChatLogSubscriber *getInstance();

    void setAccount(const AtProtocolInterface::AccountData &account);
    void start(const AtProtocolInterface::AccountData &account, const QString &cursor);
    void stop(const AtProtocolInterface::AccountData &account);

    static bool isMine(const AtProtocolInterface::AccountData &account, const QString &key);
signals:
    // シグナルは全アカウント全チャットにブロードキャストされるので自分の分か判断すること！
    void receiveLogs(const QString &key, const AtProtocolInterface::ChatBskyConvoGetLog &log);
    void errorOccured(const QString &key, const QString &code, const QString &message);

private:
    class Private;
    QHash<QString, Private *> d;
    Q_DISABLE_COPY_MOVE(ChatLogSubscriber)
};

#endif // CHATLOGSUBSCRIBER_H
