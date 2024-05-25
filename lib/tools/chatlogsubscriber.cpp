#include "chatlogsubscriber.h"

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

    QTimer m_timer;
    bool m_running;
    QString m_cursor;
};

ChatLogSubscriber::Private::Private(ChatLogSubscriber *parent) : q(parent), m_running(false)
{
    qDebug() << this << "ChatLogSubscriber::Private()";

    connect(&m_timer, &QTimer::timeout, this, &Private::getLatest);
}

ChatLogSubscriber::Private::~Private()
{
    qDebug() << this << "ChatLogSubscriber::~Private()";
    stop();
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
    if (!m_timer.isActive()) {
        qDebug().quote() << this << "start timer" << account().did << cursor;
        m_timer.start(2000);
        m_cursor = cursor;
    } else {
        qDebug().quote() << this << "(start timer)" << account().did << cursor;
    }
}

void ChatLogSubscriber::Private::stop()
{
    qDebug().quote() << this << "stop timer" << account().did;
    m_timer.stop();
}

void ChatLogSubscriber::Private::getLatest()
{
    if (m_running || m_cursor.isEmpty())
        return;
    m_running = true;

    qDebug().noquote() << this << "getLatest" << m_cursor << &m_cursor;
    ChatBskyConvoGetLog *log = new ChatBskyConvoGetLog(this);
    connect(log, &ChatBskyConvoGetLog::finished, this, [=](bool success) {
        const QString key = ChatLogSubscriber::Private::accountKey(account());

        qDebug().noquote() << this << "receive" << success << this->m_cursor << "->"
                           << log->cursor() << key;
        qDebug().noquote() << log->replyJson();
        if (success) {
            if (!log->cursor().isEmpty()) {
                this->m_cursor = log->cursor();
            }

            emit q->receiveLogs(key, *log);
        } else {
            emit q->errorOccured(key, log->errorCode(), log->errorMessage());
        }
        m_running = false;
        log->deleteLater();
    });

    log->setAccount(account());
    log->setService(account().service_endpoint);
    log->getLog(m_cursor);
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

bool ChatLogSubscriber::isMine(const AtProtocolInterface::AccountData &account, const QString &key)
{
    if (!ChatLogSubscriber::Private::validateAccount(account) || key.isEmpty())
        return false;
    return (ChatLogSubscriber::Private::accountKey(account) == key);
}
