#ifndef CHATLOGSUBSCRIBER_H
#define CHATLOGSUBSCRIBER_H

#include "atprotocol/accessatprotocol.h"

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
    void start(const AtProtocolInterface::AccountData &account,
               const QString &cursor);
    void stop(const AtProtocolInterface::AccountData &account);

signals:
    void
    receiveLogs(const QList<AtProtocolType::ChatBskyConvoDefs::MessageView> &messages,
                const QList<AtProtocolType::ChatBskyConvoDefs::DeletedMessageView> &deletedMessages,
                bool to_top);
    void errorOccured(const QString &code, const QString &message);

private:
    class Private;
    QHash<QString, Private *> d;
    Q_DISABLE_COPY_MOVE(ChatLogSubscriber)
};

#endif // CHATLOGSUBSCRIBER_H
