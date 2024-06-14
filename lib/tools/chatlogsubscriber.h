#ifndef CHATLOGSUBSCRIBER_H
#define CHATLOGSUBSCRIBER_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvogetlog.h"

#include <QObject>
#include <QHash>

class ChatLogConnector : public QObject
{
    Q_OBJECT
public:
    explicit ChatLogConnector(QObject *parent) : QObject(parent) { }
    ~ChatLogConnector() { }

signals:
    void receiveLogs(const AtProtocolInterface::ChatBskyConvoGetLog &log);
    void errorOccured(const QString &code, const QString &message);
};

class ChatLogSubscriber : public QObject
{
    Q_OBJECT
    explicit ChatLogSubscriber(QObject *parent = nullptr);
    ~ChatLogSubscriber();

public:
    static ChatLogSubscriber *getInstance();

    void setAccount(const AtProtocolInterface::AccountData &account, ChatLogConnector *connector);
    void start(const AtProtocolInterface::AccountData &account, const QString &cursor);
    void stop(const AtProtocolInterface::AccountData &account);

private:
    class Private;
    QHash<QString, Private *> d;
    Q_DISABLE_COPY_MOVE(ChatLogSubscriber)
};

#endif // CHATLOGSUBSCRIBER_H
