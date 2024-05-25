#include "chatlogsubscriber.h"

#include "atprotocol/chat/bsky/convo/chatbskyconvogetlog.h"

#include <QDebug>
#include <QTimer>

using namespace AtProtocolInterface;
using namespace AtProtocolType;
using namespace AtProtocolType::ChatBskyConvoDefs;
using AtProtocolInterface::ChatBskyConvoGetLog;

class ChatLogSubscriber::Private : public AtProtocolInterface::AtProtocolAccount
{
public:
    explicit Private(ChatLogSubscriber *parent);
    ~Private();

    static bool validateAccount(const AccountData &account);
    static QString accountKey(const AccountData &account);

    void start(const QString &cursor);
    void stop();
    void getLatest();

private:
    ChatLogSubscriber *q;

    ChatBskyConvoGetLog m_log;
    QTimer m_timer;
    bool m_running;
    QString m_cursor;
};

ChatLogSubscriber::Private::Private(ChatLogSubscriber *parent) : q(parent), m_running(false)
{
    qDebug() << this << "Private()";

    connect(&m_timer, &QTimer::timeout, this, &Private::getLatest);
    connect(&m_log, &ChatBskyConvoGetLog::finished, this, [=](bool success) {
        qDebug().noquote() << this << "receive" << success << m_cursor << "->" << m_log.cursor();
        if (success) {
            m_cursor = m_log.cursor();

            // old->new から new->oldの順番に直す
            QList<AtProtocolType::ChatBskyConvoDefs::MessageView> messages;
            for (const auto &msg : m_log.logsLogCreateMessageList()) {
                if (msg.message_type == LogCreateMessageMessageType::message_MessageView) {
                    messages.insert(0, msg.message_MessageView);
                }
            }
            QList<AtProtocolType::ChatBskyConvoDefs::DeletedMessageView> deleted_messages;
            for (const auto &msg : m_log.logsLogDeleteMessageList()) {
                if (msg.message_type == LogDeleteMessageMessageType::message_DeletedMessageView) {
                    deleted_messages.insert(0, msg.message_DeletedMessageView);
                }
            }
            emit q->receiveLogs(messages, deleted_messages, true);
        } else {
            emit q->errorOccured(m_log.errorCode(), m_log.errorMessage());
        }
        m_running = false;
    });
}

ChatLogSubscriber::Private::~Private()
{
    qDebug() << this << "~Private()";
}

bool ChatLogSubscriber::Private::validateAccount(const AccountData &account)
{
    return (account.service.startsWith("http") && account.service_endpoint.startsWith("http")
            && account.did.startsWith("did:") && !account.accessJwt.isEmpty());
}

QString ChatLogSubscriber::Private::accountKey(const AccountData &account)
{
    return QString("%1_%2").arg(account.service_endpoint).arg(account.did);
}

void ChatLogSubscriber::Private::start(const QString &cursor)
{
    m_timer.start(2000);
    m_cursor = cursor;
}

void ChatLogSubscriber::Private::stop()
{
    m_timer.stop();
}

void ChatLogSubscriber::Private::getLatest()
{
    if (m_running || m_cursor.isEmpty())
        return;
    m_running = true;

    qDebug().noquote() << this << "getLatest" << m_cursor;
    m_log.setAccount(account());
    m_log.setService(account().service_endpoint);
    m_log.getLog(m_cursor);
}

ChatLogSubscriber::ChatLogSubscriber(QObject *parent) : QObject { parent }
{
    qDebug() << this;
}

ChatLogSubscriber::~ChatLogSubscriber()
{
    qDebug() << this << "~ChatLogSubscriber()";
    for (const auto &key : d.keys()) {
        delete d[key];
    }
}

ChatLogSubscriber *ChatLogSubscriber::getInstance()
{
    static ChatLogSubscriber instance;
    return &instance;
}

void ChatLogSubscriber::setAccount(const AtProtocolInterface::AccountData &account)
{
    if (!ChatLogSubscriber::Private::validateAccount(account))
        return;
    const QString key = ChatLogSubscriber::Private::accountKey(account);
    if (!d.contains(key)) {
        d[key] = new ChatLogSubscriber::Private(this);
        // connect(this, &QObject::destroyed, [this, key]() { delete d[key]; });
    }
    d[key]->setAccount(account);
}

void ChatLogSubscriber::start(const AtProtocolInterface::AccountData &account,
                              const QString &cursor)
{
    if (!ChatLogSubscriber::Private::validateAccount(account))
        return;
    const QString key = ChatLogSubscriber::Private::accountKey(account);
    if (d.contains(key)) {
        d[key]->start(cursor);
    }
}

void ChatLogSubscriber::stop(const AtProtocolInterface::AccountData &account)
{
    if (!ChatLogSubscriber::Private::validateAccount(account))
        return;
    const QString key = ChatLogSubscriber::Private::accountKey(account);
    if (d.contains(key)) {
        d[key]->stop();
    }
}
